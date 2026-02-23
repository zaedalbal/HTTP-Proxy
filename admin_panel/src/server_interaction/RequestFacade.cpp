#include "server_interaction/RequestFacade.hpp"

RequestFacade::RequestFacade(ServerInteraction& ServerInteractionObject) : ServerInteraction_(ServerInteractionObject)
{}

void RequestFacade::sendAuthenticationRequest(const QString& login_QString, const QString& password_QString)
{
    struct Authentication_header
    {
        quint32 login_size;
        quint32 password_size;
    };

    std::string login = login_QString.toUtf8().toStdString();
    std::string password = password_QString.toUtf8().toStdString();
    Authentication_header auth_head;
    auth_head.login_size = login.size();
    auth_head.password_size = password.size();
    quint32 auth_head_size = auth_head.login_size + auth_head.password_size;
    quint32 request_data_size = sizeof(auth_head) + auth_head_size;
    api::Request request(request_data_size);
    request.Command = api::CommandName::AuthenticationRequest;
    request.isChunckedRequest = false;
    request.data_size = request_data_size;
    auto offset = request.data.get();
    std::memcpy(offset, &auth_head, sizeof(Authentication_header));
    offset += sizeof(Authentication_header);
    std::memcpy(offset, login.data(), auth_head.login_size);
    offset += auth_head.login_size;
    std::memcpy(offset, password.data(), password.size());
    
    ServerInteraction_.sendRequest(std::move(request));
}

void RequestFacade::sendGetProxyConfigRequest()
{
    api::Request request;
    request.Command = api::CommandName::Get_proxy_config;
    request.isChunckedRequest = false;
    request.data_size = 0;
    ServerInteraction_.sendRequest(std::move(request));
}

void RequestFacade::sendGetProxySessionsRequest()
{
    api::Request request;
    request.Command = api::CommandName::Get_proxy_sessions;
    request.isChunckedRequest = false;
    request.data_size = 0;
    ServerInteraction_.sendRequest(std::move(request));
}

void RequestFacade::sendGetProxyLog()
{
    api::Request request;
    request.Command = api::CommandName::Get_proxy_log;
    request.isChunckedRequest = false;
    request.data_size = 0;
    ServerInteraction_.sendRequest(std::move(request));
}