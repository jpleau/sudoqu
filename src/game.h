/*
 * game.h
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

#ifndef GAME_H
#define GAME_H

#include "constants.h"
#include "player.h"
#include "sudoku.h"

#include <QTcpServer>
#include <QJsonObject>

#include <map>
#include <memory>
#include <vector>

namespace Sudoqu {

using ID = int;

class Game : public QTcpServer {
    Q_OBJECT

public:
    Game(QObject * = nullptr);

    /**
     * @brief starts the server
     * @param bool acceptRemote determines if we accept remote connections (single / multiplayer)
     */
    void start_server(bool);

    /**
     * @brief stops the server
     */
    void stop_server();

    /**
     * @brief starts a game (puzzle)
     * @param difficulty the difficulty of the puzzle
     * @param Sudoqu::GameMode mode single player or coop
     */
    void start_game(SB::Difficulty, GameMode);

    /**
     * @brief sets the team list available to players
     * @param QStringList teams the list of team names
     */
    void setTeamNames(QStringList);

private:
    /**
     * @brief incremental ID to give to new players who connect
     */
    int current_id;

    /**
     * @brief active puzzle / game or not
     */
    bool active = false;

    /**
     * @brief the current Sudoqu::GameMode (Versus or co-op)
     */
    GameMode mode;

    /**
     * @brief the list of teams available to players
     */
    QStringList teams;

    /**
     * @brief players connected to the server, holding their socket for easy access
     */
    std::map<QTcpSocket *, std::shared_ptr<Player>> players;

    /**
     * @brief the current Sudoqu::Sudoku
     */
    std::unique_ptr<Sudoku> board;

    /**
     * @brief boards for the currently playing teams, used in coop
     */
    std::map<QString, std::vector<int>> coop_boards;

    /**
 * @brief boards for the currently playing teams, used in versus
 */
    std::map<Player *, std::vector<int>> player_boards;

    /**
     * @brief Sends a JSON encoded message to a player
     */
    void sendMessageToPlayer(QJsonObject &, Player *);

    /**
     * @brief Sends a JSON encoded message to all players
     * @param obj the object we want to send
     * @param except the player we don't want to send the message to (usually
         * the one responsible for sending the message)
     */
    void sendMessageToAllPlayers(QJsonObject &, Player * = nullptr);

    /**
     * @brief Sends a JSON encoded message to a list of players
     * @param obj the object we want to send
     * @param players the list of players the message will be sent to
     */
    void sendMessageToPlayers(QJsonObject &, std::vector<Player *> &);

    /**
     * @brief sends the current game status, to be displayed in the game panel info
     */
    void sendStatusChanges(Player * = nullptr);

    /**
     * @brief lists the connected players
     * @param exceptPlayer the player we don't want to list (usually
     * the one responsible for sending the message)
     * @return the list of connected players, except the one in paramater if passed
     */
    std::vector<Player *> listPlayers(Player * = nullptr);

    /**
     * @brief lists the connected players in a specific team
     * @param team the team we want players in
     * @param exceptPlayer the player we don't want to list (usually
     * the one responsible for sending the message)
     * @return the list of connected players in the team, except the one in paramater if passed
     */
    std::vector<Player *> listPlayersInTeam(QString, Player * = nullptr);

    /**
     * @param board the board we want to test
     * @returns true if the board matches the board's solution, false if not
     */
    bool checkSolution(std::vector<int> &) const;

    /**
     * @brief used when we want to send the player an updated board
     * @return a QJsonObject ready to be sent over the network containing the board that the player should see
     */
    QJsonObject sendBoard(QString = "");

    /**
     * @brief assign a player to a team
     * @param player the player we want to assign
     * @param team the team we want the player to join
     * @param send if this action will be sent over network to other players
     */
    void assign_team(Player *, QString, bool);

    /**
     * @return the number of values entered in a board that were not givens
     */
    int getCount(std::vector<int> &);

    /**
     * @brief send the game over message to every one that the team currently being processed has won
     */
    void gameOverWinner(QString);

    /**
     * @brief send the game over message to every one that the player currently being processed has won
     */
    void gameOverWinner(Player *);

private slots:
    /**
     * @brief called when a new client (socket) connected to the server
     */
    void clientConnected();

    /**
     * @brief called when a client (socket) disconnected from the server
     */
    void clientDisconnected(QTcpSocket *);

    /**
     * @brief read data from the socket, and dispatch the messages received
     */
    void dataReceived();
};
}

#endif