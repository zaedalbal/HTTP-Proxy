#pragma once
#include <QObject>
#include "api.hpp"
#include "pages/ProxySessionsPage/structSessionInfo.hpp"

class ResponseHandler : public QObject // при взаимодействии с этим классом использовать std::move() !!!
{
    Q_OBJECT

    public:
        ResponseHandler();

        void handleResponse(std::shared_ptr<api::Response> response);
        
    signals:
        void Signal_AuthenticationSuccessful();

        void Signal_AuthenticationUnsuccessful();

        void Signal_Get_proxy_sessions(std::vector<SessionInfo> sessions);

        void Signal_Get_proxy_config(QString config);

    private:
        void handleRequestCommand_AuthenticationRequest(std::shared_ptr<api::Response> response);

        void handleRequestCommand_Get_proxy_sessions(std::shared_ptr<api::Response> response);

        void handleRequestCommand_Get_proxy_config(std::shared_ptr<api::Response> response);

};