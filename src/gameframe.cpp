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

#include <QGuiApplication>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

#include <algorithm>

namespace Sudoqu {

GameFrame::GameFrame(QWidget *parent) : QFrame(parent), active(false) {
}

void GameFrame::newBoard(std::vector<int> &g, std::vector<int> &b, GameMode m) {
    focused = -1;
    given = g;
    board = b;
    active = true;
    gameOver = false;
    mode = m;
    notes.clear();
    playersFocus.clear();
    emit setGameMode(mode);
    repaint();
}

int GameFrame::getAt(int pos) const {
    return board[static_cast<size_t>(pos)];
}

void GameFrame::setAt(int pos, int val, bool send_network) {
    board[static_cast<size_t>(pos)] = val;
    if (send_network) {
        emit sendValue(pos, val);
    }
    notes[pos].clear();
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
    if (!active || gameOver) {
        return;
    }

    Sudoku s;
    s.setBoard(given);
    board = s.getSolution();

    int given_filled = 0;
    for (int i : given) {
        if (i > 0) {
            ++given_filled;
        }
    }

    emit sendValues(board);
    repaint();
    gameOver = true;
}

void GameFrame::clearBoard() {
    board = given;
    focused = -1;
    emit sendValues(board);
    repaint();
}

void GameFrame::otherPlayerValues(std::map<int, int> &values) {
    for (auto value : values) {
        setAt(value.first, value.second, false);
    }
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

void GameFrame::gameOverWinner() {
    gameOver = true;
    focused = -1;
    repaint();
}

void GameFrame::setColorTheme(ColorTheme theme) {
    this->colors = theme;
}

void GameFrame::receivedNotes(int pos, std::vector<int> &notes) {
    this->notes[pos] = notes;
    repaint();
}

void GameFrame::clearNotes() {
    notes.clear();
    repaint();
}

void GameFrame::setNotesEnabled(bool enabled) {
    notesEnabled = enabled;
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

    int noteFontSize = 10;

    QFont font("Ubuntu", fontSize);
    painter.setFont(font);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont fontNotes = font;
    fontNotes.setPixelSize(noteFontSize);

    std::map<int, bool> otherFocus;
    if (!playersFocus.empty()) {
        for (auto f : playersFocus) {
            if (f.second > -1) {
                otherFocus[f.second] = true;
            }
        }
    }

    QPen pen;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int pos = row * rows + col;
            QRect rect(col * width, row * height, width, height);
            int valueGiven = this->getGivenAt(pos);
            int value = this->getAt(pos);

            QColor bg;
            QColor fg;

            if (valueGiven > 0) {
                bg = colors.given_background;
                fg = colors.given_foreground;
            } else if (pos == focused) {
                bg = colors.focus_background;
                fg = colors.focus_foreground;
            } else if (value > 0) {
                bg = colors.filled_background;
                fg = colors.filled_foreground;
            } else if (otherFocus[pos]) {
                bg = colors.other_focus_background;
                fg = colors.other_focus_foreground;
            } else {
                bg = colors.background;
                fg = colors.foreground;
            }

            pen.setColor(fg);
            painter.fillRect(rect, bg);
            painter.setPen(pen);

            std::vector<int> &notes_pos = notes[pos];
            if (notesEnabled && !notes_pos.empty()) {
                painter.setFont(fontNotes);
                for (auto note : notes_pos) {
                    QString text = QString::number(note);
                    note--;
                    int note_width = width / 3;
                    int note_height = height / 3;
                    int note_pos_x = note % 3;
                    int note_pos_y = note / 3;

                    int note_x = rect.x() + (note_pos_x * note_width);
                    int note_y = rect.y() + (note_pos_y * note_height);

                    QRect rect_note(note_x, note_y, note_width, note_height);
                    painter.drawText(rect_note, Qt::AlignVCenter | Qt::AlignCenter, text);
                }

            } else if (value > 0) {
                painter.setFont(font);
                QString text = QString::number(value);
                painter.drawText(rect, Qt::AlignVCenter | Qt::AlignCenter, text);
            }
        }
    }

