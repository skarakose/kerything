#ifndef KERYTHING_FILTERHEADERVIEW_H
#define KERYTHING_FILTERHEADERVIEW_H

#include <QHeaderView>
#include <QIcon>
#include <QMap>

class FilterHeaderView : public QHeaderView {
    Q_OBJECT
public:
    explicit FilterHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    void setFilterActive(int logicalIndex, bool active);

signals:
    void filterClicked(int logicalIndex, QPoint globalPos);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QRect getFilterRect(const QRect &sectionRect) const;

    QIcon m_filterIcon;
    QIcon m_filterActiveIcon;
    QMap<int, bool> m_activeFilters;
    int m_hoveredSection = -1;
    bool m_hoveringFilter = false;
};

#endif // KERYTHING_FILTERHEADERVIEW_H
