#include "server_interaction/ResponseHandler.hpp"
#include <QDebug>
#include <iostream>

ResponseHandler::ResponseHandler()
{}

void ResponseHandler::handleResponse(std::shared_ptr<api::Response> response)
{
    std::cout << "handleResponse called\n";
    //std::cout << "RequestCommand raw value: " << static_cast<int>(response->RequestCommand) << "\n";
    switch (response->RequestCommand)
    {
        case api::CommandName::AuthenticationRequest:
            handleRequestCommand_AuthenticationRequest(response);
            break;

        case api::CommandName::Get_proxy_config:
            handleRequestCommand_Get_proxy_config(response);
            break;
        
        case api::CommandName::Get_proxy_sessions:
            handleRequestCommand_Get_proxy_sessions(response);
            break;

        default:
            qWarning() << "Unknown command!";
            break;
    }
}

void ResponseHandler::handleRequestCommand_AuthenticationRequest(std::shared_ptr<api::Response> response)
{
    if(response->ResponseCommand == api::CommandName::AuthenticationResponseSuccess)
        emit Signal_AuthenticationSuccessful();
    else
        emit Signal_AuthenticationUnsuccessful();
}

void ResponseHandler::handleRequestCommand_Get_proxy_sessions(std::shared_ptr<api::Response> response)
{}

void ResponseHandler::handleRequestCommand_Get_proxy_config(std::shared_ptr<api::Response> response)
{}