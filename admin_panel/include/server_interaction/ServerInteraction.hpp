#pragma once
#include <QTcpSocket>
#include "api.hpp"

class ServerInteraction : public QObject
{
    Q_OBJECT

    public:
        ServerInteraction();

        void connectToServer(QString host, quint16 port);

        void sendRequest(api::Request request);

        void setResponseHandler(std::function<void(const api::Response)> handler);

    signals:
        void successfulConnect();

    private slots:
        void onReadyRead();

        void onBytesWritten(qint64 bytes);

    private:
        QByteArray buffer_;

        QByteArray remainingData_;

        QTcpSocket socket_;

        std::function<void(const api::Response)> response_handler_;

};