    QPen outer_lines;
    outer_lines.setColor(colors.outer_lines);
    outer_lines.setWidth(5);

    QPen inner_lines;
    inner_lines.setColor(colors.inner_lines);
    inner_lines.setWidth(1);

    for (int i = 0; i <= rows; ++i) {
        painter.setPen(inner_lines);
        if (i % 3 == 0) {
            painter.setPen(outer_lines);
        }
        painter.drawLine(width * i, 0, width * i, height * rows);
    }

    for (int i = 0; i <= cols; ++i) {
        painter.setPen(inner_lines);
        if (i % 3 == 0) {
            painter.setPen(outer_lines);
        }
        painter.drawLine(0, height * i, width * cols, height * i);
    }
}

void GameFrame::mouseReleaseEvent(QMouseEvent *event) {
    if (!active || gameOver) {
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
    int key = event->key();

    if (key == Qt::Key_Control && notesEnabled) {
        takingNotes = !takingNotes;
        if (takingNotes) {
            emit toggleTakingNotes("Input mode: NOTE");
        } else {
            emit toggleTakingNotes("Input mode: NORMAL");
        }
        return;
    }

    if (focused == -1 || gameOver) {
        return;
    }

    if (key == Qt::Key_Right || key == Qt::Key_Left || key == Qt::Key_Down || key == Qt::Key_Up) {
        int old_focus = focused;

        int row = focused / 9;
        int col = focused % 9;

        switch (key) {
        case Qt::Key_Right:
            col = moveFocus(1, false);
            break;
        case Qt::Key_Left:
            col = moveFocus(-1, false);
            break;
        case Qt::Key_Up:
            row = moveFocus(-1, true);
            break;
        case Qt::Key_Down:
            row = moveFocus(1, true);
            break;
        }

        focused = row * 9 + col;

        if (focused != old_focus && mode == COOP) {
            emit sendFocusedSquare(focused);
        }

    } else {
        if (key == Qt::Key_0 || key == Qt::Key_Backspace || key == Qt::Key_Delete) {
            if (!takingNotes || !notesEnabled) {
                setAt(focused, 0, true);
            } else {
                notes[focused].clear();
                if (mode == COOP) {
                    emit sendNotes(focused, notes[focused]);
                }
            }
        } else if (key == Qt::Key_Escape) {
            focused = -1;
        } else {
            auto check = key_map.find(key);
            if (check != key_map.end()) {
                if (!takingNotes || !notesEnabled) {
                    setAt(focused, check->second, true);
                } else {
                    if (getAt(focused) > 0) {
                        return;
                    }
                    std::vector<int> &notes_pos = notes[focused];

                    auto iterator = std::find(notes_pos.begin(), notes_pos.end(), check->second);
                    if (iterator == notes_pos.end()) {
                        notes_pos.push_back(check->second);
                        std::sort(notes_pos.begin(), notes_pos.end());
                    } else {
                        notes_pos.erase(iterator);
                    }
                    if (mode == COOP) {
                        emit sendNotes(focused, notes_pos);
                    }
                }
            }
        }
    }
    repaint();
}

int GameFrame::moveFocus(int delta, bool moving_row) {
    int row = focused / 9;
    int col = focused % 9;

    int ret = moving_row ? row : col;

    if (delta != 0) {
        int tmp = moving_row ? row : col;
        while (true) {
            tmp += delta;
            if (tmp < 0 || tmp > 8) {
                break;
            }
            int pos = (moving_row ? tmp : row) * 9 + (moving_row ? col : tmp);
            if (getGivenAt(pos) > 0) {
                continue;
            } else {
                ret = tmp;
                break;
            }
        }
    }
    return ret;
}
}