// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/storage/client.h"
#include "google/cloud/storage/notification_event_type.h"
#include "google/cloud/storage/notification_payload_format.h"
#include "google/cloud/storage/oauth2/google_credentials.h"
#include "google/cloud/storage/retry_policy.h"
#include "google/cloud/storage/testing/canonical_errors.h"
#include "google/cloud/storage/testing/mock_client.h"
#include "google/cloud/storage/testing/retry_tests.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
namespace {
using ::testing::_;
using ::testing::HasSubstr;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ms = std::chrono::milliseconds;
using testing::canonical_errors::TransientError;

/**
 * Test the BucketAccessControls-related functions in storage::Client.
 */
class NotificationsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mock_ = std::make_shared<testing::MockClient>();
    EXPECT_CALL(*mock_, client_options())
        .WillRepeatedly(ReturnRef(client_options_));
    client_.reset(new Client{std::shared_ptr<internal::RawClient>(mock_)});
  }
  void TearDown() override {
    client_.reset();
    mock_.reset();
  }

  std::shared_ptr<testing::MockClient> mock_;
  std::unique_ptr<Client> client_;
  ClientOptions client_options_ =
      ClientOptions(oauth2::CreateAnonymousCredentials());
};

TEST_F(NotificationsTest, ListNotifications) {
  std::vector<NotificationMetadata> expected{
      NotificationMetadata::ParseFromString(R"""({
          "id": "test-notification-1",
          "topic": "test-topic-1"
      })""")
          .value(),
      NotificationMetadata::ParseFromString(R"""({
          "id": "test-notification-2",
          "topic": "test-topic-2"
      })""")
          .value(),
  };

  EXPECT_CALL(*mock_, ListNotifications(_))
      .WillOnce(Return(
          StatusOr<internal::ListNotificationsResponse>(TransientError())))
      .WillOnce(Invoke([&expected](
                           internal::ListNotificationsRequest const& r) {
        EXPECT_EQ("test-bucket", r.bucket_name());

        return make_status_or(internal::ListNotificationsResponse{expected});
      }));
  Client client{std::shared_ptr<internal::RawClient>(mock_)};

  StatusOr<std::vector<NotificationMetadata>> actual =
      client.ListNotifications("test-bucket");
  ASSERT_TRUE(actual.ok());
  EXPECT_EQ(expected, actual.value());
}

TEST_F(NotificationsTest, ListNotificationsTooManyFailures) {
  testing::TooManyFailuresStatusTest<internal::ListNotificationsResponse>(
      mock_, EXPECT_CALL(*mock_, ListNotifications(_)),
      [](Client& client) {
        return client.ListNotifications("test-bucket-name").status();
      },
      "ListNotifications");
}

TEST_F(NotificationsTest, ListNotificationsPermanentFailure) {
  testing::PermanentFailureStatusTest<internal::ListNotificationsResponse>(
      *client_, EXPECT_CALL(*mock_, ListNotifications(_)),
      [](Client& client) {
        return client.ListNotifications("test-bucket-name").status();
      },
      "ListNotifications");
}

TEST_F(NotificationsTest, CreateNotification) {
  NotificationMetadata expected = NotificationMetadata::ParseFromString(R"""({
          "id": "test-notification-1",
          "topic": "test-topic-1",
          "payload_format": "JSON_API_V1",
          "object_prefix": "test-object-prefix-",
          "event_type": [ "OBJECT_FINALIZE" ]
      })""")
                                      .value();

  EXPECT_CALL(*mock_, CreateNotification(_))
      .WillOnce(Return(StatusOr<NotificationMetadata>(TransientError())))
      .WillOnce(
          Invoke([&expected](internal::CreateNotificationRequest const& r) {
            EXPECT_EQ("test-bucket", r.bucket_name());
            EXPECT_THAT(r.json_payload(), HasSubstr("test-topic-1"));
            EXPECT_THAT(r.json_payload(), HasSubstr("JSON_API_V1"));
            EXPECT_THAT(r.json_payload(), HasSubstr("test-object-prefix-"));
            EXPECT_THAT(r.json_payload(), HasSubstr("OBJECT_FINALIZE"));

            return make_status_or(expected);
          }));
  Client client{std::shared_ptr<internal::RawClient>(mock_)};

  StatusOr<NotificationMetadata> actual = client.CreateNotification(
      "test-bucket", "test-topic-1", payload_format::JsonApiV1(),
      NotificationMetadata()
          .set_object_name_prefix("test-object-prefix-")
          .append_event_type(event_type::ObjectFinalize()));
  ASSERT_TRUE(actual.ok());
  EXPECT_EQ(expected, actual.value());
}

