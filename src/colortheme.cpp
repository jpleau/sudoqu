/*
 * colortheme.cpp
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

#include "colortheme.h"

#include <QDebug>

namespace Sudoqu {

QDataStream &operator<<(QDataStream &out, const ColorTheme &theme) {
    out << theme.background;
    out << theme.foreground;
    out << theme.lines;
    out << theme.focus_background;
    out << theme.focus_foreground;
    out << theme.other_focus_background;
    out << theme.other_focus_foreground;
    out << theme.given_background;
    out << theme.given_foreground;
    out << theme.filled_background;
    out << theme.filled_foreground;

    return out;
}

QDataStream &operator>>(QDataStream &in, ColorTheme &theme) {
    in >> theme.background;
    in >> theme.foreground;
    in >> theme.lines;
    in >> theme.focus_background;
    in >> theme.focus_foreground;
    in >> theme.other_focus_background;
    in >> theme.other_focus_foreground;
    in >> theme.given_background;
    in >> theme.given_foreground;
    in >> theme.filled_background;
    in >> theme.filled_foreground;
    return in;
}

ColorTheme::ColorTheme() : ColorTheme(CUSTOM) {
}

ColorTheme::ColorTheme(Theme theme) {

    switch (theme) {
    case CUSTOM:
        name = CUSTOM;
        background = "#ffffff";
        foreground = "#000000";
        lines = "#000000";
        focus_background = "#ffff00";
        focus_foreground = "#000000";
        given_background = "#808080";
        given_foreground = "#000000";
        filled_background = "#ffffff";
        filled_foreground = "#000000";
        other_focus_background = "#adeff9";
        other_focus_foreground = "#000000";
        break;

    case MOLOKAI:
        name = MOLOKAI;
        background = "#808080";
        foreground = "#F8F8F2";
        lines = "#455354";
        focus_background = "#ffff00";
        focus_foreground = "#000000";
        given_background = "#272822";
        given_foreground = "#000000";
        filled_background = "#ffffff";
        filled_foreground = "#000000";
        other_focus_background = "#66D9EF";
        other_focus_foreground = "#f8f8f2";
        break;
    }
}
}
