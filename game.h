/*
 * game.h
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

#ifndef GAME_H
#define GAME_H

#include "player.h"

#include <QTcpServer>
#include <QJsonObject>

#include <map>
#include <vector>

namespace Sudoqu {

using ID = int;

class Game : public QTcpServer {
    Q_OBJECT

public:
    Game(QObject * = nullptr);
    void start();

private:
    void clientConnected();
    void dataReceived();

    void sendMessageToPlayer(QJsonObject &, Player *);
    void sendMessageToAllPlayers(QJsonObject &);
    void sendMessageToPlayers(QJsonObject &, std::vector<Player *> &);
    void sendMessageToPlayersExcept(QJsonObject &, Player *);

    std::vector<Player*> listPlayers(Player * = nullptr);
    int current_id;
    std::map<QTcpSocket* , Player*> players;
};
}

#endif