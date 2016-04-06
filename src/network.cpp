/*
 * network.cpp
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

#include "network.h"

#include <QJsonDocument>
#include <QTcpSocket>

namespace Sudoqu {

void Network::sendNetworkMessage(QJsonObject &obj, QTcpSocket *socket) {
    QJsonDocument doc(obj);
    QString rules(doc.toJson(QJsonDocument::Compact));
    QTextStream data(socket);
    data << rules << endl;
}

QJsonObject Network::readNetworkMessage(QString data) {
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    return doc.object();
}
}