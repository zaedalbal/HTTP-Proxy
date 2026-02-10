#include "server_interaction/ServerInteraction.hpp"

ServerInteraction::ServerInteraction()
{
    connect(&socket_, &QTcpSocket::bytesWritten, this, &ServerInteraction::onBytesWritten);
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