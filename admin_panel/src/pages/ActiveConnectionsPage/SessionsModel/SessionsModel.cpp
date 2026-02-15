#include "pages/ProxySessionsPage/SessionsModel/SessionsModel.hpp"

SessionModel::SessionModel(QObject* parent) : QAbstractTableModel(parent)
{}

void SessionModel::setSessions(std::vector<SessionInfo> sessions)
{
    beginResetModel();
    sessions_ = std::move(sessions);
    endResetModel();
}

void SessionModel::addSession(const SessionInfo& session)
{
    const int row = static_cast<int>(sessions_.size());

    beginInsertRows(QModelIndex(), row, row);
    sessions_.push_back(session);
    endInsertRows();
}

void SessionModel::removeSession(int row)
{
    if(row < 0 || row >= static_cast<int>(sessions_.size()))
        return;
    
    beginRemoveRows(QModelIndex(), row, row);
    sessions_.erase(sessions_.begin() + row);
    endRemoveRows();
}

int SessionModel::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(sessions_.size());
}

int SessionModel::columnCount(const QModelIndex& parent) const
{
    return ColumnCount;
}

QVariant SessionModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid())
        return{};
    if(role != Qt::DisplayRole)
        return{};

    const SessionInfo& session = sessions_[index.row()];
    switch(index.column())
    {
        case Ip:
            return session.ip_;

        default:
            return{};
    }
    return{};
}

QVariant SessionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return{};
    if(orientation == Qt::Horizontal)
    {
        switch(section)
        {
            case Ip:
                return "IP";

            default:
                return{};
        }
    }
    return{};
}