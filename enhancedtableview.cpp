#include "enhancedtableview.h"
#include <QTextDocument>
#include <QApplication>
#include <QAbstractItemModel>
#include <QPainter>
#include <QtDebug>
#include <QAbstractTextDocumentLayout>

EnhancedTableView::EnhancedTableView(QWidget *parent): QTableView (parent)
{
    setMouseTracking(true);

    auto header = new EnhancedHeader(this);
    auto state = horizontalHeader()->saveState();
    header->restoreState(state);
    setHorizontalHeader(header);

    connect(header, &EnhancedHeader::filterChanged, this,
            &EnhancedTableView::filterData);
}

void EnhancedTableView::mousePressEvent(QMouseEvent *event)
{
    QTableView::mousePressEvent(event);
    if(event->button() == Qt::LeftButton) {
        _mousePressAnchor = anchorAt(event->pos());
    }
}

void EnhancedTableView::mouseMoveEvent(QMouseEvent *event)
{
    QString anchor = anchorAt(event->pos());

    if(_mousePressAnchor != anchor) {
        _mousePressAnchor.clear();
    }

    if(_lastHoveredAnchor != anchor) {
        _lastHoveredAnchor = anchor;
        if(!_lastHoveredAnchor.isEmpty()) {
            setCursor(Qt::PointingHandCursor);
            emit linkHovered(_lastHoveredAnchor);
        } else {
            setCursor(Qt::ArrowCursor);
            emit linkUnhovered();
        }
    }
}

void EnhancedTableView::mouseReleaseEvent(QMouseEvent *event)
{
    QTableView::mouseReleaseEvent(event);
    if(!_mousePressAnchor.isEmpty()) {
        QString anchor = anchorAt(event->pos());

        if(anchor == _mousePressAnchor) {
            emit linkActivated(_mousePressAnchor);
        } else {
            _mousePressAnchor.clear();
        }
    }
}

QString EnhancedTableView::anchorAt(const QPoint &pos) const
{
    QModelIndex index = indexAt(pos);
    if(index.isValid()) {
        int width = columnWidth(index.column()) - 1;
        JumpDelegate *delegate = dynamic_cast<JumpDelegate *>(itemDelegate(index));
        if(delegate != nullptr) {
            QRect itemRect = visualRect(index);
            QPoint relativeClickPosition = pos - itemRect.topLeft();
            QString html = model()->data(index).toString();

            QTextDocument doc;
            doc.setDefaultFont(this->font());
            doc.setHtml(html);
            doc.setTextWidth(width);

            auto textLayout = doc.documentLayout();
            Q_ASSERT(textLayout != nullptr);
            return textLayout->anchorAt(relativeClickPosition);
        }
    }
    return  QString();
}

void EnhancedTableView::setShowFilters(bool on)
{
    auto header = dynamic_cast<EnhancedHeader*>(horizontalHeader());
    if(header != nullptr) {
        header->setShowFilters(on);
    }
}

void EnhancedTableView::setHorizontalHeaderWrap(bool on)
{
    auto header = dynamic_cast<EnhancedHeader*>(horizontalHeader());
    if(header != nullptr) {
        header->setTextWrap(on);
    }
}

void EnhancedTableView::filterData(int col, QString key)
{
    if(key == "") {
        filterMap.remove(col);
    } else {
        filterMap.insert(col, key);
    }

    QAbstractItemModel *model = this->model();
    if(model != nullptr) {
        int colCount = model->columnCount();
        int rowCount = model->rowCount();

        for (int i = 0; i < rowCount; i++) {
            bool hasAllKey = true;
            for (int j = 0; j < colCount; j++) {
                QModelIndex index = model->index(i, j);
                QString data = model->data(index).toString();

                if(!data.contains(filterMap.value(j), Qt::CaseInsensitive)) {
                    hasAllKey = false;
                    break;
                }
            }
            setRowHidden(i, !hasAllKey);
        }
    }
}

void EnhancedTableView::setModel(QAbstractItemModel *model)
{
    int oldColCount = 0;
    if(this->model() != nullptr) {
        oldColCount = this->model()->columnCount();

    }

    QTableView::setModel(model);

    int newColCount = model->columnCount();
    for(int i = oldColCount; i < newColCount; i++) {
        JumpDelegate *delegate = new JumpDelegate(this);
        this->setItemDelegateForColumn(i, delegate);
    }
}

JumpDelegate::JumpDelegate(QObject *parent): QStyledItemDelegate (parent)
{}

QSize JumpDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QVariant text = index.model()->data(index);

    QTextDocument doc;
    const QWidget *widget = option.widget;
    doc.setDefaultFont(widget->font());

    QRect textRect = QApplication::style()->
                     subElementRect(QStyle::SE_ItemViewItemText, &option);
    doc.setTextWidth(textRect.width());
    doc.setHtml(index.model()->data(index).toString());
    return doc.size().toSize();
}

void JumpDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text = "";

    // 取消选中虚线框
    if(opt.state.testFlag(QStyle::State_HasFocus)) {
        opt.state = opt.state ^ QStyle::State_HasFocus;
    }
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    // 绘制超链接
    QTextDocument doc;
    doc.setDefaultFont(widget->font());
    doc.setHtml(index.model()->data(index).toString());
    QAbstractTextDocumentLayout::PaintContext paintContext;
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt);
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));

    doc.setTextWidth(textRect.width());
    doc.documentLayout()->draw(painter, paintContext);

    painter->restore();
}
