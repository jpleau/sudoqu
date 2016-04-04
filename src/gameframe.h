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

#include <map>
#include <vector>

namespace Sudoqu {

class GameFrame : public QFrame {
    Q_OBJECT
public:
    GameFrame(QWidget * = nullptr);

    void newBoard(std::vector<int>);

    int getAt(int) const;
    void setAt(int, int);

    int getGivenAt(int) const;

    void stop();

    bool isGameActive() const;

    void cheat();

    void clearBoard();

signals:
    void setCount(int);
    void completeBoard(std::vector<int> &, int);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    bool active;
    std::vector<int> board;
    std::vector<int> given;

    std::map<int, int> key_map = {
        {Qt::Key_1, 1}, {Qt::Key_2, 2}, {Qt::Key_3, 3}, {Qt::Key_4, 4}, {Qt::Key_5, 5},
        {Qt::Key_6, 6}, {Qt::Key_7, 7}, {Qt::Key_8, 8}, {Qt::Key_9, 9},
    };

    int focused = -1;

    void sendData();
};
}

#endif // GAMEFRAME_H