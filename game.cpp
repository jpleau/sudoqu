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

void Game::start_game() {
	board.reset(new Sudoku);
	board->generate();
	
	QJsonObject obj;
	obj["message"] = NEW_BOARD;
	
	std::vector<int> puzzle = board->get_puzzle();
	std::list<QVariant> list(puzzle.begin(), puzzle.end());
	QList<QVariant> puzzle_json = QList<QVariant>::fromStdList(list);
	
	QJsonArray array = QJsonArray::fromVariantList(QList<QVariant>::fromStdList(list));
	obj["board"] = array;
	
	sendMessageToAllPlayers(obj);
}

void Game::start_server() {
    listen(QHostAddress::AnyIPv4, 19770);
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

	Player* player = players[socket].get();
	
	player->setId(current_id);
	
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
	
    sendReadyChange(player);
}

void Game::sendMessageToPlayer(QJsonObject &obj, Player *player) {
    sendNetworkMessage(obj, *player);
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

void Game::sendReadyChange(Player* except) {
    QJsonArray list_players;
    QJsonArray list_ready;

    for (auto &p : players) {
		if (p.second.get() != except) {
			list_players.append(p.second->getName());
	        list_ready.append(p.second->getReady());	
		}
    }

    QJsonObject send;
    send["message"] = READY_CHANGE;
    send["players"] = list_players;
    send["ready"] = list_ready;
    sendMessageToAllPlayers(send);
}

void Game::dataReceived() {
    QTcpSocket *socket = static_cast<QTcpSocket *>(this->sender());
    Player *player = players[socket].get();
    QString data;
    while (socket && socket->canReadLine()) {
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
                sendReadyChange();
            } else if (message == DISCONNECT) {
                clientDisconnected(socket);
            }
        }
    }
}
}
