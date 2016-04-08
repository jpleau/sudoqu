/*
 * gameframe.cpp
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

#include "gameframe.h"

#include "sudoku.h"

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

namespace Sudoqu {

GameFrame::GameFrame(QWidget *parent) : QFrame(parent), active(false) {
}

void GameFrame::newBoard(std::vector<int> &g, std::vector<int> &b, GameMode m) {
    focused = -1;
    given = g;
    board = b;
    active = true;
    mode = m;
    repaint();
    emit setGameMode(mode);
}

int GameFrame::getAt(int pos) const {
    return board[static_cast<size_t>(pos)];
}

void GameFrame::setAt(int pos, int val, bool send_network) {
    board[static_cast<size_t>(pos)] = val;
    if (send_network) {
        sendData(pos, val);
    }
}

int GameFrame::getGivenAt(int pos) const {
    return given[static_cast<size_t>(pos)];
}

void GameFrame::stop() {
    board.clear();
    given.clear();
    active = false;
    repaint();
}

bool GameFrame::isGameActive() const {
    return active;
}

void GameFrame::cheat() {
    Sudoku s;
    s.setBoard(given);
    board = s.getSolution();

    int given_filled = 0;
    for (int i : given) {
        if (i > 0) {
            ++given_filled;
        }
    }

    sendData();

    repaint();
}

void GameFrame::clearBoard() {
    board = given;
    focused = -1;
    sendData();
    repaint();
}

void GameFrame::receiveData(int pos, int val) {
    setAt(pos, val, false);
    repaint();
}

void GameFrame::otherPlayerFocus(int id, int pos) {
    if (pos == -1) {
        playersFocus.erase(id);
    } else {
        playersFocus[id] = pos;
    }
    repaint();
}

void GameFrame::paintEvent(QPaintEvent *) {
    if (!active) {
        return;
    }

    int rows = 9;
    int cols = 9;

    int window_width = geometry().size().width();
    int window_height = geometry().size().height();

    int width = window_width / cols;
    int height = window_height / rows;

    QPainter painter(this);

    QBrush brush(Qt::SolidPattern);

    int fontSize = 30;
    if (window_width < 500 || window_height < 500) {
        fontSize = 12;
    }

    QFont font("Ubuntu", fontSize);
    painter.setFont(font);

    QPen pen(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(pen);

    std::map<int, bool> otherFocus;
    if (!playersFocus.empty()) {
        for (auto f : playersFocus) {
            if (f.second > -1) {
                otherFocus[f.second] = true;
            }
        }
    }

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int pos = row * rows + col;
            QRect rect(col * width, row * height, width, height);
            int valueGiven = this->getGivenAt(pos);
            int value = this->getAt(pos);

            if (valueGiven > 0) {
                painter.fillRect(rect, Qt::gray);
            }

            if (pos == focused) {
                painter.fillRect(rect, Qt::yellow);
            } else if (otherFocus[pos]) {
                painter.fillRect(rect, QColor(173, 239, 249));
            }

            if (value > 0) {
                QString text = QString::number(value);
                painter.drawText(rect, Qt::AlignVCenter | Qt::AlignCenter, text);
            }
        }
    }

    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            painter.drawLine(x * width, y * height, x * width + width, y * height);
            painter.drawLine(x * width, y * height, x * width, y * height + height);
        }
    }

    pen.setWidth(5);
    painter.setPen(pen);
    painter.drawLine(0, 0, rows * width, 0);
    painter.drawLine(0, 0, 0, cols * height);
    painter.drawLine(0, rows * height, cols * width, rows * height);
    painter.drawLine(cols * width, 0, cols * width, rows * height);
    painter.drawLine(0, height * 3, rows * width, height * 3);
    painter.drawLine(0, height * 6, rows * width, height * 6);
    painter.drawLine(width * 3, 0, width * 3, cols * height);
    painter.drawLine(width * 6, 0, width * 6, cols * height);
}

void GameFrame::mouseReleaseEvent(QMouseEvent *event) {
    if (!active) {
        return;
    }

    setFocus();

    int window_width = geometry().size().width();
    int window_height = geometry().size().height();

    int width = window_width / 9;
    int height = window_height / 9;

    const int x = event->pos().x();
    const int y = event->pos().y();

    int col = x / width;
    int row = y / height;

    int pos = row * 9 + col;

    if (getGivenAt(pos) > 0) {
        focused = -1;
    } else {
        focused = pos;
    }

    if (mode == COOP) {
        emit sendFocusedSquare(focused);
    }

    repaint();
}

void GameFrame::keyPressEvent(QKeyEvent *event) {
    if (focused == -1) {
        return;
    }
    int key = event->key();
    if (key == Qt::Key_0 || key == Qt::Key_Backspace || key == Qt::Key_Delete) {
        setAt(focused, 0, true);
    } else if (key == Qt::Key_Escape) {
        focused = -1;
    } else {
        auto check = key_map.find(key);
        if (check != key_map.end()) {
            setAt(focused, check->second, true);
        }
    }

    repaint();
}

void GameFrame::sendData(int pos, int val) {
    if (mode == VERSUS) {
        pos = val = -1;
    }

    int given_filled = 0;
    int board_filled = 0;
    int player_filled = 0;

    for (int i : board) {
        if (i > 0) {
            ++board_filled;
        }
    }

    for (int i : given) {
        if (i > 0) {
            ++given_filled;
        }
    }

    player_filled = board_filled - given_filled;

    if (board_filled == 81) {
        emit completeBoard(board, player_filled);
    } else {
        emit setCount(player_filled, pos, val);
    }
}
}