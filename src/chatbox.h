/*
 * chatbox.h
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

#ifndef CHATBOX_H
#define CHATBOX_H

#include <QLineEdit>

class QKeyEvent;

namespace Sudoqu {

class ChatBox : public QLineEdit {
    Q_OBJECT
public:
    enum : int {
        CHAT_UP,
        CHAT_DOWN,
    };

    ChatBox(QWidget * = nullptr);

signals:
    void chatMessageScroll(int);

protected:
    void keyPressEvent(QKeyEvent *) override;
};
}

#endif