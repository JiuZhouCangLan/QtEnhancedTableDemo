#ifndef ENHANCEDSTANDARDITEMMODEL_H
#define ENHANCEDSTANDARDITEMMODEL_H

#include <QStandardItemModel>

class EnhancedStandardItemModel: public QStandardItemModel
{
    Q_OBJECT
public:
    EnhancedStandardItemModel(QObject *parent = nullptr);
    EnhancedStandardItemModel(int rows, int columns, QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    inline void setCellIsCheckable(QModelIndex index)
    {
        checkState.insert(index, Qt::Unchecked);
    }
    enum role{HtmlRole = Qt::UserRole + 200};

private:
    QList<int> checkColumns;
    QHash<QModelIndex, Qt::CheckState> checkState;
};

#endif // ENHANCEDSTANDARDITEMMODEL_H
