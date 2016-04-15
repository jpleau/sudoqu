/*
 * settings.cpp
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

#include "settings.h"

namespace Sudoqu {

Sudoqu::Settings::Settings(QObject *parent) : QSettings(QSettings::IniFormat, QSettings::UserScope, "jpleau", "Sudoqu", parent) {
}

QString Settings::getHost() const {
    return this->value("lastConnectedHost", "").toString();
}

void Settings::setHost(QString h) {
    this->setValue("lastConnectedHost", h);
}

QString Settings::getName() const {
    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    } else if (name.isEmpty()) {
        name = "Player";
    }

    return this->value("playerName", name).toString();
}

void Settings::setName(QString n) {
    this->setValue("playerName", n);
}

QStringList Settings::getTeamNames() {
    QStringList teams_default = {
        "Team A", "Team B", "Team C",
    };

    QStringList teams = this->value("teamNames", QVariant(teams_default)).toStringList();
    this->setValue("teamNames", teams);

    return teams;
}

ColorTheme Settings::getColorTheme() {
    ColorTheme theme;

    beginGroup("colors");

    theme.background = QColor(value("background", "#ffffff").toString());
    setValue("background", theme.background.name());

    theme.foreground = QColor(value("foreground", "#000000").toString());
    setValue("foreground", theme.foreground.name());

    theme.lines = QColor(value("lines", "#000000").toString());
    setValue("lines", theme.lines.name());

    theme.focus_background = QColor(value("focus_background", "#ffff00").toString());
    setValue("focus_background", theme.focus_background.name());

    theme.focus_foreground = QColor(value("focus_foreground", "#000000").toString());
    setValue("focus_foreground", theme.focus_foreground.name());

    theme.given_background = QColor(value("given_background", "#808080").toString());
    setValue("given_background", theme.given_background.name());

    theme.given_foreground = QColor(value("given_foreground", "#000000").toString());
    setValue("given_foreground", theme.given_foreground.name());

    theme.filled_background = QColor(value("filled_background", "#ffffff").toString());
    setValue("filled_background", theme.filled_background.name());

    theme.filled_foreground = QColor(value("filled_foreground", "#000000").toString());
    setValue("filled_foreground", theme.filled_foreground.name());

    theme.other_focus_background = QColor(value("other_focus_background", "#adeff9").toString());
    setValue("other_focus_background", theme.other_focus_background.name());

    theme.other_focus_foreground = QColor(value("other_focus_foreground", "#000000").toString());
    setValue("other_focus_foreground", theme.other_focus_foreground.name());

    endGroup();

    return theme;
}
}
