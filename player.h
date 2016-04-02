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

#include <QObject>
#include <QTcpSocket>

#include <memory>
#include <tuple>
#include <vector>

namespace Sudoqu {

struct SocketDeleter {
    void operator()(QTcpSocket *);
};

struct ReadyChange {
    bool done;
    bool ready;
    int count;
    QString name;
    ReadyChange(bool, bool, int, QString);
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

    bool getReady() const;
    void setReady(bool, bool = false);

    void sendCount(int);

    void setDone(bool);
    bool isDone() const;

    void testBoard(std::vector<int> &, int);

signals:
    void receivedNewPlayer(int, QString);
    void playerConnected();
    void playerDisconnected();
    void receivedChatMessage(QString, QString);
    void receivedReadyChanges(std::vector<ReadyChange> &, int);
    void otherPlayerDisconnected(QString);
    void receivedNewBoard(std::vector<int> &);

private:
    int id;
    QString name;
    std::unique_ptr<QTcpSocket, SocketDeleter> socket;
    bool ready = false;
    bool done = false;

    void sendMessage(QJsonObject &);

private slots:
    void clientConnected();
    void clientDisconnected();
    void dataReceived();
    void socketError(QAbstractSocket::SocketError);
};
};

#endif // PLAYER_H