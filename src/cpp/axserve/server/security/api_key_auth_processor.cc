// Copyright 2023 Yunseong Hwang
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
//
// SPDX-FileCopyrightText: 2025 Yunseong Hwang
//
// SPDX-License-Identifier: Apache-2.0

#include "api_key_auth_processor.h"

grpc::Status ApiKeyAuthProcessor::Process(
    const InputMetadata &auth_metadata, grpc::AuthContext *context,
    OutputMetadata *consumed_auth_metadata, OutputMetadata *response_metadata
) {
  auto it = auth_metadata.find("x-api-key");
  if (it == auth_metadata.end()) {
    it = auth_metadata.find("authorization");
  }
  if (it != auth_metadata.end()) {
    std::string api_key = std::string(it->second.data(), it->second.length());
    if (m_allowed_keys.count(api_key) > 0) {
      context->AddProperty("api_key", api_key);
      return grpc::Status::OK;
    }
  }
  return grpc::Status(
      grpc::StatusCode::UNAUTHENTICATED, "Invalid or missing API key"
  );
}
