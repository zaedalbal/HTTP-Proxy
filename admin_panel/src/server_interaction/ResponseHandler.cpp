#include "server_interaction/ResponseHandler.hpp"
#include <QDebug>

ResponseHandler::ResponseHandler()
{}

void ResponseHandler::handleResponse(api::Response response)
{
    switch (response.RequestCommand)
    {
        case api::CommandName::AuthenticationRequest:
            handleRequestCommand_AuthenticationRequest(std::move(response));
            break;

        case api::CommandName::Get_proxy_config:
            handleRequestCommand_Get_proxy_config(std::move(response));
            break;
        
        case api::CommandName::Get_proxy_sessions:
            handleRequestCommand_Get_proxy_sessions(std::move(response));
            break;

        default:
            qWarning() << "Unknown command!";
            break;
    }
}

void ResponseHandler::handleRequestCommand_AuthenticationRequest(api::Response response)
{
    emit Signal_Authentication(response.RequestCommand);
}

void ResponseHandler::handleRequestCommand_Get_proxy_sessions(api::Response response)
{}

void ResponseHandler::handleRequestCommand_Get_proxy_config(api::Response response)
{}