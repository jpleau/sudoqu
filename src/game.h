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
#include "sudoku.h"

#include <QTcpServer>
#include <QJsonObject>

#include <map>
#include <memory>
#include <vector>

namespace Sudoqu {

using ID = int;

class Game : public QTcpServer {
    Q_OBJECT

public:
    Game(QObject * = nullptr);
    void start_server(bool);
    void stop_server();
    void start_game(SB::Difficulty);

private:
    int current_id;
    std::map<QTcpSocket *, std::shared_ptr<Player>> players;
    std::unique_ptr<Sudoku> board;
    bool active = false;
    bool done = false;

    void sendMessageToPlayer(QJsonObject &, Player *);
    void sendMessageToAllPlayers(QJsonObject &);
    void sendMessageToPlayers(QJsonObject &, std::vector<Player *> &);
    void sendMessageToPlayersExcept(QJsonObject &, Player *);
    void sendStatusChanges(Player * = nullptr);
    std::vector<Player *> listPlayers(Player * = nullptr);
    std::map<int, int> counts;
    bool checkSolution(std::vector<int> &) const;
    QJsonObject sendBoard();

private slots:
    void clientConnected();
    void clientDisconnected(QTcpSocket *);
    void dataReceived();
};
}

#endif