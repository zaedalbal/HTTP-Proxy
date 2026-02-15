#pragma once
#include "ServerInteraction.hpp"

class RequestFacade : public QObject // данный класс нужен формирования запросов и отправки их на сервер
{
    Q_OBJECT

    public:
        RequestFacade(ServerInteraction& ServerInteractionObject);

        void sendAuthenticationRequest(const QString& login, const QString& password);

        void sendGetProxyConfigRequest();

    private:
        ServerInteraction& ServerInteraction_;

};