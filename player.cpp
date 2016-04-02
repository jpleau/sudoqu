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

    connect(socket.get(), static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this,
            &Player::socketError);
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

bool Player::getReady() const {
    return ready;
}

void Player::setReady(bool r, bool send) {
    ready = r;
    if (send) {
        QJsonObject obj;
        obj["message"] = READY_CHANGE;
        obj["ready"] = ready;
        sendMessage(obj);
    }
}

void Player::sendCount(int n) {
    QJsonObject obj;
    obj["message"] = NEW_COUNT;
    obj["count"] = n;
    sendMessage(obj);
}

void Player::wait() {
    socket->waitForBytesWritten(3000);
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
                setReady(ready, true);
            } else if (message == NEW_PLAYER) {
                int id = obj["id"].toInt();
                QString name = obj["name"].toString();
                emit receivedNewPlayer(id, name);
            } else if (message == CHAT_MESSAGE) {
                QString text = obj["text"].toString();
                QString name = obj["name"].toString();
                emit receivedChatMessage(name, text);
            } else if (message == READY_CHANGE) {
                int size = obj["players"].toArray().size();
                int count_total = obj["count_total"].toInt();
                std::vector<std::tuple<QString, bool, int, bool>> list;

                auto players = obj["players"].toArray();
                auto ready = obj["ready"].toArray();
                auto counts = obj["counts"].toArray();
                auto done = obj["done"].toArray();

                for (int i = 0; i < size; ++i) {
                    list.emplace_back(players[i].toString(), ready[i].toBool(), counts[i].toInt(), done[i].toBool());
                }

                emit receivedReadyChanges(list, count_total);
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
            }
        }
    }
}

void Player::socketError(QAbstractSocket::SocketError) {
}
}
