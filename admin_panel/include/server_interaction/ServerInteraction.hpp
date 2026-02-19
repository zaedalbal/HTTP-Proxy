#pragma once
#include <QTcpSocket>
#include <QSslSocket>
#include "api.hpp"

class ServerInteraction : public QObject
{
    Q_OBJECT

    public:
        ServerInteraction();

        void connectToServer(QString host, quint16 port);

        void sendRequest(api::Request request);

        void disconnect();

        QAbstractSocket::SocketState state();
    signals:
        void responseReceived(std::shared_ptr<api::Response> response);

        void successfulConnect();

        void disconnected();

    private slots:
        void onReadyRead();

        void onBytesWritten(qint64 bytes);

    private:
        QByteArray buffer_;

        QByteArray remainingData_;

        QSslSocket socket_;

};