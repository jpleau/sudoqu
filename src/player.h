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
#include "network.h"

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
     * @param pos the square the player changed
     * @param value the value of the square changed
     */
    void sendValue(int, int);

    /**
 * @brief send a complete board to the server
 * @param board the values
 */
    void sendValues(std::vector<int> &);

    /**
     * @brief changeName change the player's name, and send the new name to the server
     * @param the player's new name
     */
    void changeName(QString);

    /**
     * @brief changeName change the player's team, and send the new team to the server
     * @param the player's new team
     */
    void changeTeam(QString);

    /**
     * @brief sendFocusedSquare send the currently focused square to the server
     *	useful in co-op mode so teammates can see what you are "working" on
     * @param pos the position currently focused
     */
    void sendFocusedSquare(int);

    /**
     * @brief send the player's notes to the server for a position
     * @param pos the position
     * @param notes the notes for that position
     */
    void sendNotes(int, std::vector<int> &);

signals:
    /**
     * @brief emitted after a new player has connected
     * @param id the new player's id
     * @param name the new player's name
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
     * @brief emitted after receiving a chat message from the server
     * @param name the player who sent the message
     * @param message the actual chat message
     */
    void receivedChatMessage(QString, QString);

    /**
     * @brief emitted after receiving status changes (for the game info panel)
     * @param list the list of Sudoqu::StatusChange
     * @param count_total the number of total squares
     */
    void receivedStatusChanges(std::vector<StatusChange> &, int);

    /**
     * @brief emitted after a player has disconnected
     * @param name the name of the disconnect player
     */
    void otherPlayerDisconnected(QString);
    /**
     * @brief emitted after receiving a new board
     * @param given the list of given for the current game
     * @param board the current board for the player / team
     * @param mode the game mode (versus / coop)
     */
    void receivedNewBoard(std::vector<int> &, std::vector<int> &, GameMode);

    /**
     * @brief emitted after another player changed their name on the server
     * @param id the player's id
     * @param old_name player's old name
     * @param new_name player's new name
     */
    void otherPlayerChangedName(QString, QString);

    /**
     * @brief emitted after receiving a new value from a new player (coop mode)
     * @param list of updated values
     */
    void otherPlayerValues(std::map<int, int> &);

    /**
     * @brief emitted after receiving a new focus position for a player (coop mode)
     * @param id other player's id
     * @param pos other player's new focus
     */
    void otherPlayerFocus(int, int);

    /**
     * @brief emitted when the player's game version does match the server's
     * @param server_version the version of the game the server is running
     * @param client_version the version of the game the player is running
     */
    void badVersion(int, int);

    /**
     * @brief emitted after receiving the team list from the server
     * @param teams the list of teams
     */
    void receivedTeamList(QStringList &);

    /**
     * @brief emitted after another player changed their team
     * @param player other player's name
     * @param team other player's new team
     */
    void otherPlayerChangedTeam(QString, QString);

    /**
     * @brief emitted when the server declared one player or one team as winner
     * @param winner the winner (team or player)
     */
    void gameOverWinner(QString);

    /**
     * @brief received notes for a position
     * @param pos the position
     * @param the notes for that position
     */
    void receivedNotes(int, std::vector<int> &);

    /**
     * @brief clear the player's notes
     */
    void clearNotes();

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