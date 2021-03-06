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

#include "google/cloud/storage/notification_metadata.h"

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
StatusOr<NotificationMetadata> NotificationMetadata::ParseFromJson(
    internal::nl::json const& json) {
  if (not json.is_object()) {
    return Status(StatusCode::kInvalidArgument, __func__);
  }
  NotificationMetadata result{};

  if (json.count("custom_attributes") != 0U) {
    for (auto const& kv : json["custom_attributes"].items()) {
      result.custom_attributes_.emplace(kv.key(),
                                        kv.value().get<std::string>());
    }
  }
  result.etag_ = json.value("etag", "");

  if (json.count("event_types") != 0U) {
    for (auto const& kv : json["event_types"].items()) {
      result.event_types_.emplace_back(kv.value().get<std::string>());
    }
  }

  result.id_ = json.value("id", "");
  result.kind_ = json.value("kind", "");
  result.object_name_prefix_ = json.value("object_name_prefix", "");
  result.payload_format_ = json.value("payload_format", "");
  result.self_link_ = json.value("selfLink", "");
  result.topic_ = json.value("topic", "");

  return result;
}

StatusOr<NotificationMetadata> NotificationMetadata::ParseFromString(
    std::string const& payload) {
  internal::nl::json json = internal::nl::json::parse(payload, nullptr, false);
  return ParseFromJson(json);
}

std::string NotificationMetadata::JsonPayloadForInsert() const {
  // Required fields, always include them, even if empty.
  internal::nl::json json{
      {"topic", topic()},
      {"payload_format", payload_format()},
  };

  // custom_attributes()
  if (not custom_attributes().empty()) {
    internal::nl::json ca;
    for (auto const& kv : custom_attributes()) {
      ca[kv.first] = kv.second;
    }
    json["custom_attributes"] = ca;
  }

  // event_types()
  if (not event_types().empty()) {
    internal::nl::json events;
    for (auto const& v : event_types()) {
      events.push_back(v);
    }
    json["event_types"] = events;
  }

  if (not object_name_prefix().empty()) {
    json["object_name_prefix"] = object_name_prefix();
  }

  return json.dump();
}

std::ostream& operator<<(std::ostream& os, NotificationMetadata const& rhs) {
  os << "NotificationMetadata={id=" << rhs.id();

  // custom_attributes()
  char const* sep = "custom_attributes.";
  for (auto const& kv : rhs.custom_attributes()) {
    os << sep << kv.first << "=" << kv.second;
    sep = ", custom_attributes.";
  }

  os << ", etag=" << rhs.etag();

  // event_types()
  os << ", event_types=[";
  sep = "";
  for (auto const& v : rhs.event_types()) {
    os << sep << v;
    sep = ", ";
  }
  os << "]";

  return os << ", kind=" << rhs.kind()
            << ", object_name_prefix=" << rhs.object_name_prefix()
            << ", payload_format=" << rhs.payload_format()
            << ", self_link=" << rhs.self_link() << ", topic=" << rhs.topic()
            << "}";
}

}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google
