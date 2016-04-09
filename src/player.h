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

#ifndef SUDOQU_PLAYER_H
#define SUDOQU_PLAYER_H

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

/**
 * @struct StatusChange
 * @brief Contains information that should be displayed in the game status pane
 */
struct StatusChange {
    /**
     * @brief done check if the player / team has finished the game
     */
    bool done;

    /**
     * @brief count number of squares that each player / team has entered
     */
    int count;

    /**
     * @brief name the name of the player / team
     */
    QString name;

    StatusChange(bool, int, QString);
};

/**
 * @class Player
 * @brief The Player / Network client
 */
class Player : public QObject {
    Q_OBJECT

public:
    Player(QTcpSocket * = nullptr);

    void setName(QString);
    QString getName() const;

    void setId(int);
    int getId() const;

    void setDone(bool);
    bool isDone() const;

    QString getTeam() const;
    void setTeam(const QString &value);

    /**
     * @brief connectToGame connect to the server
     * @param host the host to connect to
     */
    void connectToGame(QString);

    /**
     * @brief disconnectFromServer disconnect from the server
     */
    void disconnectFromServer();

    /**
     * @brief sendChatMessage send a chat message to the server
     * @param message the message to send
     */
    void sendChatMessage(QString);

    operator QTcpSocket *();

    /**
     * @brief send new value for the player's board
     * @param int pos the square the player changed
     * @param int value the value of the square changed
     */
    void sendValue(int, int);

    /**
 * @brief send a complete board to the server
 * @param board the values
 */
    void sendValues(std::vector<int> &);

    /**
     * @brief changeName change the player's name, and send the new name to the server
     * @param QString the player's new name
     */
    void changeName(QString);

    /**
     * @brief changeName change the player's team, and send the new team to the server
     * @param QString the player's new team
     */
    void changeTeam(QString);

    /**
     * @brief sendFocusedSquare send the currently focused square to the server
     *	useful in co-op mode so teammates can see what you are "working" on
     * @param int pos the position currently focused
     */
    void sendFocusedSquare(int);

signals:
    /**
     * @brief server tells the player about a new connected player
     * @param int id the new player's id
     * @param QString name the new player's name
     */
    void receivedNewPlayer(int, QString);

    /**
     * @brief emitted when the player is connected to the server
     */
    void playerConnected();

    /**
     * @brief emitted when the player is disconnected to the server
     */
    void playerDisconnected();

    /**
     * @brief received a chat message from the server
     * @param QString name the player who sent the message
     * @param QString message the actual chat message
     */
    void receivedChatMessage(QString, QString);

    /**
     * @brief received status changes (for the game info panel)
     * @param list the list of Sudoqu::StatusChange
     * @param int count_total the number of total squares
     */
    void receivedStatusChanges(std::vector<StatusChange> &, int);

    /**
     * @brief received when a player disconnected
     * @param QString name the name of the disconnect player
     */
    void otherPlayerDisconnected(QString);
    /**
     * @brief received a new board
     * @param given the list of given for the current game
     * @param board the current board for the player / team
     * @param Sudoqu::GameMode mode the game mode (versus / coop)
     */
    void receivedNewBoard(std::vector<int> &, std::vector<int> &, GameMode);

    /**
     * @brief another player changed their name on the server
     * @param int id the player's id
     * @param QString old_name player's old name
     * @param QString new_name player's new name
     */
    void otherPlayerChangedName(int, QString, QString);

    /**
     * @brief received a new value from a new player (coop mode)
     * @param int pos the position that is updated
     * @param int value the new value
     */
    void otherPlayerValue(int, int);

    /**
     * @brief received a new focus position for a player (coop mode)
     * @param int id other player's id
     * @param int pos other player's new focus
     */
    void otherPlayerFocus(int, int);

    /**
     * @brief received when the player's game version does match the server's
     * @param int server_version the version of the game the server is running
     * @param int client_version the version of the game the player is running
     */
    void badVersion(int, int);

    /**
     * @brief received the team list from the server
     * @param QStringList teams the list of teams
     */
    void receivedTeamList(QStringList &);

    /**
     * @brief received when another player changes team
     * @param QString player other player's name
     * @param QString team other player's new team
     */
    void otherPlayerChangedTeam(QString, QString);

private:
    bool done = false;

    /**
     * @brief the player's ID on the server
     */
    int id;

    /**
     * @brief the player's name
     */
    QString name;

    /**
     * @brief a pointer to the socket used to communicate with the server
     */
    std::unique_ptr<QTcpSocket, SocketDeleter> socket;

    /**
     * @brief the player's team
     */
    QString team;

    /**
     * @brief sends a JSON encoded message to the server
     * wrapper around Sudoqu::Network::sendNetworkMessage
     */
    void sendMessage(QJsonObject &);

private slots:
    /**
     * @brief called when the client connected to the server
     */
    void clientConnected();

    /**
     * @brief called when the client disconnected from the server
     */
    void clientDisconnected();

    /**
     * @brief read data from the socket, and dispatch the messages received
     */
    void dataReceived();
};
};

#endif