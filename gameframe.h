/*
 * gameframe.h
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

#ifndef GAMEFRAME_H
#define GAMEFRAME_H

#include <QFrame>

#include <vector>

namespace Sudoqu {

class GameFrame : public QFrame {
public:
    GameFrame(QWidget * = nullptr);

    void newBoard(std::vector<int>);

    int getAt(int) const;
    void setAt(int, int);

    int getGivenAt(int) const;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    bool active;
    std::vector<int> board;
    std::vector<int> given;

    int focused;
};
}

#endif // GAMEFRAME_H