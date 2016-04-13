/*
 * constants
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

#ifndef SUDOQU_CONSTANTS_H
#define SUDOQU_CONSTANTS_H

#define SUDOQU_VERSION 6

namespace Sudoqu {

enum Messages : int {
    BAD_VERSION,
    CHAT_MESSAGE,
    CHANGE_NAME,
    CHANGE_TEAM,
    DISCONNECT,
    DISCONNECT_OK,
    GAME_OVER_WINNER,
    NEW_PLAYER,
    NEW_GAME,
    NEW_VALUE,
    STATUS_CHANGE,
    SEND_NAME,
    SERVER_DOWN,
    SET_FOCUS,
    YOUR_ID,
};

enum GameMode : int {
    NOT_PLAYING,
    VERSUS,
    COOP,
};
}

#endif
