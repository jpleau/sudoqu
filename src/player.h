/*
 * player.h
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

#ifndef PLAYER_H
#define PLAYER_H

#include "constants.h"

#include <QObject>
#include <QString>

#include <memory>
#include <vector>

class QObject;
class QTcpSocket;

namespace Sudoqu {

struct SocketDeleter {
    void operator()(QTcpSocket *);
};

struct StatusChange {
    bool done;
    int count;
    QString name;
    StatusChange(bool, int, QString);
};

class Player : public QObject {
    Q_OBJECT

public:
    Player(QTcpSocket * = nullptr);

    void connectToGame(QString);
    void disconnectFromServer();

    void setName(QString);
    QString getName() const;

    void setId(int);
    int getId() const;

    void sendChatMessage(QString);

    operator QTcpSocket *() {
        return socket.get();
    }

    void sendCount(int, int, int);

    void setDone(bool);
    bool isDone() const;

    void testBoard(std::vector<int> &, int);

    void changeName(QString);

    void sendFocusedSquare(int);

signals:
    void receivedNewPlayer(int, QString);
    void playerConnected();
    void playerDisconnected();
    void receivedChatMessage(QString, QString);
    void receivedStatusChanges(std::vector<StatusChange> &, int);
    void otherPlayerDisconnected(QString);
    void receivedNewBoard(std::vector<int> &, std::vector<int> &, GameMode);
    void otherPlayerChangedName(int, QString, QString);
    void otherPlayerValue(int, int);
    void otherPlayerFocus(int, int);
    void badVersion(int, int);

private:
    int id;
    QString name;
    std::unique_ptr<QTcpSocket, SocketDeleter> socket;
    bool done = false;

    void sendMessage(QJsonObject &);

private slots:
    void clientConnected();
    void clientDisconnected();
    void dataReceived();
};
};

#endif // PLAYER_H