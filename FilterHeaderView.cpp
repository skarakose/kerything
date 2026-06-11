#include "FilterHeaderView.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>

FilterHeaderView::FilterHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setSectionsClickable(true);
    setMouseTracking(true);
    
    // Using standard KDE/freedesktop icons for filtering
    m_filterIcon = QIcon::fromTheme("view-filter");
    // Fallback if the theme doesn't have it
    if (m_filterIcon.isNull()) {
        m_filterIcon = style()->standardIcon(QStyle::SP_FileDialogListView);
    }
    
    // An active icon could be highlighted or standard icon depending on what is available
    m_filterActiveIcon = QIcon::fromTheme("view-filter", m_filterIcon); 
}

void FilterHeaderView::setFilterActive(int logicalIndex, bool active) {
    m_activeFilters[logicalIndex] = active;
    viewport()->update();
}

QRect FilterHeaderView::getFilterRect(const QRect &sectionRect) const {
    // 16x16 icon, vertically centered, left aligned with 4px margin
    int size = 16;
    int margin = 4;
    return QRect(sectionRect.left() + margin,
                 sectionRect.top() + (sectionRect.height() - size) / 2,
                 size, size);
}

void FilterHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const {
    // Let the base class paint the standard header (text, sort indicator, etc.)
    QHeaderView::paintSection(painter, rect, logicalIndex);

    // We only add filter icons to columns 1 (Path), 2 (Size), and 3 (Date)
    if (logicalIndex >= 1 && logicalIndex <= 3) {
        QRect filterRect = getFilterRect(rect);
        
        bool active = m_activeFilters.value(logicalIndex, false);
        bool hovering = (m_hoveredSection == logicalIndex && m_hoveringFilter);

        // Optional: draw a background if hovering over the filter icon
        if (hovering) {
            painter->fillRect(filterRect.adjusted(-2, -2, 2, 2), QColor(128, 128, 128, 64));
        }

        QIcon icon = active ? m_filterActiveIcon : m_filterIcon;
        // If active, maybe paint it blue? We can just use QIcon::Active mode
        QIcon::Mode mode = active ? QIcon::Active : QIcon::Normal;
        
        icon.paint(painter, filterRect, Qt::AlignCenter, mode);
    }
}

void FilterHeaderView::mousePressEvent(QMouseEvent *event) {
    int logicalIndex = logicalIndexAt(event->pos());
    if (logicalIndex >= 1 && logicalIndex <= 3) {
        QRect sectionRect = QRect(sectionViewportPosition(logicalIndex), 0, sectionSize(logicalIndex), height());
        QRect filterRect = getFilterRect(sectionRect);

        if (filterRect.contains(event->pos())) {
            QPoint globalPos = viewport()->mapToGlobal(filterRect.bottomLeft());
            emit filterClicked(logicalIndex, globalPos);
            return; // Consume the event so it doesn't trigger sorting
        }
    }
    
    QHeaderView::mousePressEvent(event);
}

void FilterHeaderView::mouseMoveEvent(QMouseEvent *event) {
    int logicalIndex = logicalIndexAt(event->pos());
    bool hoveringFilter = false;

    if (logicalIndex >= 1 && logicalIndex <= 3) {
        QRect sectionRect = QRect(sectionViewportPosition(logicalIndex), 0, sectionSize(logicalIndex), height());
        QRect filterRect = getFilterRect(sectionRect);
        
        if (filterRect.contains(event->pos())) {
            hoveringFilter = true;
        }
    }

    if (m_hoveredSection != logicalIndex || m_hoveringFilter != hoveringFilter) {
        m_hoveredSection = logicalIndex;
        m_hoveringFilter = hoveringFilter;
        viewport()->update();
    }

    QHeaderView::mouseMoveEvent(event);
}

void FilterHeaderView::leaveEvent(QEvent *event) {
    m_hoveredSection = -1;
    m_hoveringFilter = false;
    viewport()->update();
    QHeaderView::leaveEvent(event);
}
