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

    ui->select_theme->blockSignals(true);
    ui->select_theme->addItem("", ColorTheme::NONE);
    ui->select_theme->addItem("Default", ColorTheme::DEFAULT);
    ui->select_theme->addItem("Molokai", ColorTheme::MOLOKAI);
    ui->select_theme->blockSignals(false);
    reloadColors();
}

ColorTheme ColorThemeDialog::getColorTheme() const {
    return theme;
}

ColorThemeDialog::~ColorThemeDialog() {
    delete ui;
}

void ColorThemeDialog::reloadColors() {
    widgets.clear();

    std::vector<std::map<QString, QString *>> widgets_names = {
        {{"Background", &theme.background}, {"Foreground", &theme.foreground}},
        {{"Givens background", &theme.given_background}, {"Givens foreground", &theme.given_foreground}},
        {{"Focus Background", &theme.focus_background}, {"Focus foreground", &theme.focus_foreground}},
        {{"Coop focus background", &theme.other_focus_background},
         {"Coop focus foreground", &theme.other_focus_foreground}},
        {{"Filled background", &theme.filled_background}, {"Filled foreground", &theme.filled_foreground}},
        {{"Outer lines", &theme.outer_lines}, {"Inner lines", &theme.inner_lines}},
    };
    QFormLayout *form = ui->form;
    for (auto &list : widgets_names) {
        for (auto &w : list) {
            QLabel *label = new QLabel(w.first, this);
            ColorWidget *widget = new ColorWidget(this);
            widget->setReadOnly(true);
            widget->setStyleSheet("background-color: " + *w.second);
            form->addRow(label, widget);
            connect(widget, &ColorWidget::focused, [=]() {
                *w.second = QColorDialog::getColor(*w.second).name();
                widget->setStyleSheet("background-color: " + *w.second);
            });
            widgets.push_back(std::unique_ptr<QWidget>(label));
            widgets.push_back(std::unique_ptr<QWidget>(widget));
        }
        QFrame *line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        form->addRow(line);
        widgets.push_back(std::unique_ptr<QWidget>(line));
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
