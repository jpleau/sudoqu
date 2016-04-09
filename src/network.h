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

#ifndef SUDOQU_NETWORK_H
#define SUDOQU_NETWORK_H

#include "constants.h"

#include <QJsonObject>

class QTcpSocket;

namespace Sudoqu {

/**
 * @class Network
 * @brief Static methods for sending and reading JSON over network
 */
class Network {
public:
    /**
     * @fn static void sendNetworkMessage(QJsonObject &, QTcpSocket *)
     * @brief Sends a network message encoded in JSON
     *
     * @param QJsonObject obj The QJsonObject that will be encoded in JSON
     * @param QTcpSocket* socket The client which will receive this message
     */
    static void sendNetworkMessage(QJsonObject &, QTcpSocket *);

    /**
     * @fn static QJsonObject readNetworkMessage(QString)
     * @brief Reads a JSON encoded message
     *
     * @param QString data The JSON string received over network
     * @return a Qt JSON object
     */
    static QJsonObject readNetworkMessage(QString);
};

/**
 * @struct StatusChange
 * @brief Contains information that should be displayed in the game status pane
 */
struct StatusChange {
    /**
     * @brief done check if the player / team has finished the game
     */
    bool done;

    /**
     * @brief count number of squares that each player / team has entered
     */
    int count;

    /**
     * @brief name the name of the player / team
     */
    QString name;

    /**
     * @return a json object of the status change
     */
    QJsonObject toJson() const;

    /**
     * @brief construct StatusChange from a QJSonObject
     */
    StatusChange(const QJsonObject &);

    StatusChange(bool, int, QString);
};
}

#endif