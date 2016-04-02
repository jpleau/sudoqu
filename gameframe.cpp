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

#include <QDebug>
#include <QPainter>
#include <iostream>

namespace Sudoqu {

GameFrame::GameFrame(QWidget *parent) : QFrame(parent), active(false) {
}

void GameFrame::newBoard(std::vector<int> b) {
    for (int i : b) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;
    board = b;
    given = b;
    active = true;
    repaint();
}

int GameFrame::getAt(int pos) const {
    return board[static_cast<size_t>(pos)];
}

void GameFrame::setAt(int pos, int val) {
    board[static_cast<size_t>(pos)] = val;
}

int GameFrame::getGivenAt(int pos) const {
    return given[static_cast<size_t>(pos)];
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

    QFont font("Ubuntu", 30);
    painter.setFont(font);

    QPen pen(Qt::black);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(pen);

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int pos = row * rows + col;
            QRect rect(col * width, row * height, width, height);
            int valueGiven = this->getGivenAt(pos);
            int value = this->getAt(pos);

            if (valueGiven > 0) {
                painter.fillRect(rect, Qt::gray);
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
    painter.drawLine(0, rows * height, cols * width, rows * height);
    painter.drawLine(cols * width, 0, cols * width, rows * height);
}
}