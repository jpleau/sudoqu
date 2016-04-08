/*
 * game.cpp
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

#include "game.h"

#include "network.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTcpSocket>

#include <algorithm>

namespace Sudoqu {

Game::Game(QObject *parent) : QTcpServer(parent) {
    current_id = 0;
    mode = NOT_PLAYING;
}

void Game::start_game(SB::Difficulty difficulty, GameMode m) {
    mode = m;
    board.reset(new Sudoku);
    board->generate(difficulty);

    if (mode == COOP) {
        auto p = board->getPuzzle();
        for (auto team : teams) {
            coop_boards[team] = p;
        }
    }

    if (mode == VERSUS) {
        QJsonObject obj(sendBoard());
        sendMessageToAllPlayers(obj);
    } else {
        for (auto t : teams) {
            std::vector<Player *> list_players;
            for (auto p : players) {
                if (p.second->getTeam() == t) {
                    list_players.push_back(p.second.get());
                }
            }
            if (!list_players.empty()) {
                QJsonObject obj(sendBoard(t));
                sendMessageToPlayers(obj, list_players);
            }
        }
    }

    active = true;

    counts.clear();
    sendStatusChanges();
}

void Game::setTeamNames(QStringList t) {
    teams = t;
}

void Game::start_server(bool acceptRemote) {
    QHostAddress host = QHostAddress::AnyIPv4;

    if (!acceptRemote) {
        host = QHostAddress::LocalHost;
    }

    listen(host, 19770);
    connect(this, &QTcpServer::newConnection, this, &Game::clientConnected);
}

void Game::stop_server() {
    QJsonObject obj;
    obj["message"] = SERVER_DOWN;

    sendMessageToAllPlayers(obj);
}

void Game::clientConnected() {
    QTcpSocket *socket = nextPendingConnection();
    socket->setParent(this);

    connect(socket, &QTcpSocket::readyRead, this, &Game::dataReceived);

    ++current_id;

    players[socket] = std::make_unique<Player>(socket);

    Player *player = players[socket].get();

    player->setId(current_id);

    QJsonObject obj;
    obj["message"] = YOUR_ID;
    obj["id"] = current_id;

    QJsonArray team_array = QJsonArray::fromStringList(teams);
    obj["teams"] = team_array;

    obj["team"] = "";
    if (!teams.empty()) {
        obj["team"] = teams[0];
        assign_team(player, teams[0], false);
    }

    sendMessageToPlayer(obj, player);

    auto list = listPlayers(player);

    QJsonObject newPlayer;
    newPlayer["message"] = NEW_PLAYER;
    for (auto &p : list) {
        newPlayer["id"] = p->getId();
        newPlayer["name"] = p->getName();
        Network::sendNetworkMessage(newPlayer, *player);
    }
}

void Game::clientDisconnected(QTcpSocket *socket) {
    Player *player = players[socket].get();

    QJsonObject send;
    send["message"] = DISCONNECT_OK;
    sendMessageToPlayer(send, player);

    send = QJsonObject();
    send["message"] = DISCONNECT;
    send["name"] = player->getName();

    sendMessageToPlayersExcept(send, player);

    players.erase(socket);

    sendStatusChanges(player);
}

void Game::sendMessageToPlayer(QJsonObject &obj, Player *player) {
    Network::sendNetworkMessage(obj, *player);
}

void Game::sendMessageToAllPlayers(QJsonObject &obj) {
    auto list = listPlayers();
    sendMessageToPlayers(obj, list);
}

void Game::sendMessageToPlayers(QJsonObject &obj, std::vector<Player *> &players) {
    for (Player *p : players) {
        sendMessageToPlayer(obj, p);
    }
}

void Game::sendMessageToPlayersExcept(QJsonObject &obj, Player *except) {
    auto list = listPlayers(except);
    sendMessageToPlayers(obj, list);
}

std::vector<Player *> Game::listPlayers(Player *exceptPlayer) {
    std::vector<Player *> ret;
    for (auto &p : players) {
        if (p.second.get() != exceptPlayer) {
            ret.push_back(p.second.get());
        }
    }
    return ret;
}

bool Game::checkSolution(std::vector<int> &board_check) const {
    std::vector<int> solution = board->getSolution();
    return board_check == solution;
}

QJsonObject Game::sendBoard(QString team) {
    QJsonObject obj;
    obj["message"] = NEW_GAME;
    obj["mode"] = mode;

    std::vector<int> puzzle = board->getPuzzle();
    std::list<QVariant> list(puzzle.begin(), puzzle.end());
    QJsonArray array = QJsonArray::fromVariantList(QList<QVariant>::fromStdList(list));
    obj["given"] = array;

    if (mode == COOP) {
        std::list<QVariant> list_coop(coop_boards[team].begin(), coop_boards[team].end());
        QJsonArray coop_json = QJsonArray::fromVariantList(QList<QVariant>::fromStdList(list_coop));
        obj["board"] = coop_json;
    }
    return obj;
}

void Game::assign_team(Player *player, QString team, bool send) {
    player->setTeam(team);
    if (send) {
        QJsonObject obj;
        obj["message"] = CHANGE_TEAM;
        obj["player"] = player->getName();
        obj["team"] = team;
        sendMessageToAllPlayers(obj);
    }
}

std::vector<Player *> Game::listPlayersInTeam(QString t, Player *except) {
    std::vector<Player *> ret;
    for (auto p : players) {
        if (p.second.get() != except && p.second->getTeam() == t) {
            ret.push_back(p.second.get());
        }
    }
    return ret;
}

void Game::sendStatusChanges(Player *except) {
    QJsonArray list_teams;
    QJsonArray list_players;
    QJsonArray list_count;
    QJsonArray list_done;

    QJsonObject send;

    int count_total = 0;
    if (active) {
        auto puzzle = board->getPuzzle();
        for (int i : puzzle) {
            if (i > 0) {
                ++count_total;
            }
        }
        count_total = static_cast<int>(puzzle.size()) - count_total;
    }

    send["count_total"] = count_total;

    if (mode != COOP) {
        for (auto &p : players) {
            if (p.second.get() != except) {
                list_players.append(p.second->getName());
                if (active) {
                    list_count.append(counts[p.second->getId()]);
                    list_done.append(p.second->isDone());
                } else {
                    list_done.append(false);
                }
            }
        }
    } else {
        for (auto t : teams) {
            int count = 0;
            bool done = false;
            QStringList players_team;
            for (auto &p : players) {
                if (p.second->getTeam() == t) {
                    players_team.push_back(p.second->getName());
                }
            }

            auto given_board = board->getPuzzle();
            for (size_t i = 0; i < coop_boards[t].size(); ++i) {
                if (coop_boards[t][i] > 0 && given_board[i] == 0) {
                    ++count;
                }
            }
            done = checkSolution(coop_boards[t]);

            if (!players_team.empty()) {
                QString fullName = QString("%1: %2").arg(t).arg(players_team.join(", "));
                list_players.append(fullName);
                list_count.append(count);
                list_done.append(done);
            }
        }
    }

    send["message"] = STATUS_CHANGE;
    send["players"] = list_players;
    send["counts"] = list_count;
    send["done"] = list_done;

    sendMessageToAllPlayers(send);
}

void Game::dataReceived() {
    QTcpSocket *socket = static_cast<QTcpSocket *>(this->sender());
    Player *player = players[socket].get();
    QString data;
    while (socket && socket->canReadLine()) {
        data = socket->readLine();
        QJsonObject obj = Network::readNetworkMessage(data);
        if (obj.contains("message")) {
            int id = obj["id"].toInt();
            int message = obj["message"].toInt();

            switch (message) {
            case SEND_NAME:
                if (obj.find("version") == obj.end() || obj["version"].toInt() != SUDOQU_VERSION) {
                    QJsonObject bad;
                    bad["message"] = BAD_VERSION;
                    bad["server_version"] = SUDOQU_VERSION;
                    bad["client_version"] = obj["version"].toInt();
                    sendMessageToPlayer(bad, player);
                    return;
                }
                if (id == player->getId()) {
                    player->setName(obj["name"].toString());
                    obj["message"] = NEW_PLAYER;
                    sendMessageToAllPlayers(obj);
                }

                if (active) {
                    QJsonObject obj(sendBoard(mode == COOP ? player->getTeam() : ""));
                    sendMessageToPlayer(obj, player);
                }

                sendStatusChanges();
                break;
            case CHAT_MESSAGE:
                obj["name"] = player->getName();
                sendMessageToPlayersExcept(obj, player);
                break;

            case DISCONNECT:
                clientDisconnected(socket);
                break;

            case NEW_COUNT: {
                counts[player->getId()] = obj["count"].toInt();

                if (mode == COOP) {
                    obj["message"] = NEW_VALUE;
                    obj.remove("count");

                    QString team = player->getTeam();
                    std::vector<Player *> list_players;
                    for (auto p : players) {
                        if (p.second->getTeam() == team && p.second.get() != player) {
                            list_players.push_back(p.second.get());
                        }
                    }
                    if (!list_players.empty()) {
                        sendMessageToPlayers(obj, list_players);
                    }

                    size_t pos = static_cast<size_t>(obj["pos"].toInt());
                    coop_boards[player->getTeam()][pos] = obj["val"].toInt();
                }

                sendStatusChanges();
                break;
            }

            case TEST_SOLUTION: {
                QJsonArray array = obj["board"].toArray();
                std::vector<int> board;
                for (int i = 0; i < array.size(); ++i) {
                    board.push_back(array[i].toInt());
                }

                if (mode == VERSUS) {
                    counts[player->getId()] = obj["count"].toInt();

                    player->setDone(checkSolution(board));
                    sendStatusChanges();
                } else {
                    coop_boards[player->getTeam()] = board;
                    obj = sendBoard(player->getTeam());
                    auto send_players = listPlayersInTeam(player->getTeam(), player);
                    sendMessageToPlayers(obj, send_players);
                    sendStatusChanges();
                }
                break;
            }

            case CHANGE_NAME:
                player->setName(obj["new_name"].toString());
                obj["id"] = player->getId();
                sendMessageToAllPlayers(obj);
                sendStatusChanges();
                break;

            case SET_FOCUS: {
                QString team = player->getTeam();
                std::vector<Player *> list_players;
                for (auto p : players) {
                    if (p.second->getTeam() == team && p.second.get() != player) {
                        list_players.push_back(p.second.get());
                    }
                }
                if (!list_players.empty()) {
                    sendMessageToPlayers(obj, list_players);
                }
                break;
            }

            case CHANGE_TEAM: {
                QString team = obj["team"].toString();
                if (player->getTeam() != team) {
                    assign_team(player, team, true);
                    if (active) {
                        obj = sendBoard(mode == COOP ? team : "");
                        sendMessageToPlayer(obj, player);

                        QJsonObject unfocus;
                        unfocus["message"] = SET_FOCUS;
                        unfocus["id"] = player->getId();
                        unfocus["pos"] = -1;
                        sendMessageToPlayersExcept(unfocus, player);
                    }

                    sendStatusChanges();
                }
                break;
            }
            }
        }
    }
}
}
