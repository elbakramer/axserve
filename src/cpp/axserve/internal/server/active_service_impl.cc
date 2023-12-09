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
// SPDX-License-Identifier: Apache-2.0

#include "active_service_impl.h"

ActiveServiceImpl::ActiveServiceImpl(ActiveServiceRequestProcessor *processor)
    : m_processor(processor) {}

ServerUnaryReactor *ActiveServiceImpl::Describe(
    CallbackServerContext *context, const DescribeRequest *request,
    DescribeResponse *response
) {
  return m_processor->create_unary_reactor(context, request, response);
}

ServerUnaryReactor *ActiveServiceImpl::GetProperty(
    CallbackServerContext *context, const GetPropertyRequest *request,
    GetPropertyResponse *response
) {
  return m_processor->create_unary_reactor(context, request, response);
}

ServerUnaryReactor *ActiveServiceImpl::SetProperty(
    CallbackServerContext *context, const SetPropertyRequest *request,
    SetPropertyResponse *response
) {
  return m_processor->create_unary_reactor(context, request, response);
}

ServerUnaryReactor *ActiveServiceImpl::InvokeMethod(
    CallbackServerContext *context, const InvokeMethodRequest *request,
    InvokeMethodResponse *response
) {
  return m_processor->create_unary_reactor(context, request, response);
}

ServerUnaryReactor *ActiveServiceImpl::ConnectEvent(
    CallbackServerContext *context, const ConnectEventRequest *request,
    ConnectEventResponse *response
) {
  return m_processor->create_unary_reactor(context, request, response);
}

ServerUnaryReactor *ActiveServiceImpl::DisconnectEvent(
    CallbackServerContext *context, const DisconnectEventRequest *request,
    DisconnectEventResponse *response
) {
  return m_processor->create_unary_reactor(context, request, response);
}

ServerBidiReactor<HandleEventResponse, HandleEventRequest> *
ActiveServiceImpl::HandleEvent(CallbackServerContext *context) {
  return m_processor->create_handle_reactor(context);
}
