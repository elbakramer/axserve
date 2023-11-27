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