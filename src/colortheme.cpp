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
    out << theme.outer_lines;
    out << theme.inner_lines;
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
    in >> theme.outer_lines;
    in >> theme.inner_lines;
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

ColorTheme::ColorTheme() : ColorTheme(DEFAULT) {
}

std::map<ColorTheme::Theme, std::map<QString, QString>> ColorTheme::themes = {
    {DEFAULT,
     {
         {"background", "#ffffff"},
         {"foreground", "#000000"},
         {"outer_lines", "#000000"},
         {"inner_lines", "#000000"},
         {"focus_background", "#ffff00"},
         {"focus_foreground", "#000000"},
         {"given_background", "#808080"},
         {"given_foreground", "#000000"},
         {"filled_background", "#ffffff"},
         {"filled_foreground", "#000000"},
         {"other_focus_background", "#adeff9"},
         {"other_focus_foreground", "#000000"},
     }

    },

    {SUDOKAI,
     {
         {"background", "#3d3d3d"},
         {"foreground", "#ffffff"},
         {"outer_lines", "#1c1f1e"},
         {"inner_lines", "#141414"},
         {"focus_background", "#008a85"},
         {"focus_foreground", "#000000"},
         {"given_background", "#807878"},
         {"given_foreground", "#000000"},
         {"filled_background", "#34944c"},
         {"filled_foreground", "#000000"},
         {"other_focus_background", "#adeff9"},
         {"other_focus_foreground", "#000000"},

     }}

};

ColorTheme::ColorTheme(Theme theme) {
    if (themes.find(theme) == themes.end()) {
        theme = DEFAULT;
    }

    background = themes[theme]["background"];
    foreground = themes[theme]["foreground"];
    outer_lines = themes[theme]["outer_lines"];
    inner_lines = themes[theme]["inner_lines"];
    focus_background = themes[theme]["focus_background"];
    focus_foreground = themes[theme]["focus_foreground"];
    given_background = themes[theme]["given_background"];
    given_foreground = themes[theme]["given_foreground"];
    filled_background = themes[theme]["filled_background"];
    filled_foreground = themes[theme]["filled_foreground"];
    other_focus_background = themes[theme]["other_focus_background"];
    other_focus_foreground = themes[theme]["other_focus_foreground"];
}
}
