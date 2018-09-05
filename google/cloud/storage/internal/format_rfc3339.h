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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_INTERNAL_FORMAT_RFC3339_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_INTERNAL_FORMAT_RFC3339_H_

#include "google/cloud/storage/version.h"
#include <chrono>
#include <string>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
namespace internal {
/**
 * Format @p tp as a RFC-3339 timestamp.
 *
 * Google Cloud Storage uses RFC-3339 for timestamps, this function is used
 * convert from `std::chrono::system_clock::time_point`, the C++
 * class used to represent timestamps, to the RFC-3339 format.
 *
 * There are many possible formats for RFC-3339 timestamps, this function always
 * uses YYYY-MM-DDTHH:MM:SS.FFFZ.  The fractional seconds always represent the
 * full precision of `std::chrono::system_clock::time_point`. Note, however,
 * that C++ does not specify the minimum precision of these time points. In
 * most platforms time points have sub-second precision, and microseconds are
 * common.
 *
 * @see https://tools.ietf.org/html/rfc3339
 */
std::string FormatRfc3339(std::chrono::system_clock::time_point tp);

}  // namespace internal
}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_INTERNAL_FORMAT_RFC3339_H_