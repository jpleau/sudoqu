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

Sudoqu::Settings::Settings(QObject *parent) : QSettings("jpleau", "Sudoqu", parent) {
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
}
