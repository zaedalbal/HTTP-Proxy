#pragma once
#include <QObject>
#include "api.hpp"

class ResponseHandler : public QObject // при взаимодействии с этим классом использовать std::move() !!!
{
    Q_OBJECT

    public:
        ResponseHandler();

        void handleResponse(std::shared_ptr<api::Response> response);
        
    // данная структура нужна на будущее, если в информации о сессии будет что то ещё
    struct Session
    {
        QString ip_;
    };
    
    signals:
        void Signal_AuthenticationSuccessful();

        void Signal_AuthenticationUnsuccessful();

        void Signal_Get_proxy_sessions(QVector<Session> sessions);

        void Signal_Get_proxy_config(QString config);

    private:
        void handleRequestCommand_AuthenticationRequest(std::shared_ptr<api::Response> response);

        void handleRequestCommand_Get_proxy_sessions(std::shared_ptr<api::Response> response);

        void handleRequestCommand_Get_proxy_config(std::shared_ptr<api::Response> response);

};