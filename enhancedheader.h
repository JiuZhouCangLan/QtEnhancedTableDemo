#ifndef ENHANCEDHEADER_H
#define ENHANCEDHEADER_H

#include <QHeaderView>
#include <QMouseEvent>

class EnhancedHeader: public QHeaderView
{
    Q_OBJECT
public:
    EnhancedHeader(QWidget *parent = nullptr);
    void setShowFilters(bool on);
    void setTextWrap(bool on);
    QString filterText(int logicalIndex);
    void clearFilters();
    QSize sectionSizeFromContents(int logicalIndex) const override;
    bool restoreState(const QByteArray &state);
    void setStretchSection(int logicalIndex);

signals:
    void filterChanged(int logicalIndex, QString filter);

private slots:
    void adjustPositions(int logicalIndex) const;
    void setFilterBoxes();

private:
    QList<QLineEdit*> editors;

    QSize sizeHint() const override;
    void paintSection(QPainter *painter, const QRect &rect,
                      int logicalIndex) const override;
    bool showFilters = true;
    bool textWrap = true;
    int stretchSection = -1;
};

#endif // ENHANCEDHEADER_H
