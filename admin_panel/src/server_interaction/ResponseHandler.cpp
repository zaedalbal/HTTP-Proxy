#include "server_interaction/ResponseHandler.hpp"
#include <QDebug>
#include <iostream>
#include <QHostAddress>

ResponseHandler::ResponseHandler()
{}

void ResponseHandler::handleResponse(std::shared_ptr<api::Response> response)
{
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
        
        case api::CommandName::Get_proxy_log:
            handleRequestCommand_Get_proxy_log(response);
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
{
    struct Session
    {
        quint32 ip_;
    };
    std::vector<SessionInfo> sessions;
    if(response->RequestFailed || response->data_size == 0)
    {
        emit Signal_Get_proxy_sessions(sessions);
        return;
    }
    if(response->data_size % sizeof(Session) != 0)
    {
        emit Signal_Get_proxy_sessions(sessions);
        return;
    }
    size_t count = response->data_size / sizeof(Session);
    const Session* rawSessions = reinterpret_cast<const Session*>(response->data.get());
    sessions.reserve(count);
    for(size_t i = 0; i < count; ++i)
    {
        QHostAddress addr(rawSessions[i].ip_);
        QString ipString = addr.toString();
        SessionInfo session;
        session.ip_ = ipString;
        sessions.push_back(session);
    }
    emit Signal_Get_proxy_sessions(sessions);
}

void ResponseHandler::handleRequestCommand_Get_proxy_log(std::shared_ptr<api::Response> response)
{
    std::string log_std_string;
    log_std_string.resize(response->data_size);
    std::memcpy(log_std_string.data(), response->data.get(), response->data_size);
    QString log = QString::fromStdString(log_std_string);
    emit Signal_Get_proxy_log(log);
}