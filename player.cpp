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

Player::Player(QTcpSocket *s) {
    if (s == nullptr) {
        socket = new QTcpSocket;
    } else {
        socket = s;
    }
}

Player::Player(int i, QString n) : name(n), id(i) {
}

void Player::connectToGame(QString host) {
    socket->connectToHost(host, 19770);

    connect(socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this,
            &Player::socketError);
    connect(socket, &QTcpSocket::connected, this, &Player::clientConnected);
    connect(socket, &QTcpSocket::disconnected, this, &Player::clientDisconnected);
}

void Player::disconnectFromServer() {
	socket->disconnect();
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

void Player::sendMessage(QJsonObject &obj) {
    sendNetworkMessage(obj, socket);
}

void Player::setId(int i) {
    id = i;
}

void Player::clientConnected() {
    connect(socket, &QTcpSocket::readyRead, this, &Player::dataReceived);
    emit playerConnected();
}

void Player::clientDisconnected() {
    emit playerDisconnected();
}

void Player::dataReceived() {
    QString data;
    while (socket->canReadLine()) {
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
                std::vector<std::tuple<QString, bool>> list;
                for (int i = 0; i < size; ++i) {
                    list.emplace_back(obj["players"].toArray()[i].toString(), obj["ready"].toArray()[i].toBool());
                }
                emit receivedReadyChanges(list);
            }
        }
    }
}

void Player::socketError(QAbstractSocket::SocketError err) {
	emit playerDisconnected();
}

}