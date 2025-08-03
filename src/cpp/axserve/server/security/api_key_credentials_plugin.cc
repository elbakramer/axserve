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

#include "api_key_credentials_plugin.h"

grpc::Status ApiKeyCredentialsPlugin::GetMetadata(
    grpc::string_ref service_url, grpc::string_ref method_name,
    const grpc::AuthContext &channel_auth_context,
    std::multimap<grpc::string, grpc::string> *metadata
) {
  metadata->insert({"x-api-key", m_api_key});
  metadata->insert({"authorization", "Bearer " + m_api_key});
  return grpc::Status::OK;
}

std::shared_ptr<grpc::CallCredentials>
ApiKeyCredentials(const std::string &api_key) {
  std::unique_ptr<ApiKeyCredentialsPlugin> plugin =
      std::make_unique<ApiKeyCredentialsPlugin>(api_key);
  return grpc::MetadataCredentialsFromPlugin(std::move(plugin));
}
