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
}

void Game::start() {
    listen(QHostAddress::AnyIPv4, 19770);
    connect(this, &QTcpServer::newConnection, this, &Game::clientConnected);

    qDebug() << "DEBUG: Server started";
}

void Game::clientConnected() {
    qDebug() << "DEBUG: Client connected";
    QTcpSocket *socket = nextPendingConnection();

    connect(socket, &QTcpSocket::readyRead, this, &Game::dataReceived);

    ++current_id;

    Player *player = new Player(socket);
    player->setId(current_id);

    players[socket] = player;

    QJsonObject obj;
    obj["message"] = YOUR_ID;
    obj["id"] = current_id;
    sendMessageToPlayer(obj, player);
	
	auto list = listPlayers(player);
	
	obj["message"] = NEW_PLAYER;
	for (auto &p : list) {
		obj["id"] = p->getId();
		obj["name"] = p->getName();
		sendNetworkMessage(obj, *player);
	}
}

void Game::sendMessageToPlayer(QJsonObject &obj, Player *player) {
    sendNetworkMessage(obj, *player);
}

void Game::sendMessageToAllPlayers(QJsonObject &obj) {
    auto list = listPlayers();
    sendMessageToPlayers(obj, list);
}

void Game::sendMessageToPlayers(QJsonObject &obj, std::vector<Player *> &players) {
	for (Player* p : players) {
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
		if (p.second != exceptPlayer) {
			ret.push_back(p.second);	
		}
	}
    return ret;
}

void Game::dataReceived() {
    QTcpSocket *socket = static_cast<QTcpSocket *>(this->sender());
	Player *player = players[socket];
    QString data;
    while (socket->canReadLine()) {
        data = socket->readLine();
        QJsonObject obj = readNetworkMessage(data);
        if (obj.contains("message")) {
			int id = obj["id"].toInt();
            int message = obj["message"].toInt();
            if (message == SEND_NAME) {
                QString name = obj["name"].toString();
                if (id == player->getId()) {
                    player->setName(name);
                    obj["message"] = NEW_PLAYER;
                    sendMessageToAllPlayers(obj);
                }
            } else if (message == CHAT_MESSAGE) {
				obj["name"] = player->getName();
				sendMessageToPlayersExcept(obj, player);
			} else if (message == READY_CHANGE) {
				player->setReady(obj["ready"].toBool());
				QJsonArray list_players;
				QJsonArray list_ready;
				
				for (auto &p : players) {
					list_players.append(p.second->getName());
					list_ready.append(p.second->getReady());
				}
				
				QJsonObject send;
				send["message"] = READY_CHANGE;
				send["players"] = list_players;
				send["ready"] = list_ready;
				sendMessageToAllPlayers(send);
			}
        }
    }
}
}
