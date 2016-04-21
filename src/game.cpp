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

    coop_boards.clear();
    notes.clear();
    player_boards.clear();

    board.reset(new Sudoku);
    board->generate(difficulty);

    auto puzzle = board->getPuzzle();

    if (mode == COOP) {
        for (auto team : teams) {
            coop_boards[team] = puzzle;
            auto players_in_team = listPlayersInTeam(team);
            if (!players_in_team.empty()) {
                QJsonObject obj(sendBoard(team));
                sendMessageToPlayers(obj, players_in_team);
            }
        }
    } else {
        for (auto p : players) {
            player_boards[p.second.get()] = puzzle;
            QJsonObject obj(sendBoard());
            sendMessageToAllPlayers(obj);
        }
    }

    active = true;
    sendStatusChanges();
}

void Game::setTeamNames(QStringList teams) {
    this->teams = teams;
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

    if (active) {
        player_boards[player] = board->getPuzzle();
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

    sendMessageToAllPlayers(send, player);

    players.erase(socket);

    sendStatusChanges(player);
}

void Game::sendMessageToPlayer(QJsonObject &obj, Player *player) {
    Network::sendNetworkMessage(obj, *player);
}

void Game::sendMessageToAllPlayers(QJsonObject &obj, Player *except) {
    auto list = listPlayers(except);
    sendMessageToPlayers(obj, list);
}

void Game::sendMessageToPlayers(QJsonObject &obj, std::vector<Player *> &players) {
    for (Player *p : players) {
        sendMessageToPlayer(obj, p);
    }
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

bool Game::checkSolution(std::vector<int> &board) const {
    std::vector<int> solution = this->board->getSolution();
    return board == solution;
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

        QJsonObject obj_notes;
        for (auto &note_list : notes[team]) {
            std::list<QVariant> list_notes(note_list.second.begin(), note_list.second.end());
            obj_notes[QString::number(note_list.first)] =
                QJsonArray::fromVariantList(QVariantList::fromStdList(list_notes));
        }
        obj["notes"] = obj_notes;
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

int Game::getCount(std::vector<int> &board) {
    int count = 0;
    auto puzzle = this->board->getPuzzle();
    for (size_t i = 0; i < board.size(); ++i) {
        if (board[i] > 0 && puzzle[i] == 0) {
            ++count;
        }
    }

    return count;
}

void Game::gameOverWinner(QString team) {
    QJsonObject obj;
    obj["message"] = GAME_OVER_WINNER;
    obj["team"] = team;
    sendMessageToAllPlayers(obj);
}

void Game::gameOverWinner(Player *player) {
    QJsonObject obj;
    obj["message"] = GAME_OVER_WINNER;
    obj["player"] = player->getName();
    sendMessageToAllPlayers(obj);
}

QString Game::generatePlayerName(int id, QString name) {
    int inc = 0;
    std::vector<Player *> list_players = listPlayers();
    while (true) {
        QString tmp_name = name;
        if (inc > 0) {
            tmp_name = QString("(%1) %2").arg(inc).arg(tmp_name);
        }
        bool taken = false;
        for (auto player : list_players) {
            if (player->getId() != id && player->getName() == tmp_name) {
                taken = true;
            }
        }

        if (taken) {
            ++inc;
        } else {
            name = tmp_name;
            break;
        }
    }
    return name;
}

std::vector<Player *> Game::listPlayersInTeam(QString team, Player *except) {
    std::vector<Player *> ret;
    for (auto p : players) {
        if (p.second.get() != except && p.second->getTeam() == team) {
            ret.push_back(p.second.get());
        }
    }
    return ret;
}

void Game::sendStatusChanges(Player *except) {
    QJsonArray changes;
    QJsonObject obj;

    if (mode != COOP) {
        for (auto &p : players) {
            Player *player = p.second.get();
            if (player != except) {
                bool done = active && checkSolution(player_boards[player]);
                int count = !active ? 0 : getCount(player_boards[player]);
                changes.push_back(StatusChange(done, count, player->getName()).toJson());
            }
        }
    } else {
        for (auto team : teams) {
            auto players_in_team = listPlayersInTeam(team);
            if (!players_in_team.empty()) {
                QStringList player_names;
                for (auto player : players_in_team) {
                    player_names.push_back(player->getName());
                }
                QString fullName = QString("%1: %2").arg(team).arg(player_names.join(", "));
                bool done = active && checkSolution(coop_boards[team]);
                int count = !active ? 0 : getCount(coop_boards[team]);
                changes.push_back(StatusChange(done, count, fullName).toJson());
            }
        }
    }

    obj["message"] = STATUS_CHANGE;
    obj["count_total"] = active ? 81 - board->getGivenCount() : 0;
    obj["changes"] = changes;

    sendMessageToAllPlayers(obj);
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
            case SEND_NAME: {
                if (obj.find("version") == obj.end() || obj["version"].toInt() != SUDOQU_VERSION) {
                    QJsonObject bad;
                    bad["message"] = BAD_VERSION;
                    bad["server_version"] = SUDOQU_VERSION;
                    bad["client_version"] = obj["version"].toInt();
                    sendMessageToPlayer(bad, player);
                    return;
                }
                if (id == player->getId()) {
                    QString name = generatePlayerName(id, obj["name"].toString());
                    player->setName(name);
                    obj["name"] = name;
                    obj["message"] = NEW_PLAYER;
                    sendMessageToAllPlayers(obj);
                }

                if (active) {
                    QJsonObject obj(sendBoard(mode == COOP ? player->getTeam() : ""));
                    sendMessageToPlayer(obj, player);
                }

                sendStatusChanges();
                break;
            }
            case CHAT_MESSAGE:
                obj["name"] = player->getName();
                sendMessageToAllPlayers(obj, player);
                break;

            case DISCONNECT:
                clientDisconnected(socket);
                break;

            case NEW_VALUE: {
                if (mode == COOP) {
                    QString team = player->getTeam();
                    auto list_players = listPlayersInTeam(team, player);
                    sendMessageToPlayers(obj, list_players);
                }

                std::map<size_t, int> values;

                if (obj.find("values") == obj.end()) {
                    size_t pos = static_cast<size_t>(obj["pos"].toInt());
                    int val = obj["val"].toInt();
                    values[pos] = val;
                } else {
                    auto tmp_values = obj["values"].toArray();
                    for (int i = 0; i < tmp_values.size(); ++i) {
                        values[static_cast<size_t>(i)] = tmp_values[i].toInt();
                    }
                }

                for (auto update : values) {
                    size_t pos = update.first;
                    int val = update.second;

                    if (mode == COOP) {
                        QString team = player->getTeam();
                        coop_boards[team][pos] = val;
                        if (checkSolution(coop_boards[team])) {
                            gameOverWinner(team);
                        }
                    } else {
                        player_boards[player][pos] = val;
                        if (checkSolution(player_boards[player])) {
                            gameOverWinner(player);
                        }
                    }
                }

                sendStatusChanges();
                break;
            }

            case CHANGE_NAME:
                obj["id"] = player->getId();
                obj["old_name"] = player->getName();
                obj["new_name"] = generatePlayerName(player->getId(), obj["new_name"].toString());
                player->setName(obj["new_name"].toString());
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
                        sendMessageToAllPlayers(unfocus, player);
                    }

                    sendStatusChanges();
                }
                break;
            }

            case UPDATE_NOTES: {
                int position = obj["pos"].toInt();
                std::vector<int> &team_notes = notes[player->getTeam()][position];
                team_notes.clear();
                auto list = obj["notes"].toArray();
                for (auto i : list) {
                    team_notes.push_back(i.toInt());
                }
                if (mode == COOP) {
                    auto players = listPlayersInTeam(player->getTeam(), player);
                    sendMessageToPlayers(obj, players);
                }
                break;
            }
            }
        }
    }
}
}
