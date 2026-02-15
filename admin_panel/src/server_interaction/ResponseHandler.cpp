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

void ResponseHandler::handleRequestCommand_Get_proxy_config(std::shared_ptr<api::Response> response)
{
    QString config;
    api::Proxy_Settings raw_config;
    std::memcpy(&raw_config, response->data.get(), sizeof(api::Proxy_Settings));
    config.append(QString("Blacklist on: %1\n").arg(raw_config.blacklist_on ? "true" : "false"));
    config.append(QString("Log on: %1\n").arg(raw_config.log_on ? "true" : "false"));
    config.append(QString("Admin_panel_on: %1\n").arg(raw_config.admin_panel_on ? "true" : "false"));
    config.append(QString("Port: %1\n").arg(raw_config.port));
    config.append(QString("Admin_panel_port: %1\n").arg(raw_config.admin_panel_port));
    config.append(QString("Max_connections: %1\n").arg(raw_config.max_connections));
    config.append(QString("Timeout milliseconds: %1\n").arg(raw_config.timeout_milliseconds));
    config.append(QString("Log_file_size_bytes: %1\n").arg(raw_config.log_file_size_bytes));
    config.append(QString("Max_bindwidth_per_sec: %1\n").arg(raw_config.max_bandwidth_per_sec));
    emit Signal_Get_proxy_config(config);
}

void ResponseHandler::handleRequestCommand_Get_proxy_sessions(std::shared_ptr<api::Response> response)
{}