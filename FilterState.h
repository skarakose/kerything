#ifndef KERYTHING_FILTERSTATE_H
#define KERYTHING_FILTERSTATE_H

#include <QString>
#include <cstdint>

enum class FilterOperator {
    None,
    GreaterThan,
    LessThan,
    Equals,
    Between
};

struct PathFilterState {
    bool active = false;
    QString text;
};

struct SizeFilterState {
    bool active = false;
    FilterOperator op = FilterOperator::None;
    uint64_t val1 = 0; // in bytes
    uint64_t val2 = 0; // in bytes, used for Between
};

struct DateFilterState {
    bool active = false;
    FilterOperator op = FilterOperator::None;
    uint64_t val1 = 0; // unix epoch seconds
    uint64_t val2 = 0; // unix epoch seconds, used for Between
};

struct FilterState {
    PathFilterState path;
    SizeFilterState size;
    DateFilterState date;
    
    bool isActive() const {
        return path.active || size.active || date.active;
    }
};

#endif // KERYTHING_FILTERSTATE_H