TEST_F(NotificationsTest, CreateNotificationTooManyFailures) {
  testing::TooManyFailuresStatusTest<NotificationMetadata>(
      mock_, EXPECT_CALL(*mock_, CreateNotification(_)),
      [](Client& client) {
        return client.CreateNotification("test-bucket-name", "test-topic-1",
                                         payload_format::JsonApiV1(),
                                         NotificationMetadata()).status();
      },
      "CreateNotification");
}

TEST_F(NotificationsTest, CreateNotificationPermanentFailure) {
  testing::PermanentFailureStatusTest<NotificationMetadata>(
      *client_, EXPECT_CALL(*mock_, CreateNotification(_)),
      [](Client& client) {
        return client.CreateNotification("test-bucket-name", "test-topic-1",
                                         payload_format::JsonApiV1(),
                                         NotificationMetadata()).status();
      },
      "CreateNotification");
}

TEST_F(NotificationsTest, GetNotification) {
  NotificationMetadata expected = NotificationMetadata::ParseFromString(R"""({
          "id": "test-notification-1",
          "topic": "test-topic-1",
          "payload_format": "JSON_API_V1",
          "object_prefix": "test-object-prefix-",
          "event_type": [ "OBJECT_FINALIZE" ]
      })""")
                                      .value();

  EXPECT_CALL(*mock_, GetNotification(_))
      .WillOnce(Return(StatusOr<NotificationMetadata>(TransientError())))
      .WillOnce(Invoke([&expected](internal::GetNotificationRequest const& r) {
        EXPECT_EQ("test-bucket", r.bucket_name());
        EXPECT_EQ("test-notification-1", r.notification_id());

        return make_status_or(expected);
      }));
  Client client{std::shared_ptr<internal::RawClient>(mock_)};

  StatusOr<NotificationMetadata> actual =
      client.GetNotification("test-bucket", "test-notification-1");
  ASSERT_TRUE(actual.ok());
  EXPECT_EQ(expected, actual.value());
}

TEST_F(NotificationsTest, GetNotificationTooManyFailures) {
  testing::TooManyFailuresStatusTest<NotificationMetadata>(
      mock_, EXPECT_CALL(*mock_, GetNotification(_)),
      [](Client& client) {
        return client.GetNotification("test-bucket-name",
                                      "test-notification-1").status();
      },
      "GetNotification");
}

TEST_F(NotificationsTest, GetNotificationPermanentFailure) {
  testing::PermanentFailureStatusTest<NotificationMetadata>(
      *client_, EXPECT_CALL(*mock_, GetNotification(_)),
      [](Client& client) {
        return client.GetNotification("test-bucket-name",
                                      "test-notification-1").status();
      },
      "GetNotification");
}

TEST_F(NotificationsTest, DeleteNotification) {
  EXPECT_CALL(*mock_, DeleteNotification(_))
      .WillOnce(Return(StatusOr<internal::EmptyResponse>(TransientError())))
      .WillOnce(Invoke([](internal::DeleteNotificationRequest const& r) {
        EXPECT_EQ("test-bucket", r.bucket_name());
        EXPECT_EQ("test-notification-1", r.notification_id());

        return make_status_or(internal::EmptyResponse{});
      }));
  Client client{std::shared_ptr<internal::RawClient>(mock_)};

  StatusOr<void> status =
      client.DeleteNotification("test-bucket", "test-notification-1");
  ASSERT_TRUE(status.ok());
}

TEST_F(NotificationsTest, DeleteNotificationTooManyFailures) {
  testing::TooManyFailuresStatusTest<internal::EmptyResponse>(
      mock_, EXPECT_CALL(*mock_, DeleteNotification(_)),
      [](Client& client) {
        return client.DeleteNotification("test-bucket-name",
                                         "test-notification-1").status();
      },
      "DeleteNotification");
}

TEST_F(NotificationsTest, DeleteNotificationPermanentFailure) {
  testing::PermanentFailureStatusTest<internal::EmptyResponse>(
      *client_, EXPECT_CALL(*mock_, DeleteNotification(_)),
      [](Client& client) {
        return client.DeleteNotification("test-bucket-name",
                                         "test-notification-1").status();
      },
      "DeleteNotification");
}

}  // namespace
}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google
