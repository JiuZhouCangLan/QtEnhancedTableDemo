#include "enhancedstandarditemmodel.h"
#include <QTextDocument>
#include <QtDebug>

EnhancedStandardItemModel::EnhancedStandardItemModel(QObject *parent):
    QStandardItemModel(parent)
{}

EnhancedStandardItemModel::EnhancedStandardItemModel(int rows, int columns,
                                                     QObject *parent): QStandardItemModel (rows, columns, parent)
{}

Qt::ItemFlags EnhancedStandardItemModel::flags(const QModelIndex &index) const
{
    if(!index.isValid()) {
        return QStandardItemModel::flags(index);
    }

    if(checkState.contains(index)) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }
    return QStandardItemModel::flags(index);
}

QVariant EnhancedStandardItemModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QStandardItemModel::data(index, role);
    }

    if(role == Qt::CheckStateRole && checkState.contains(index)) {
        return checkState.value(index, Qt::Unchecked);
    }

    return QStandardItemModel::data(index, role);
}

bool EnhancedStandardItemModel::setData(const QModelIndex &index,
                                        const QVariant &value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    if(role == Qt::CheckStateRole) {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        checkState.insert(index, state);
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    } else if (role == HtmlRole) {
        QTextDocument doc;
        doc.setHtml(value.toString());
        QStandardItemModel::setData(index, doc.toPlainText());
    }

    return QStandardItemModel::setData(index, value, role);
}
