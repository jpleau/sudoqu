/*
 * colorthemedialog.h
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

#ifndef COLORTHEMEDIALOG_H
#define COLORTHEMEDIALOG_H

#include "colortheme.h"

#include <QDialog>
#include <QLineEdit>

#include <map>
#include <memory>
#include <tuple>

namespace Ui {
class ColorThemeDialog;
}

class QLabel;
namespace Sudoqu {

class ColorWidget : public QLineEdit {
    Q_OBJECT
public:
    ColorWidget(QWidget * = nullptr);
    ~ColorWidget();
signals:
    void focused();

protected:
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
};

class ColorThemeDialog : public QDialog {
    Q_OBJECT

public:
    ColorThemeDialog(ColorTheme, QWidget *parent = 0);

    ColorTheme getColorTheme() const;

    ~ColorThemeDialog();

private:
    std::map<QString, std::tuple<QString *, std::unique_ptr<QLabel>, std::unique_ptr<ColorWidget>>> widgets;

    void reloadColors();

    ColorTheme theme;
    Ui::ColorThemeDialog *ui;
};
}

#endif
