#pragma once

#include <QAbstractTableModel>
#include <vector>
#include "pages/ProxySessionsPage/structSessionInfo.hpp"

// данный класс нужен чтобы в будущем не возникло проблем при добавлении новой информации в SessionInfo

class SessionModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        explicit SessionModel(QObject* parent = nullptr);

        void setSessions(std::vector<SessionInfo> sessions);
        void addSession(const SessionInfo& session);
        void removeSession(int row);

        const SessionInfo& sessionAt(int row) const;

        // методы для Qt
        int rowCount(const QModelIndex& parent = QModelIndex()) const override; // сколько строк в таблице
        int columnCount(const QModelIndex& parent = QModelIndex()) const override; // сколько колонок в таблице
        QVariant data(const QModelIndex& index, int role) const override; // возврат значения в конкретной ячейке
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override; // возврат заголовков столбцов и строк

    private:
        std::vector<SessionInfo> sessions_;

        enum Columns
        {
            Ip = 0,
            ColumnCount
        };
};