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
#include "connectdialog.h"

#include "ui_mainwindow.h"

#include "game.h"
#include "player.h"

#include <QMessageBox>

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

    ui->start_game->setEnabled(false);
    ui->difficulty->setEnabled(false);

    setupMenu();

    connect(ui->nickname, &QLineEdit::returnPressed, this, &MainWindow::changeName);
    connect(ui->clear_fields, &QPushButton::clicked, ui->frame, &GameFrame::clearBoard);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (konamiCount < konamiKeys.size()) {
        if (konamiKeys[konamiCount] == event->key()) {
            ++konamiCount;
        } else {
            konamiCount = 0;
        }

        if (konamiCount >= konamiKeys.size()) {
            ui->frame->repaint();
        }
    }

    if (event->key() == Qt::Key_L) {
        auto modifiers = event->modifiers();
        if (modifiers & Qt::AltModifier && modifiers & Qt::ControlModifier) {
            ui->frame->cheat();
        }
    }
}

void MainWindow::disconnectPlayer() {
    me.reset();
    disconnectAction->setEnabled(false);
    ui->nickname->setEnabled(true);
    ui->start_game->setEnabled(false);
    ui->difficulty->setEnabled(false);
    ui->chat_area->clear();
    ui->player_list->clear();
    ui->frame->stop();
    ui->clear_chat->setEnabled(true);
    ui->clear_fields->setEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *) {
    if (me) {
        me->disconnectFromServer();
    }
}

void MainWindow::newBoard(std::vector<int> &board) {
    ui->frame->newBoard(board);
}

void MainWindow::setupMenu() {
    newGameGroup = std::make_unique<QActionGroup>(this);

    hostGameAction = std::make_unique<QAction>("Host a game", newGameGroup.get());
    hostGameAction->setShortcut(QKeySequence::New);
    connectGameAction = std::make_unique<QAction>("Connect to a game", newGameGroup.get());
    connectGameAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
    ui->menuGame->addActions(newGameGroup->actions());

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

    aboutGroup = std::make_unique<QActionGroup>(this);
    aboutAction = std::make_unique<QAction>("About", aboutGroup.get());
    aboutQtAction = std::make_unique<QAction>("About Qt", aboutGroup.get());
    ui->menuHelp->addActions(aboutGroup->actions());

    connect(quitAction.get(), &QAction::triggered, this, &MainWindow::close);

    connect(hostGameAction.get(), &QAction::triggered, [=]() { setupNewGame(NewGameType::HOST); });
    connect(connectGameAction.get(), &QAction::triggered, [=]() { setupNewGame(NewGameType::CONNECT); });

    connect(stopServerAction.get(), &QAction::triggered, this, &MainWindow::stopServer);

    connect(aboutAction.get(), &QAction::triggered, [=]() {
        QString aboutTitle("<h3>About Sudoqu (Version %1)</h3>"
                           "<p>Single / Multiplayer Sudoku</p>");

        aboutTitle = aboutTitle.arg(QCoreApplication::applicationVersion());

        QString aboutMsg("<p>Copyright 2016 Jason Pleau <a href=\"mailto:jason@jpleau.ca\">jason@jpleau.ca</a></p>"
                         "<p>This program is free software: you can redistribute it and/or modify"
                         "it under the terms of the GNU General Public License as published by"
                         "the Free Software Foundation, either version 3 of the License, or"
                         "(at your option) any later version.</p>"

                         "<p>This program is distributed in the hope that it will be useful,"
                         "but WITHOUT ANY WARRANTY; without even the implied warranty of"
                         "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the"
                         "GNU General Public License for more details.</p>"

                         "<p>You should have received a copy of the GNU General Public License"
                         "along with this program.  If not, see <a "
                         "href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.</p>");
        QMessageBox msgBox(this);
        msgBox.setText(aboutTitle);
        msgBox.setInformativeText(aboutMsg);
        msgBox.exec();
    });

    connect(aboutQtAction.get(), &QAction::triggered, QApplication::aboutQt);
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

    if (type == CONNECT) {
        ConnectDialog dialog;
        if (dialog.exec() == QDialog::Accepted) {
            host = dialog.getHost();
        } else {
            return;
        }
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
    ui->start_game->setEnabled(true);
    ui->difficulty->setEnabled(true);
}

void MainWindow::stopServer() {
    if (game) {
        game->stop_server();
        game.reset();
    }
    stopServerAction->setEnabled(false);
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
        ui->clear_chat->setEnabled(true);
        ui->clear_fields->setEnabled(true);
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

    connect(ui->clear_chat, &QPushButton::clicked, this, &MainWindow::clearChat);

    connect(me.get(), &Player::receivedNewBoard, this, &MainWindow::newBoard);

    connect(ui->frame, &GameFrame::setCount, me.get(), &Player::sendCount);
    connect(ui->frame, &GameFrame::completeBoard, me.get(), &Player::testBoard);

    connect(disconnectAction.get(), &QAction::triggered, me.get(), &Player::disconnectFromServer);
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
        clearChat();
        return;
    }

    if (me && !send.trimmed().isEmpty()) {
        me->sendChatMessage(send);
        QString message = QString("<strong>You: </strong> %1").arg(send);
        ui->chat_area->appendHtml(message);
    }
}

void MainWindow::clearChat() {
    ui->chat_area->clear();
}
}