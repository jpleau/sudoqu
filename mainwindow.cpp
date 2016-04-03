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

    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    } else if (name.isEmpty()) {
        name = "Player";
    }

    ui->nickname->setText(name);

    ui->difficulty->addItem("Simple", SB::SIMPLE);
    ui->difficulty->addItem("Easy", SB::EASY);
    ui->difficulty->addItem("Intermedia", SB::INTERMEDIATE);
    ui->difficulty->addItem("Expert", SB::EXPERT);

    ui->start_game->setVisible(false);
    ui->difficulty->setVisible(false);

    setupMenu();

    connect(ui->nick_change, &QPushButton::clicked, this, &MainWindow::changeName);
    connect(ui->nickname, &QLineEdit::returnPressed, this, &MainWindow::changeName);

    connect(ui->cheat, &QPushButton::clicked, ui->frame, &GameFrame::cheat);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::disconnectPlayer() {
    me.reset();
    disconnectAction->setEnabled(false);
    ui->nickname->setEnabled(true);
    ui->cheat->setEnabled(false);
    ui->start_game->setEnabled(false);
    ui->difficulty->setEnabled(false);
    ui->chat_area->clear();
    ui->player_list->clear();
    ui->frame->stop();
}

void MainWindow::closeEvent(QCloseEvent *) {
    if (me) {
        me->disconnectFromServer();
    }
}

void MainWindow::newBoard(std::vector<int> &board) {
    ui->frame->newBoard(board);
    ui->cheat->setEnabled(true);
}

void MainWindow::setupMenu() {
    newGameMenu = std::make_unique<QMenu>("New game");
    ui->menuGame->addMenu(newGameMenu.get());

    newGameGroup = std::make_unique<QActionGroup>(this);

    hostGameAction = std::make_unique<QAction>("Host a game", newGameGroup.get());
    connectGameAction = std::make_unique<QAction>("Connect to a game", newGameGroup.get());
    newGameMenu->addActions(newGameGroup->actions());

    ui->menuGame->addSeparator();

    stopServerAction = std::make_unique<QAction>("Stop server", this);
    disconnectAction = std::make_unique<QAction>("Disconnect", this);
    stopServerAction->setEnabled(false);
    disconnectAction->setEnabled(false);

    ui->menuGame->addAction(disconnectAction.get());
    ui->menuGame->addAction(stopServerAction.get());

    ui->menuGame->addSeparator();

    quitAction = std::make_unique<QAction>("Quit", this);
    quitAction->setShortcut(Qt::ALT | Qt::Key_F4);
    ui->menuGame->addAction(quitAction.get());

    connect(quitAction.get(), &QAction::triggered, this, &MainWindow::close);

    connect(hostGameAction.get(), &QAction::triggered, [=]() { setupNewGame(NewGameType::HOST); });
    connect(connectGameAction.get(), &QAction::triggered, [=]() { setupNewGame(NewGameType::CONNECT); });

    connect(disconnectAction.get(), &QAction::triggered, this, &MainWindow::disconnectPlayer);
    connect(stopServerAction.get(), &QAction::triggered, this, &MainWindow::stopServer);
}

void MainWindow::setupNewGame(NewGameType type) {
    if (ui->nickname->text().trimmed().isEmpty()) {
        ui->nickname->setStyleSheet("QLineEdit { background-color: #cc0000; color: #fff; }");
        ui->nickname->setFocus();
    }

    QString host = "localhost";

    if (type == HOST) {
        bool acceptConnections = true;
        startServer(acceptConnections);
    }

    connectToServer(host);
}

void MainWindow::startServer(bool acceptConnections) {
    game.reset(new Game(this));
    game->start_server(acceptConnections);
    connect(ui->start_game, &QPushButton::clicked, [=]() {
        int selectedDifficulty = ui->difficulty->itemData(ui->difficulty->currentIndex()).toInt();
        SB::Difficulty difficulty = static_cast<SB::Difficulty>(selectedDifficulty);
        game->start_game(difficulty);

    });
    stopServerAction->setEnabled(true);
    ui->start_game->setVisible(true);
    ui->difficulty->setVisible(true);
}

void MainWindow::stopServer() {
    if (game) {
        game->stop_server();
        game.reset();
    }
    stopServerAction->setEnabled(false);
    ui->start_game->setVisible(false);
    ui->difficulty->setVisible(false);
    ui->start_game->setEnabled(false);
    ui->difficulty->setEnabled(false);
}

void MainWindow::connectToServer(QString host) {
    me.reset(new Player(nullptr));
    me->setName(ui->nickname->text());
    me->connectToGame(host);
    disconnectAction->setEnabled(true);
    if (game) {
        ui->start_game->setEnabled(true);
        ui->difficulty->setEnabled(true);
    }

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

    connect(me.get(), &Player::receivedStatusChanges, [=](std::vector<StatusChange> &list, int count_total) {
        ui->player_list->clear();
        for (auto p : list) {
            QColor color(Qt::black);

            if (ui->frame->isGameActive()) {
                if (p.count == count_total) {
                    color = p.done ? Qt::green : Qt::red;
                }
            }

            QString msg = QString(R"(<strong style="color: %2">%1)").arg(p.name).arg(color.name());
            if (count_total > 0) {
                msg += QString(" (%1 / %2)").arg(p.count).arg(count_total);
            }
            msg += "</strong><br/>";
            ui->player_list->appendHtml(msg);
        }
    });

    connect(me.get(), &Player::otherPlayerDisconnected, [=](QString name) {
        QString message = QString("<strong>%1</strong> has disconnected.").arg(name);
        ui->chat_area->appendHtml(message);
    });

    connect(me.get(), &Player::otherPlayerChangedName, [=](int id, QString old_name, QString new_name) {
        QString msg;
        if (id == me->getId()) {
            msg = QString(R"(You have changed name to <strong>%1</strong><br/>)").arg(new_name);
        } else {
            msg =
                QString(R"(<strong>%1</strong> changed name to <strong>%2</strong><br/>)").arg(old_name).arg(new_name);
        }
        ui->chat_area->appendHtml(msg);

    });
    connect(me.get(), &Player::receivedNewBoard, this, &MainWindow::newBoard);

    connect(ui->frame, &GameFrame::setCount, me.get(), &Player::sendCount);
    connect(ui->frame, &GameFrame::completeBoard, me.get(), &Player::testBoard);
}

void MainWindow::changeName() {
    if (me) {
        if (!ui->nickname->text().trimmed().isEmpty()) {
            me->changeName(ui->nickname->text().trimmed());
        }
    }
}

void MainWindow::sendChatMessage() {
    QString send = ui->chat_text->text().trimmed();
    ui->chat_text->clear();

    if (send == "/clear") {
        ui->chat_area->clear();
        return;
    }

    if (me && !send.trimmed().isEmpty()) {
        me->sendChatMessage(send);
        QString message = QString("<strong>You: </strong> %1").arg(send);
        ui->chat_area->appendHtml(message);
    }
}
}