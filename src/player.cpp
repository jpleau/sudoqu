/*
 * player.cpp
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

#include "player.h"

#include "constants.h"
#include "network.h"

#include <QJsonArray>
#include <QTcpSocket>

namespace Sudoqu {

void SocketDeleter::operator()(QTcpSocket *s) {
    s->deleteLater();
}

Player::Player(QTcpSocket *s) {
    if (s == nullptr) {
        s = new QTcpSocket;
    }
    socket = std::unique_ptr<QTcpSocket, SocketDeleter>(s, SocketDeleter());
}

void Player::connectToGame(QString host) {
    socket->connectToHost(host, 19770);
    connect(socket.get(), &QTcpSocket::connected, this, &Player::clientConnected);
    connect(socket.get(), &QTcpSocket::disconnected, this, &Player::clientDisconnected);
    void (QAbstractSocket::*sig)(QAbstractSocket::SocketError) = &QAbstractSocket::error;
    connect(socket.get(), sig, [=](QAbstractSocket::SocketError) { emit playerDisconnected(); });
}

void Player::disconnectFromServer() {
    QJsonObject obj;
    obj["message"] = DISCONNECT;
    sendMessage(obj);
}

void Player::setName(QString name) {
    this->name = name;
}

QString Player::getName() const {
    return name;
}

int Player::getId() const {
    return id;
}

void Player::sendChatMessage(QString message) {
    QJsonObject obj;
    obj["message"] = CHAT_MESSAGE;
    obj["text"] = message;
    sendMessage(obj);
}

Sudoqu::Player::operator QTcpSocket *() {
    return socket.get();
}

void Player::sendValue(int pos, int value) {
    QJsonObject obj;
    obj["message"] = NEW_VALUE;
    obj["pos"] = pos;
    obj["val"] = value;
    sendMessage(obj);
}

void Player::sendValues(std::vector<int> &values) {
    QJsonObject obj;
    obj["message"] = NEW_VALUE;
    std::list<QVariant> list(values.begin(), values.end());
    QJsonArray array = QJsonArray::fromVariantList(QVariantList::fromStdList(list));
    obj["values"] = array;
    sendMessage(obj);
}

void Player::sendMessage(QJsonObject &obj) {
    Network::sendNetworkMessage(obj, socket.get());
}

void Player::setId(int i) {
    id = i;
}

void Player::clientConnected() {
    connect(socket.get(), &QTcpSocket::readyRead, this, &Player::dataReceived);
    emit playerConnected();
}

void Player::clientDisconnected() {
    if (socket->state() == QAbstractSocket::UnconnectedState || socket->waitForDisconnected(1000)) {
        emit playerDisconnected();
    }
}

void Player::setDone(bool d) {
    done = d;
}

bool Player::isDone() const {
    return done;
}

void Player::changeName(QString new_name) {
    this->name = new_name;
    QJsonObject obj;
    obj["message"] = CHANGE_NAME;
    obj["new_name"] = new_name.toHtmlEscaped();
    sendMessage(obj);
}

void Player::changeTeam(QString team) {
    this->team = team;
    QJsonObject obj;
    obj["message"] = CHANGE_TEAM;
    obj["team"] = team;
    sendMessage(obj);
}

void Player::sendFocusedSquare(int pos) {
    QJsonObject obj;
    obj["message"] = SET_FOCUS;
    obj["pos"] = pos;
    obj["id"] = id;
    sendMessage(obj);
}

QString Player::getTeam() const {
    return team;
}

void Player::setTeam(const QString &t) {
    team = t;
}

void Player::dataReceived() {
    QString data;
    while (socket != nullptr && socket->canReadLine()) {
        data = socket->readLine();
        QJsonObject obj = Network::readNetworkMessage(data);
        if (obj.contains("message")) {
            int message = obj["message"].toInt();

            switch (message) {
            case YOUR_ID: {
                this->setId(obj["id"].toInt());
                QJsonObject send;
                send["message"] = SEND_NAME;
                send["id"] = id;
                send["name"] = name.toHtmlEscaped();
                send["version"] = SUDOQU_VERSION;
                sendMessage(send);

                auto arr_teams = obj["teams"].toArray();
                QStringList teams;
                for (auto t : arr_teams) {
                    teams.push_back(t.toString());
                }

                team = obj["team"].toString();

                emit receivedTeamList(teams);

                break;
            }

            case NEW_PLAYER:
                emit receivedNewPlayer(obj["id"].toInt(), obj["name"].toString());
                break;

            case CHAT_MESSAGE:
                emit receivedChatMessage(obj["name"].toString(), obj["text"].toString());
                break;

            case STATUS_CHANGE: {
                std::vector<StatusChange> list;

                for (auto change : obj["changes"].toArray()) {
                    list.emplace_back(change.toObject());
                }

                std::sort(list.begin(), list.end(), [](auto &a, auto &b) {
                    if (a.count == b.count) {
                        return a.name > b.name;
                    }
                    return a.count > b.count;
                });

                emit receivedStatusChanges(list, obj["count_total"].toInt());
                break;
            }

            case DISCONNECT:
                emit otherPlayerDisconnected(obj["name"].toString());
                break;

            case DISCONNECT_OK:
            case SERVER_DOWN:
                socket->disconnectFromHost();
                break;

            case NEW_GAME: {
                auto array = obj["given"].toArray();
                std::vector<int> given;
                for (int i = 0; i < array.size(); ++i) {
                    given.push_back(array[i].toInt());
                }

                auto board = given;
                if (obj.find("board") != obj.end()) {
                    board.clear();
                    array = obj["board"].toArray();
                    for (int i = 0; i < array.size(); ++i) {
                        board.push_back(array[i].toInt());
                    }
                }
                GameMode mode = static_cast<GameMode>(obj["mode"].toInt());

                emit receivedNewBoard(given, board, mode);

                break;
            }

            case CHANGE_NAME:
                emit otherPlayerChangedName(obj["id"].toInt(), obj["old_name"].toString(), obj["new_name"].toString());
                break;

            case NEW_VALUE: {
                std::map<int, int> values;

                if (obj.find("values") == obj.end()) {
                    values[obj["pos"].toInt()] = obj["val"].toInt();
                } else {
                    auto tmp_values = obj["values"].toArray();
                    for (int i = 0; i < tmp_values.size(); ++i) {
                        values[i] = tmp_values[i].toInt();
                    }
                }

                emit otherPlayerValues(values);

                break;
            }

            case SET_FOCUS:
                emit otherPlayerFocus(obj["id"].toInt(), obj["pos"].toInt());
                break;
            case BAD_VERSION:
                emit badVersion(obj["server_version"].toInt(), obj["client_version"].toInt());
                break;

            case CHANGE_TEAM:
                emit otherPlayerChangedTeam(obj["player"].toString(), obj["team"].toString());
                break;

            case GAME_OVER_WINNER: {
                QString winner;
                if (obj.find("team") != obj.end()) {
                    winner = "Team " + obj["team"].toString();
                } else {
                    winner = obj["player"].toString();
                }

                emit gameOverWinner(winner);

                break;
            }
            }
        }
    }
}
}
