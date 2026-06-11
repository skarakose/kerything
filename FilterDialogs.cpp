#include "FilterDialogs.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QStackedWidget>
#include <cmath>

namespace FilterDialogs {

    bool showPathFilterDialog(QWidget* parent, PathFilterState& state, const QPoint& pos) {
        QDialog dlg(parent);
        dlg.setWindowFlags(Qt::Popup);
        dlg.setWindowTitle("Path Filter");
        dlg.move(pos);

        auto* layout = new QVBoxLayout(&dlg);
        
        auto* activeCheck = new QCheckBox("Enable Path Filter", &dlg);
        activeCheck->setChecked(state.active);
        layout->addWidget(activeCheck);

        auto* input = new QLineEdit(&dlg);
        input->setPlaceholderText("Enter path text...");
        input->setText(state.text);
        input->setEnabled(state.active);
        layout->addWidget(input);

        QObject::connect(activeCheck, &QCheckBox::toggled, input, &QWidget::setEnabled);

        auto* btnLayout = new QHBoxLayout();
        auto* okBtn = new QPushButton("OK");
        auto* cancelBtn = new QPushButton("Cancel");
        btnLayout->addStretch();
        btnLayout->addWidget(okBtn);
        btnLayout->addWidget(cancelBtn);
        layout->addLayout(btnLayout);

        QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
        QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

        if (dlg.exec() == QDialog::Accepted) {
            state.active = activeCheck->isChecked();
            state.text = input->text();
            return true;
        }
        return false;
    }

    struct SizeInput {
        QDoubleSpinBox* spin;
        QComboBox* unitCombo;

        SizeInput(QWidget* parent, uint64_t bytes = 0) {
            spin = new QDoubleSpinBox(parent);
            spin->setRange(0, 999999999999.0);
            spin->setDecimals(2);

            unitCombo = new QComboBox(parent);
            unitCombo->addItems({"B", "KB", "MB", "GB", "TB", "PB"});

            double val = static_cast<double>(bytes);
            int unitIdx = 0;
            while (val >= 1024.0 && unitIdx < 5) {
                val /= 1024.0;
                unitIdx++;
            }
            spin->setValue(val);
            unitCombo->setCurrentIndex(unitIdx);
        }

        uint64_t getBytes() const {
            double val = spin->value();
            int idx = unitCombo->currentIndex();
            uint64_t mult = 1;
            for (int i = 0; i < idx; ++i) mult *= 1024;
            return static_cast<uint64_t>(val * mult);
        }
    };

    bool showSizeFilterDialog(QWidget* parent, SizeFilterState& state, const QPoint& pos) {
        QDialog dlg(parent);
        dlg.setWindowFlags(Qt::Popup);
        dlg.setWindowTitle("Size Filter");
        dlg.move(pos);

        auto* layout = new QVBoxLayout(&dlg);
        
        auto* activeCheck = new QCheckBox("Enable Size Filter", &dlg);
        activeCheck->setChecked(state.active);
        layout->addWidget(activeCheck);

        auto* opCombo = new QComboBox(&dlg);
        opCombo->addItem("Greater Than", static_cast<int>(FilterOperator::GreaterThan));
        opCombo->addItem("Less Than", static_cast<int>(FilterOperator::LessThan));
        opCombo->addItem("Equals", static_cast<int>(FilterOperator::Equals));
        opCombo->addItem("Between", static_cast<int>(FilterOperator::Between));
        
        int opIdx = 0;
        if (state.op == FilterOperator::GreaterThan) opIdx = 0;
        else if (state.op == FilterOperator::LessThan) opIdx = 1;
        else if (state.op == FilterOperator::Equals) opIdx = 2;
        else if (state.op == FilterOperator::Between) opIdx = 3;
        opCombo->setCurrentIndex(opIdx);

        auto* inputLayout = new QVBoxLayout();
        
        auto* singleInputWidget = new QWidget();
        auto* singleLayout = new QHBoxLayout(singleInputWidget);
        singleLayout->setContentsMargins(0,0,0,0);
        SizeInput singleSize(&dlg, state.val1);
        singleLayout->addWidget(singleSize.spin);
        singleLayout->addWidget(singleSize.unitCombo);

        auto* rangeInputWidget = new QWidget();
        auto* rangeLayout = new QHBoxLayout(rangeInputWidget);
        rangeLayout->setContentsMargins(0,0,0,0);
        SizeInput rangeSize1(&dlg, state.val1);
        SizeInput rangeSize2(&dlg, state.val2);
        rangeLayout->addWidget(rangeSize1.spin);
        rangeLayout->addWidget(rangeSize1.unitCombo);
        rangeLayout->addWidget(new QLabel(" and "));
        rangeLayout->addWidget(rangeSize2.spin);
        rangeLayout->addWidget(rangeSize2.unitCombo);

        inputLayout->addWidget(opCombo);
        
        auto* stack = new QStackedWidget(&dlg);
        stack->addWidget(singleInputWidget);
        stack->addWidget(rangeInputWidget);
        inputLayout->addWidget(stack);

        layout->addLayout(inputLayout);

        auto updateStack = [stack](int idx) {
            stack->setCurrentIndex(idx == 3 ? 1 : 0);
        };
        QObject::connect(opCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), updateStack);
        updateStack(opCombo->currentIndex());

