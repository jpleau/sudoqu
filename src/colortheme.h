/*
 * colortheme.h
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

#ifndef COLORTHEME_H
#define COLORTHEME_H

#include <QMetaType>
#include <QString>

#include <map>
#include <vector>

namespace Sudoqu {

struct ColorTheme {

    enum Theme : int {
        NONE = 1,
        DEFAULT,
        MOLOKAI,
    };

    static std::map<Theme, std::map<QString, QString>> themes;

    Theme name;

    QString focus_background;
    QString focus_foreground;

    QString other_focus_background;
    QString other_focus_foreground;

    QString given_background;
    QString given_foreground;

    QString filled_background;
    QString filled_foreground;

    QString background;
    QString foreground;

    QString outer_lines;
    QString inner_lines;

    ColorTheme();
    ColorTheme(Theme);
};

QDataStream &operator<<(QDataStream &, const ColorTheme &);

QDataStream &operator>>(QDataStream &, ColorTheme &);
}

Q_DECLARE_METATYPE(Sudoqu::ColorTheme);

#endif
