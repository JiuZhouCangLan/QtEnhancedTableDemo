#ifndef ENHANCEDTABLEVIEW_H
#define ENHANCEDTABLEVIEW_H

#include <QTableView>
#include <QMouseEvent>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include "enhancedheader.h"


class EnhancedTableView: public QTableView
{
    Q_OBJECT
public:
    EnhancedTableView(QWidget *parent = nullptr);
    void setShowFilters(bool on);
    void setHorizontalHeaderWrap(bool on);
    void setModel(QAbstractItemModel *model) override;

signals:
    void linkActivated(QString link);
    void linkHovered(QString link);
    void linkUnhovered();

private slots:
    void filterData(int col, QString key);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QString anchorAt(const QPoint &pos) const;

    QString _mousePressAnchor;
    QString _lastHoveredAnchor;
    QHash<int, QString> filterMap;
};

class JumpDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    JumpDelegate(QObject *parent = nullptr);
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

};

#endif // ENHANCEDTABLEVIEW_H
