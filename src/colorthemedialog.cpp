/*
 * colorthemedialog.cpp
 * Copyright (C) 2016  Jason Pleau <jason@jpleau.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "colorthemedialog.h"
#include "ui_colorthemedialog.h"

#include <QColorDialog>
#include <QLabel>
#include <QComboBox>
#include <QDebug>

namespace Sudoqu {

ColorThemeDialog::ColorThemeDialog(ColorTheme current_theme, QWidget *parent)
    : QDialog(parent), theme(current_theme), ui(new Ui::ColorThemeDialog) {
    ui->setupUi(this);

    connect(ui->select_theme, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int) {
        int selected = ui->select_theme->currentData().toInt();
        theme = ColorTheme(static_cast<ColorTheme::Theme>(selected));
        reloadColors();
    });

    ui->select_theme->addItem("Custom", ColorTheme::CUSTOM);
    ui->select_theme->addItem("Molokai", ColorTheme::MOLOKAI);
    ui->select_theme->setCurrentIndex(ui->select_theme->findData(theme.name));
}

ColorTheme ColorThemeDialog::getColorTheme() const {
    return theme;
}

ColorThemeDialog::~ColorThemeDialog() {
    delete ui;
}

void ColorThemeDialog::reloadColors() {

    widgets.clear();

    std::map<QString, QString *> widgets_names = {
        {"Background", &theme.background},
        {"Foreground", &theme.foreground},
        {"Givens background", &theme.given_background},
        {"Givens foreground", &theme.given_foreground},
        {"Focus Background", &theme.focus_background},
        {"Focus foreground", &theme.focus_foreground},
        {"Coop focus background", &theme.other_focus_background},
        {"Coop focus foreground", &theme.other_focus_foreground},
        {"Filled background", &theme.filled_background},
        {"Filled foreground", &theme.filled_foreground},
        {"Lines", &theme.lines},
    };

    for (auto &w : widgets_names) {
        QLabel *label = new QLabel(w.first);
        ColorWidget *widget = new ColorWidget();
        widget->setReadOnly(true);
        widget->setStyleSheet("background-color: " + *w.second);
        ui->form->addRow(label, widget);

        connect(widget, &ColorWidget::focused, [=]() {
            *w.second = QColorDialog::getColor(*w.second).name();
            widget->setStyleSheet("background-color: " + *w.second);
        });

        widgets[w.first] =
            std::make_tuple(w.second, std::unique_ptr<QLabel>(label), std::unique_ptr<ColorWidget>(widget));
    }
}

ColorWidget::ColorWidget(QWidget *parent) : QLineEdit(parent) {
}

ColorWidget::~ColorWidget() {
}

void ColorWidget::mouseReleaseEvent(QMouseEvent *) {
    emit focused();
}

void ColorWidget::keyPressEvent(QKeyEvent *) {
    return;
}
}
