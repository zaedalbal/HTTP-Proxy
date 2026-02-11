#include <QObject>
#include "api.hpp"

class ResponseHandler : public QObject // при взаимодействии с этим классом использовать std::move() !!!
{
    Q_OBJECT

    public:
        ResponseHandler();

        void handleResponse(api::Response response);
        
    // данная структура нужна на будущее, если в информации о сессии будет что то ещё
    struct Session
    {
        QString ip_;
    };
    
    signals:
        void Signal_Authentication(api::CommandName command);

        void Signal_Get_proxy_sessions(QVector<Session> sessions);

        void Signal_Get_proxy_config(api::Proxy_Settings config);

    private:
        void handleRequestCommand_AuthenticationRequest(api::Response response);

        void handleRequestCommand_Get_proxy_sessions(api::Response response);

        void handleRequestCommand_Get_proxy_config(api::Response response);

};