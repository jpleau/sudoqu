/*
 * mainwindow.cpp
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "game.h"
#include "player.h"

namespace Sudoqu {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    connect(ui->start_server, &QPushButton::clicked, [=]() {
        game.reset(new Game(this));
        game->start_server();
        ui->start_server->setEnabled(false);
        ui->stop_server->setEnabled(true);
        connect(ui->stop_server, &QPushButton::clicked, [=]() {
            if (game) {
                game->stop_server();
                game.reset();
            }
            ui->start_server->setEnabled(true);
            ui->stop_server->setEnabled(false);
        });
    });

    connect(ui->connect_server, &QPushButton::clicked, this, &MainWindow::connectPlayer);
    connect(ui->nickname, &QLineEdit::returnPressed, this, &MainWindow::connectPlayer);
    connect(ui->host, &QLineEdit::returnPressed, this, &MainWindow::connectPlayer);
    connect(ui->disconnect_server, &QPushButton::clicked, [=]() {
        if (me) {
            me->disconnectFromServer();
        }
    });
    connect(ui->start_game, &QPushButton::clicked, [=]() { game->start_game(); });

    connect(ui->frame, &GameFrame::setCount, [=](int n) { me->sendCount(n); });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::connectPlayer() {
    me.reset(new Player(nullptr));
    QString name = ui->nickname->text();
    QString host = ui->host->text();
    ui->nickname->setStyleSheet("QLineEdit { background-color: auto; color: auto; }");
    ui->host->setStyleSheet("QLineEdit { background-color: auto; color: auto; }");

    if (name.trimmed().isEmpty() || host.trimmed().isEmpty()) {
        if (name.trimmed().isEmpty()) {
            ui->nickname->setStyleSheet("QLineEdit { background-color: #cc0000; color: #fff; }");
            ui->nickname->setFocus();
        } else if (host.trimmed().isEmpty()) {
            ui->host->setStyleSheet("QLineEdit { background-color: #cc0000; color: #fff; }");
            ui->host->setFocus();
        }
    } else {
        me->setName(name);
        me->connectToGame(host);
        ui->connect_server->setEnabled(false);
        ui->disconnect_server->setEnabled(true);
        ui->nickname->setEnabled(false);
        ui->host->setEnabled(false);
        ui->ready->setEnabled(true);

        connect(me.get(), &Player::playerDisconnected, this, &MainWindow::disconnectPlayer);

        connect(me.get(), &Player::receivedNewPlayer, [this](int, QString name) {
            QString message = QString("<strong>%1</strong> has connected.").arg(name);
            ui->chat_area->appendHtml(message);
        });

        connect(me.get(), &Player::playerConnected, [=]() {
            ui->chat_send_button->setEnabled(true);
            connect(ui->chat_send_button, &QPushButton::clicked, this, &MainWindow::sendChatMessage);
            connect(ui->chat_text, &QLineEdit::returnPressed, this, &MainWindow::sendChatMessage);
        });

        connect(me.get(), &Player::receivedChatMessage, [=](QString name, QString text) {
            QString msg = QString("<strong>%1</strong>: %2").arg(name).arg(text);
            ui->chat_area->appendHtml(msg);
        });

        connect(me.get(), &Player::receivedReadyChanges,
                [=](std::vector<std::tuple<QString, bool, int, bool>> &list, int count_total) {
                    ui->player_list->clear();
                    for (auto t : list) {
                        QString name = std::get<0>(t);
                        bool ready = std::get<1>(t);
                        int count = std::get<2>(t);
                        bool done = std::get<3>(t);
                        QColor color;

                        if (ui->frame->isGameActive()) {
                            if (count == count_total) {
                                color = done ? Qt::green : Qt::red;
                            } else {
                                color = Qt::black;
                            }
                        } else {
                            color = ready ? Qt::green : Qt::red;
                        }
                        QString msg = QString(R"(<strong style="color: %2">%1)").arg(name).arg(color.name());
                        if (count_total > 0) {
                            msg += QString(" (%1 / %2)").arg(count).arg(count_total);
                        }
                        msg += "</strong><br/>";
                        ui->player_list->appendHtml(msg);
                    }
                });

        connect(me.get(), &Player::otherPlayerDisconnected, [=](QString name) {
            QString message = QString("<strong>%1</strong> has disconnected.").arg(name);
            ui->chat_area->appendHtml(message);
        });

        connect(me.get(), &Player::receivedNewBoard, this, &MainWindow::newBoard);

        connect(ui->ready, &QCheckBox::clicked, [=](bool checked) { me->setReady(checked, true); });
    }
}

void MainWindow::disconnectPlayer() {
    me.reset();
    ui->connect_server->setEnabled(true);
    ui->disconnect_server->setEnabled(false);
    ui->nickname->setEnabled(true);
    ui->host->setEnabled(true);
    ui->ready->setEnabled(false);
    ui->chat_area->clear();
    ui->player_list->clear();
    ui->frame->stop();
}

void MainWindow::closeEvent(QCloseEvent *) {
    // TODO: send disconnect
}

void MainWindow::newBoard(std::vector<int> &board) {
    ui->frame->newBoard(board);
}

void MainWindow::sendChatMessage() {
    QString send = ui->chat_text->text().trimmed();
    ui->chat_text->clear();

    if (send == "/clear") {
        ui->chat_area->clear();
        return;
    }

    if (!send.trimmed().isEmpty()) {
        me->sendChatMessage(send);
        QString message = QString("<strong>You: </strong> %1").arg(send);
        ui->chat_area->appendHtml(message);
    }
}
}