#include "server_interaction/ServerInteraction.hpp"

ServerInteraction::ServerInteraction()
{
    connect(&socket_, &QSslSocket::bytesWritten, this, &ServerInteraction::onBytesWritten);
    connect(&socket_, &QSslSocket::readyRead, this, &ServerInteraction::onReadyRead);

    connect(&socket_, &QSslSocket::connected, this, [this](){emit socket_.startClientEncryption();});
    connect(&socket_, &QSslSocket::encrypted, this, [this](){emit successfulConnect();});

    // В БУДУЩЕМ УБРАТЬ ЭТОТ CONNECT, КОГДА БУДЕТ ДОБАВЛЯТЬСЯ ПОДДЕРЖКА СЕРТИФИКАТОВ
    // (ПОКА ЧТО ОШИБКИ СВЯЗАННЫЕ С СЕРТИФИКАТЫМИ ПРОСТО ИГНОРИРУЮТСЯ)
    connect(&socket_, QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors), this,
    [this](const QList<QSslError> &errors)
    {
        for(const auto &err : errors)
            qWarning() << "SSL Error: " << err.errorString();
        socket_.ignoreSslErrors();
    });

    connect(&socket_, &QSslSocket::disconnected, this, [this](){emit disconnected();}); 
}

void ServerInteraction::connectToServer(QString host, quint16 port)
{
    socket_.connectToHost(host, port);
}

void ServerInteraction::sendRequest(api::Request request)
{
    if(socket_.state() != QAbstractSocket::ConnectedState)
    {
        qWarning() << "Socket not connected";
        return;
    }
    qint32 total_size = sizeof(api::RequestHeader) + request.data_size;
    QByteArray buffer;
    buffer.resize(total_size);
    char* offset = buffer.data();
    std::memcpy(offset, &request, sizeof(api::RequestHeader));
    offset += sizeof(api::RequestHeader);
    if(request.data_size > 0)
    {
        std::memcpy(offset, request.data.get(), request.data_size);
    }
    remainingData_ = buffer;
    auto written = socket_.write(buffer);
    if(written < remainingData_.size())
        remainingData_ = remainingData_.mid(written);
    else
        remainingData_.clear();
}

void ServerInteraction::onBytesWritten(qint64 bytes)
{
    if(!remainingData_.isEmpty())
    {
        qint64 written = socket_.write(remainingData_);
        if(written < remainingData_.size())
            remainingData_ = remainingData_.mid(written);
        else
            remainingData_.clear(); 
    }
}

void ServerInteraction::onReadyRead()
{
    buffer_.append(socket_.readAll());
    while(true)
    {
        if(buffer_.size() < static_cast<int>(sizeof(api::RequestHeader)))
            return;
        api::ResponseHeader header;
        std::memcpy(&header, buffer_.constData(), sizeof(header));
        qsizetype total_size = sizeof(api::ResponseHeader) + header.data_size;
        if(buffer_.size() < total_size)
            return;

        auto response = std::make_shared<api::Response>(header.data_size);
        response->ProxyStatus = header.ProxyStatus;
        response->RequestFailed = header.RequestFailed;
        response->id = header.id;
        response->ResponseCommand = header.ResponseCommand;
        response->RequestCommand = header.RequestCommand;
        response->isChunckedResponse = header.isChunckedResponse;
        response->data_size = header.data_size;
        if(response->data_size > 0)
            std::memcpy(response->data.get(), buffer_.constData() + sizeof(api::ResponseHeader), header.data_size);
        buffer_.remove(0, total_size);
        emit responseReceived(response);
    }
}

void ServerInteraction::disconnect()
{
    socket_.close();
}

QAbstractSocket::SocketState ServerInteraction::state()
{
    return socket_.state();
}