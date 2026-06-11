#ifndef KERYTHING_FILTERDIALOGS_H
#define KERYTHING_FILTERDIALOGS_H

#include <QWidget>
#include "FilterState.h"

namespace FilterDialogs {
    bool showPathFilterDialog(QWidget* parent, PathFilterState& state, const QPoint& pos);
    bool showSizeFilterDialog(QWidget* parent, SizeFilterState& state, const QPoint& pos);
    bool showDateFilterDialog(QWidget* parent, DateFilterState& state, const QPoint& pos);
}

#endif // KERYTHING_FILTERDIALOGS_H
