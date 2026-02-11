#include <QObject>
#include "api.hpp"

class ResponseHandler : public QObject // при взаимодействии с этим классом использовать std::move() !!!
{
    Q_OBJECT

    public:
        ResponseHandler();
        
    // данная структура нужна на будущее, если в информации о сессии будет что то ещё
    struct Session
    {
        uint32_t ip_;
    };
    
    signals:
        void Signal_Authentication(api::CommandName command);

        void Signal_Get_proxy_sessions(QVector<Session> sessions);

        void Signal_Get_proxy_config(api::Proxy_Settings config);

    private:
        void ResponseCommand_AuthenticationRequest(api::Response response);

        void ResponseCommand_Get_proxy_sessions(api::Response response);

        void ResponseCommand_Get_proxy_config(api::Response response);

};