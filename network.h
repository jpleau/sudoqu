/*
 * network.h
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

#ifndef NETWORK_H
#define NETWORK_H

#include <QJsonObject>

class QTcpSocket;

namespace Sudoqu {

enum Messages : int {
    NEW_PLAYER = 1,
    YOUR_ID,
    SEND_NAME,
    CHAT_MESSAGE,
    READY_CHANGE,
    DISCONNECT,
    DISCONNECT_OK,
    SERVER_DOWN,
    NEW_GAME,
    NEW_COUNT,
    TEST_SOLUTION,
};

void sendNetworkMessage(QJsonObject &, QTcpSocket *);
QJsonObject readNetworkMessage(QString);
}

#endif