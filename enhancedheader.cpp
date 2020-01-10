#include "enhancedheader.h"
#include <QLineEdit>
#include <QPainter>
#include <QtDebug>

EnhancedHeader::EnhancedHeader(QWidget *parent):
    QHeaderView (Qt::Horizontal, parent)
{
    connect(this, &QHeaderView::sectionCountChanged, this,
            &EnhancedHeader::setFilterBoxes);
}

bool EnhancedHeader::restoreState(const QByteArray &state)
{
    bool ret = QHeaderView::restoreState(state);
    setFilterBoxes();
    return ret;
}

void EnhancedHeader::setShowFilters(bool on)
{
    showFilters = on;
}

void EnhancedHeader::setFilterBoxes()
{
    while(!editors.isEmpty()) {
        QLineEdit *edit = editors.takeFirst();
        edit->deleteLater();
    }
    int count = this->count();
    for (int i = 0; i < count; i++) {
        QLineEdit *edit = new QLineEdit(this);
        edit->setVisible(false);
        edit->setPlaceholderText("过滤");
        connect(edit, &QLineEdit::textChanged, this, [ = ](QString filter) {
            emit this->filterChanged(i, filter);
        });
        editors.append(edit);
    }
}

// 增加高度用来放置输入框
QSize EnhancedHeader::sizeHint() const
{
    QSize size = QHeaderView::sizeHint();
    if(!editors.isEmpty() && showFilters) {
        int height = editors[0]->sizeHint().height();
        size.setHeight(size.height() + height);
    }
    return size;
}

// 计算文字换行及控件高度
QSize EnhancedHeader::sectionSizeFromContents(int logicalIndex) const
{
    if(this->model() != nullptr && textWrap) {
        ensurePolished();
        auto headerText = this->model()->headerData(logicalIndex, this->orientation(),
                                                    Qt::DisplayRole).toString();
        auto options = this->viewport();
        auto metrics = QFontMetrics(options->font());
        auto maxWidth = this->sectionSize(logicalIndex);
        auto rect = metrics.boundingRect(QRect(50, 50, maxWidth, 5000),
                                         static_cast<int>(this->defaultAlignment() |
                                                          Qt::TextWordWrap |
                                                          Qt::TextExpandTabs)
                                         , headerText, 4);
        QSize size = rect.size();
        size.setHeight(size.height() + 10);
        return size;
    } else {
        return QHeaderView::sectionSizeFromContents(logicalIndex);
    }
}

void EnhancedHeader::paintSection(QPainter *painter, const QRect &rect,
                                  int logicalIndex) const
{
    QRect newRect(rect);
    int height = 0;
    if(!editors.isEmpty() && showFilters) {
        height = editors[0]->sizeHint().height();
    }
    newRect.setHeight(rect.height() - height);

    if(this->model() != nullptr && textWrap) {
        QString headerText = this->model()->headerData(logicalIndex, this->orientation(),
                                                       Qt::DisplayRole).toString();
        /*
         *(1)阻塞model信号，防止setHeaderData时发生死循环
         *(2)model数据置空，以绘制一个没有文字的空白控件，否则默认绘制的文字与自动换行文字会叠成杀人书
         *(3)恢复model数据
         *(4)恢复model信号
         *(5)绘制可换行文字
        */
        painter->save();
        bool oldState = this->model()->blockSignals(true);
        this->model()->setHeaderData(logicalIndex, this->orientation(), "");
        QHeaderView::paintSection(painter, newRect, logicalIndex);
        this->model()->setHeaderData(logicalIndex, this->orientation(), headerText);
        this->model()->blockSignals(oldState);
        int flags = static_cast<int>(this->defaultAlignment() | Qt::TextWordWrap);
        QFont font = painter->font();
        painter->restore();
        painter->setFont(font);
        painter->drawText(QRectF(newRect), flags, headerText);
    } else {
        QHeaderView::paintSection(painter, newRect, logicalIndex);
    }

    adjustPositions(logicalIndex);
}

void EnhancedHeader::adjustPositions(int logicalIndex) const
{
    if(logicalIndex > editors.count() - 1) {
        return;
    }
    int count = editors.count();
    int off = offset();
    for(int i = logicalIndex + 1; i < count; i++) {
        if(sectionPosition(i) - off > this->width()) {
            editors[i]->setVisible(false);
        }
    }
    QLineEdit *edit = editors[logicalIndex];
    edit->setVisible(showFilters);
    int height = edit->sizeHint().height();
    edit->move(sectionPosition(logicalIndex) - off, this->height() - height - 1);
    edit->resize(sectionSize(logicalIndex), height);
}

QString EnhancedHeader::filterText(int logicalIndex)
{
    if(logicalIndex >= 0 && logicalIndex < editors.count()) {
        return editors[logicalIndex]->text();
    }
    return "";
}

void EnhancedHeader::clearFilters()
{
    for (QLineEdit *edit : editors) {
        edit->clear();
    }
}

void EnhancedHeader::setTextWrap(bool on)
{
    textWrap = on;
}
