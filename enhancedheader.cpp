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

void EnhancedHeader::setStretchSection(int logicalIndex)
{
    stretchSection = logicalIndex;
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
    QSize size = QHeaderView::sectionSizeFromContents(logicalIndex);

    if(this->model() != nullptr && textWrap) {
        ensurePolished();
        auto headerText = this->model()->headerData(logicalIndex, this->orientation(),
                                                    Qt::DisplayRole).toString();
        auto options = this->viewport();
        auto metrics = QFontMetrics(options->font());
        auto maxWidth = this->sectionSize(logicalIndex);
        auto rect = metrics.boundingRect(QRect(0, 0, maxWidth, INT_MAX),
                                         static_cast<int>(this->defaultAlignment() |
                                                          Qt::TextWordWrap |
                                                          Qt::TextExpandTabs)
                                         , headerText, 4);
        size = rect.size();
        size.setHeight(size.height() + 10);
        return size;
    }

    return size;
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
        // ***************************************
        /*
        //Q_D((QHeaderView)
        static_cast<QHeaderViewPrivate>(d_ptr);
        if (!rect.isValid())
            return;
        // get the state of the section
        QStyleOptionHeader opt;
        initStyleOption(&opt);
        QStyle::State state = QStyle::State_None;
        if (isEnabled())
            state |= QStyle::State_Enabled;
        if (window()->isActiveWindow())
            state |= QStyle::State_Active;
        if (sectionsClickable()) {
            if (logicalIndex == d->hover)
                state |= QStyle::State_MouseOver;
            if (logicalIndex == d->pressed)
                state |= QStyle::State_Sunken;
            else if (d->highlightSelected) {
                if (d->sectionIntersectsSelection(logicalIndex))
                    state |= QStyle::State_On;
                if (d->isSectionSelected(logicalIndex))
                    state |= QStyle::State_Sunken;
            }

        }
        if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex)
            opt.sortIndicator = (sortIndicatorOrder() == Qt::AscendingOrder)
                                ? QStyleOptionHeader::SortDown : QStyleOptionHeader::SortUp;

        // setup the style options structure
        QVariant textAlignment = d->model->headerData(logicalIndex, d->orientation,
                                                      Qt::TextAlignmentRole);
        opt.rect = rect;
        opt.section = logicalIndex;
        opt.state |= state;
        opt.textAlignment = Qt::Alignment(textAlignment.isValid()
                                          ? Qt::Alignment(textAlignment.toInt())
                                          : d->defaultAlignment);

        opt.iconAlignment = Qt::AlignVCenter;
        opt.text = d->model->headerData(logicalIndex, d->orientation,
                                        Qt::DisplayRole).toString();

        int margin = 2 * style()->pixelMetric(QStyle::PM_HeaderMargin, nullptr, this);

        const Qt::Alignment headerArrowAlignment = static_cast<Qt::Alignment>(style()->styleHint(
                                                                                  QStyle::SH_Header_ArrowAlignment, nullptr, this));
        const bool isHeaderArrowOnTheSide = headerArrowAlignment & Qt::AlignVCenter;
        if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex && isHeaderArrowOnTheSide)
            margin += style()->pixelMetric(QStyle::PM_HeaderMarkSize, nullptr, this);

        const QVariant variant = d->model->headerData(logicalIndex, d->orientation,
                                                      Qt::DecorationRole);
        opt.icon = qvariant_cast<QIcon>(variant);
        if (opt.icon.isNull())
            opt.icon = qvariant_cast<QPixmap>(variant);
        if (!opt.icon.isNull()) // see CT_HeaderSection
            margin += style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this) +
                      style()->pixelMetric(QStyle::PM_HeaderMargin, nullptr, this);

        if (d->textElideMode != Qt::ElideNone) {
            const QRect textRect = style()->subElementRect(QStyle::SE_HeaderLabel, &opt, this);
            opt.text = opt.fontMetrics.elidedText(opt.text, d->textElideMode, textRect.width() - margin);
        }

        QVariant foregroundBrush = d->model->headerData(logicalIndex, d->orientation,
                                                        Qt::ForegroundRole);
        if (foregroundBrush.canConvert<QBrush>())
            opt.palette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(foregroundBrush));

        QPointF oldBO = painter->brushOrigin();
        QVariant backgroundBrush = d->model->headerData(logicalIndex, d->orientation,
                                                        Qt::BackgroundRole);
        if (backgroundBrush.canConvert<QBrush>()) {
            opt.palette.setBrush(QPalette::Button, qvariant_cast<QBrush>(backgroundBrush));
            opt.palette.setBrush(QPalette::Window, qvariant_cast<QBrush>(backgroundBrush));
            painter->setBrushOrigin(opt.rect.topLeft());
        }

        // the section position
        int visual = visualIndex(logicalIndex);
        Q_ASSERT(visual != -1);
        bool first = d->isFirstVisibleSection(visual);
        bool last = d->isLastVisibleSection(visual);
        if (first && last)
            opt.position = QStyleOptionHeader::OnlyOneSection;
        else if (first)
            opt.position = d->reverse() ? QStyleOptionHeader::End : QStyleOptionHeader::Beginning;
        else if (last)
            opt.position = d->reverse() ? QStyleOptionHeader::Beginning : QStyleOptionHeader::End;
        else
            opt.position = QStyleOptionHeader::Middle;
        opt.orientation = d->orientation;
        // the selected position
        bool previousSelected = d->isSectionSelected(this->logicalIndex(visual - 1));
        bool nextSelected =  d->isSectionSelected(this->logicalIndex(visual + 1));
        if (previousSelected && nextSelected)
            opt.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
        else if (previousSelected)
            opt.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
        else if (nextSelected)
            opt.selectedPosition = QStyleOptionHeader::NextIsSelected;
        else
            opt.selectedPosition = QStyleOptionHeader::NotAdjacent;
        // draw the section
        style()->drawControl(QStyle::CE_Header, &opt, painter, this);

        painter->setBrushOrigin(oldBO);
        // ***************************************
        */


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
