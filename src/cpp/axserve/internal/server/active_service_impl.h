#ifndef ACTIVE_SERVICE_IMPL_H
#define ACTIVE_SERVICE_IMPL_H

#include "active.grpc.pb.h"
#include "active_service_request_processor.h"

using grpc::CallbackServerContext;
using grpc::ServerBidiReactor;
using grpc::ServerUnaryReactor;

class ActiveServiceImpl final : public Active::CallbackService {
private:
  ActiveServiceRequestProcessor *m_processor;

public:
  explicit ActiveServiceImpl(ActiveServiceRequestProcessor *processor);

  ServerUnaryReactor *Describe(
      CallbackServerContext *context, const DescribeRequest *request,
      DescribeResponse *response
  ) override;
  ServerUnaryReactor *GetProperty(
      CallbackServerContext *context, const GetPropertyRequest *request,
      GetPropertyResponse *response
  ) override;
  ServerUnaryReactor *SetProperty(
      CallbackServerContext *context, const SetPropertyRequest *request,
      SetPropertyResponse *response
  ) override;
  ServerUnaryReactor *InvokeMethod(
      CallbackServerContext *context, const InvokeMethodRequest *request,
      InvokeMethodResponse *response
  ) override;
  ServerUnaryReactor *ConnectEvent(
      CallbackServerContext *context, const ConnectEventRequest *request,
      ConnectEventResponse *response
  ) override;
  ServerUnaryReactor *DisconnectEvent(
      CallbackServerContext *context, const DisconnectEventRequest *request,
      DisconnectEventResponse *response
  ) override;
  ServerBidiReactor<HandleEventResponse, HandleEventRequest> *
  HandleEvent(CallbackServerContext *context) override;
};

#endif
