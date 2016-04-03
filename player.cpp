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
}

void Player::disconnectFromServer() {
    QJsonObject obj;
    obj["message"] = DISCONNECT;
    sendMessage(obj);
}

void Player::setName(QString n) {
    name = n;
}

QString Player::getName() const {
    return name;
}

int Player::getId() const {
    return id;
}

void Player::sendChatMessage(QString msg) {
    QJsonObject obj;
    obj["message"] = CHAT_MESSAGE;
    obj["text"] = msg;
    sendMessage(obj);
}

void Player::sendCount(int n) {
    QJsonObject obj;
    obj["message"] = NEW_COUNT;
    obj["count"] = n;
    sendMessage(obj);
}

void Player::sendMessage(QJsonObject &obj) {
    sendNetworkMessage(obj, socket.get());
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

void Player::testBoard(std::vector<int> &board, int count) {
    QJsonObject obj;
    std::list<QVariant> list(board.begin(), board.end());
    QList<QVariant> puzzle_json = QList<QVariant>::fromStdList(list);
    QJsonArray array = QJsonArray::fromVariantList(QList<QVariant>::fromStdList(list));

    obj["message"] = TEST_SOLUTION;
    obj["board"] = array;
    obj["count"] = count;

    sendMessage(obj);
}

void Player::changeName(QString new_name) {
    QJsonObject obj;
    obj["message"] = CHANGE_NAME;
    obj["old_name"] = this->name;
    obj["new_name"] = new_name;
    sendMessage(obj);
}

void Player::dataReceived() {
    QString data;
    while (socket != nullptr && socket->canReadLine()) {
        data = socket->readLine();
        QJsonObject obj = readNetworkMessage(data);
        if (obj.contains("message")) {
            int message = obj["message"].toInt();
            if (message == YOUR_ID) {
                this->setId(obj["id"].toInt());
                QJsonObject send;
                send["message"] = SEND_NAME;
                send["id"] = id;
                send["name"] = name;
                sendMessage(send);
            } else if (message == NEW_PLAYER) {
                int id = obj["id"].toInt();
                QString name = obj["name"].toString();
                emit receivedNewPlayer(id, name);
            } else if (message == CHAT_MESSAGE) {
                QString text = obj["text"].toString();
                QString name = obj["name"].toString();
                emit receivedChatMessage(name, text);
            } else if (message == STATUS_CHANGE) {
                int size = obj["players"].toArray().size();
                int count_total = obj["count_total"].toInt();
                std::vector<StatusChange> list;

                auto players = obj["players"].toArray();
                auto counts = obj["counts"].toArray();
                auto done = obj["done"].toArray();

                for (int i = 0; i < size; ++i) {
                    list.emplace_back(done[i].toBool(), counts[i].toInt(), players[i].toString());
                }

                emit receivedStatusChanges(list, count_total);
            } else if (message == DISCONNECT) {
                QString name = obj["name"].toString();
                emit otherPlayerDisconnected(name);
            } else if (message == DISCONNECT_OK || message == SERVER_DOWN) {
                socket->disconnectFromHost();
            } else if (message == NEW_GAME) {
                auto array = obj["board"].toArray();
                std::vector<int> board;
                for (int i = 0; i < array.size(); ++i) {
                    board.push_back(array[i].toInt());
                }
                emit receivedNewBoard(board);
            } else if (message == CHANGE_NAME) {
                emit otherPlayerChangedName(obj["id"].toInt(), obj["old_name"].toString(), obj["new_name"].toString());
            }
        }
    }
}

StatusChange::StatusChange(bool d, int c, QString n) : done(d), count(c), name(n) {
}
}