        auto updateEnabled = [=](bool active) {
            opCombo->setEnabled(active);
            stack->setEnabled(active);
        };
        QObject::connect(activeCheck, &QCheckBox::toggled, updateEnabled);
        updateEnabled(state.active);

        auto* btnLayout = new QHBoxLayout();
        auto* okBtn = new QPushButton("OK");
        auto* cancelBtn = new QPushButton("Cancel");
        btnLayout->addStretch();
        btnLayout->addWidget(okBtn);
        btnLayout->addWidget(cancelBtn);
        layout->addLayout(btnLayout);

        QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
        QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

        if (dlg.exec() == QDialog::Accepted) {
            state.active = activeCheck->isChecked();
            state.op = static_cast<FilterOperator>(opCombo->currentData().toInt());
            if (state.op == FilterOperator::Between) {
                state.val1 = rangeSize1.getBytes();
                state.val2 = rangeSize2.getBytes();
            } else {
                state.val1 = singleSize.getBytes();
            }
            return true;
        }
        return false;
    }

    bool showDateFilterDialog(QWidget* parent, DateFilterState& state, const QPoint& pos) {
        QDialog dlg(parent);
        dlg.setWindowFlags(Qt::Popup);
        dlg.setWindowTitle("Date Filter");
        dlg.move(pos);

        auto* layout = new QVBoxLayout(&dlg);
        
        auto* activeCheck = new QCheckBox("Enable Date Filter", &dlg);
        activeCheck->setChecked(state.active);
        layout->addWidget(activeCheck);

        auto* opCombo = new QComboBox(&dlg);
        opCombo->addItem("After", static_cast<int>(FilterOperator::GreaterThan));
        opCombo->addItem("Before", static_cast<int>(FilterOperator::LessThan));
        opCombo->addItem("Exactly", static_cast<int>(FilterOperator::Equals));
        opCombo->addItem("Between", static_cast<int>(FilterOperator::Between));
        
        int opIdx = 0;
        if (state.op == FilterOperator::GreaterThan) opIdx = 0;
        else if (state.op == FilterOperator::LessThan) opIdx = 1;
        else if (state.op == FilterOperator::Equals) opIdx = 2;
        else if (state.op == FilterOperator::Between) opIdx = 3;
        opCombo->setCurrentIndex(opIdx);

        auto* inputLayout = new QVBoxLayout();
        
        auto* singleInputWidget = new QWidget();
        auto* singleLayout = new QHBoxLayout(singleInputWidget);
        singleLayout->setContentsMargins(0,0,0,0);
        
        auto* singleDate = new QDateTimeEdit(&dlg);
        singleDate->setCalendarPopup(true);
        if (state.val1 > 0) singleDate->setDateTime(QDateTime::fromSecsSinceEpoch(state.val1));
        else singleDate->setDateTime(QDateTime::currentDateTime());
        
        singleLayout->addWidget(singleDate);

        auto* rangeInputWidget = new QWidget();
        auto* rangeLayout = new QHBoxLayout(rangeInputWidget);
        rangeLayout->setContentsMargins(0,0,0,0);
        
        auto* rangeDate1 = new QDateTimeEdit(&dlg);
        rangeDate1->setCalendarPopup(true);
        if (state.val1 > 0) rangeDate1->setDateTime(QDateTime::fromSecsSinceEpoch(state.val1));
        else rangeDate1->setDateTime(QDateTime::currentDateTime());

        auto* rangeDate2 = new QDateTimeEdit(&dlg);
        rangeDate2->setCalendarPopup(true);
        if (state.val2 > 0) rangeDate2->setDateTime(QDateTime::fromSecsSinceEpoch(state.val2));
        else rangeDate2->setDateTime(QDateTime::currentDateTime());

        rangeLayout->addWidget(rangeDate1);
        rangeLayout->addWidget(new QLabel(" and "));
        rangeLayout->addWidget(rangeDate2);

        inputLayout->addWidget(opCombo);
        
        auto* stack = new QStackedWidget(&dlg);
        stack->addWidget(singleInputWidget);
        stack->addWidget(rangeInputWidget);
        inputLayout->addWidget(stack);

        layout->addLayout(inputLayout);

        auto updateStack = [stack](int idx) {
            stack->setCurrentIndex(idx == 3 ? 1 : 0);
        };
        QObject::connect(opCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), updateStack);
        updateStack(opCombo->currentIndex());

        auto updateEnabled = [=](bool active) {
            opCombo->setEnabled(active);
            stack->setEnabled(active);
        };
        QObject::connect(activeCheck, &QCheckBox::toggled, updateEnabled);
        updateEnabled(state.active);

        auto* btnLayout = new QHBoxLayout();
        auto* okBtn = new QPushButton("OK");
        auto* cancelBtn = new QPushButton("Cancel");
        btnLayout->addStretch();
        btnLayout->addWidget(okBtn);
        btnLayout->addWidget(cancelBtn);
        layout->addLayout(btnLayout);

        QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
        QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

        if (dlg.exec() == QDialog::Accepted) {
            state.active = activeCheck->isChecked();
            state.op = static_cast<FilterOperator>(opCombo->currentData().toInt());
            if (state.op == FilterOperator::Between) {
                state.val1 = rangeDate1->dateTime().toSecsSinceEpoch();
                state.val2 = rangeDate2->dateTime().toSecsSinceEpoch();
            } else {
                state.val1 = singleDate->dateTime().toSecsSinceEpoch();
            }
            return true;
        }
        return false;
    }

}
