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
#include "colorthemedialog.h"
#include "game.h"
#include "player.h"

#include "ui_mainwindow.h"

#include <QMessageBox>

namespace Sudoqu {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->nickname->setText(settings.getName());

    ui->difficulty->addItem("Simple", SB::SIMPLE);
    ui->difficulty->addItem("Easy", SB::EASY);
    ui->difficulty->addItem("Intermediate", SB::INTERMEDIATE);
    ui->difficulty->addItem("Expert", SB::EXPERT);

    ui->start_game->setEnabled(false);
    ui->difficulty->setEnabled(false);

    setupMenu();

    connect(ui->nickname, &QLineEdit::returnPressed, this, &MainWindow::changeName);
    connect(ui->nickname_change, &QToolButton::clicked, this, &MainWindow::changeName);
    connect(ui->clear_fields, &QPushButton::clicked, ui->frame, &GameFrame::clearBoard);

    ui->game_mode->setId(ui->puzzle_versus, VERSUS);
    ui->game_mode->setId(ui->puzzle_coop, COOP);

    konamiKeys = {
        Qt::Key_Up,   Qt::Key_Up,    Qt::Key_Down, Qt::Key_Down, Qt::Key_Left,   Qt::Key_Right,
        Qt::Key_Left, Qt::Key_Right, Qt::Key_B,    Qt::Key_A,    Qt::Key_Return,
    };

    ui->frame->setColorTheme(settings.getColorTheme());
    ui->frame->setNotesEnabled(settings.getNotesEnabled());
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
            // TODO: do something :)
            konamiCount = 0;
        }
    }
#ifdef DEBUG
    if (ui->frame->isGameActive() && event->key() == Qt::Key_L) {
        auto modifiers = event->modifiers();
        if (modifiers & Qt::AltModifier && modifiers & Qt::ControlModifier) {
            ui->frame->cheat();
        }
    }
#endif
}

void MainWindow::mousePressEvent(QMouseEvent *) {
    setFocus();
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
    ui->clear_fields->setEnabled(false);
    ui->select_team->setEnabled(false);
    connectGameAction->setEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *) {
    if (me) {
        me->disconnectFromServer();
    }
}

void MainWindow::setupMenu() {
    newGameGroup = std::make_unique<QActionGroup>(this);

    hostGameAction = std::make_unique<QAction>("Host game", newGameGroup.get());
    hostGameAction->setShortcut(Qt::CTRL | Qt::Key_N);

    connectGameAction = std::make_unique<QAction>("Connect to a game", newGameGroup.get());
    connectGameAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_C);

    ui->menuGame->addActions(newGameGroup->actions());

    ui->menuGame->addSeparator();

    stopServerAction = std::make_unique<QAction>("Stop server", this);
    disconnectAction = std::make_unique<QAction>("Disconnect", this);
    stopServerAction->setEnabled(false);
    disconnectAction->setEnabled(false);

    ui->menuGame->addAction(disconnectAction.get());
    ui->menuGame->addAction(stopServerAction.get());

    ui->menuGame->addSeparator();

    enableNotesAction = std::make_unique<QAction>("Enable notes", this);
    enableNotesAction->setCheckable(true);
    enableNotesAction->setChecked(settings.getNotesEnabled());

    ui->menuGame->addAction(enableNotesAction.get());

    ui->menuGame->addSeparator();

    colorsAction = std::make_unique<QAction>("Configure colors", this);

    ui->menuGame->addAction(colorsAction.get());

    ui->menuGame->addSeparator();

    quitAction = std::make_unique<QAction>("Quit", this);
    quitAction->setShortcut(Qt::ALT | Qt::Key_F4);
    ui->menuGame->addAction(quitAction.get());

    aboutGroup = std::make_unique<QActionGroup>(this);
    aboutAction = std::make_unique<QAction>("About", aboutGroup.get());
    aboutQtAction = std::make_unique<QAction>("About Qt", aboutGroup.get());
    ui->menuHelp->addActions(aboutGroup->actions());

    connect(quitAction.get(), &QAction::triggered, this, &MainWindow::close);

    connect(hostGameAction.get(), &QAction::triggered, [=]() { setupServer(); });

    connect(connectGameAction.get(), &QAction::triggered, [=]() { connectToServer(); });

    connect(stopServerAction.get(), &QAction::triggered, this, &MainWindow::stopServer);

    connect(aboutAction.get(), &QAction::triggered, [=]() {
        QString aboutTitle("<h3>About Sudoqu (Version %1)</h3>"
                           "<p>Single / Multiplayer Sudoku</p>");

        aboutTitle = aboutTitle.arg(QCoreApplication::applicationVersion());

        QString aboutMsg("<p>Copyright 2016 Jason Pleau <a href=\"mailto:jason@jpleau.ca\">jason@jpleau.ca</a></p>"
                         "<p>This program is free software: you can redistribute it and/or modify "
                         "it under the terms of the GNU General Public License as published by "
                         "the Free Software Foundation, either version 3 of the License, or "
                         "(at your option) any later version.</p>"

                         "<p>This program is distributed in the hope that it will be useful, "
                         "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                         "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
                         "GNU General Public License for more details.</p>"

                         "<p>You should have received a copy of the GNU General Public License "
                         "along with this program.  If not, see <a "
                         "href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.</p>");
        QMessageBox msgBox(this);
        msgBox.setText(aboutTitle);
        msgBox.setInformativeText(aboutMsg);
        msgBox.exec();
    });
    connect(aboutQtAction.get(), &QAction::triggered, QApplication::aboutQt);

    connect(colorsAction.get(), &QAction::triggered, this, &MainWindow::configureColors);

    connect(enableNotesAction.get(), &QAction::toggled, [=](bool checked) {
        settings.setNotesEnabled(checked);
        ui->frame->setNotesEnabled(checked);
    });
}

void MainWindow::startServer(bool acceptConnections) {
    game.reset(new Game(this));
    game->start_server(acceptConnections);
    connect(ui->start_game, &QPushButton::clicked, [=]() {
        int selectedDifficulty = ui->difficulty->itemData(ui->difficulty->currentIndex()).toInt();
        SB::Difficulty difficulty = static_cast<SB::Difficulty>(selectedDifficulty);
        GameMode mode = static_cast<GameMode>(ui->game_mode->checkedId());
        game->start_game(difficulty, mode);

    });
    stopServerAction->setEnabled(true);
    ui->start_game->setEnabled(true);
    ui->difficulty->setEnabled(true);
    ui->puzzle_coop->setEnabled(true);
    ui->puzzle_versus->setEnabled(true);
    game->setTeamNames(settings.getTeamNames());
}

void MainWindow::stopServer() {
    if (game) {
        game->stop_server();
        game.reset();
    }
    stopServerAction->setEnabled(false);
    hostGameAction->setEnabled(true);
    ui->start_game->setEnabled(false);
    ui->difficulty->setEnabled(false);
}

void MainWindow::connectToServer(QString host) {
    ui->select_team->clear();
    if (ui->nickname->text().trimmed().isEmpty()) {
        ui->nickname->setStyleSheet("QLineEdit { background-color: #cc0000; color: #fff; }");
        ui->nickname->setFocus();
        return;
    }

#ifdef DEBUG
    host = "localhost";
#endif

    if (host.trimmed().isEmpty()) {
        ConnectDialog dialog(settings.getHost(), this);
        if (dialog.exec() == QDialog::Accepted) {
            host = dialog.getHost();
            settings.setHost(host);
        } else {
            return;
        }
    }

    me.reset(new Player(nullptr));
    me->setName(ui->nickname->text());
    me->connectToGame(host);
    connectGameAction->setEnabled(false);
    disconnectAction->setEnabled(true);
    if (game) {
        ui->start_game->setEnabled(true);
        ui->difficulty->setEnabled(true);
    }

    ui->select_team->blockSignals(true);
    ui->select_team->setEnabled(true);
    ui->select_team->blockSignals(false);

    connect(me.get(), &Player::playerDisconnected, this, &MainWindow::disconnectPlayer);

    connect(me.get(), &Player::receivedNewPlayer, [this](int id, QString name) {
        QString message = QString("<strong>%1</strong> has connected.").arg(name);
        ui->chat_area->appendHtml(message);

        if (id == me->getId()) {
            me->setName(name);
            ui->nickname->setText(name);
        }
    });

    connect(me.get(), &Player::playerConnected, [=]() {
        ui->chat_send_button->setEnabled(true);
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

    connect(me.get(), &Player::otherPlayerChangedName, [=](QString old_name, QString new_name) {
        QString msg =
            QString(R"(<strong>%1</strong> changed name to <strong>%2</strong><br/>)").arg(old_name).arg(new_name);
        ui->chat_area->appendHtml(msg);
    });

    connect(me.get(), &Player::otherPlayerValues, ui->frame, &GameFrame::otherPlayerValues);

    connect(me.get(), &Player::receivedNewBoard, ui->frame, [=](auto &given, auto &board, GameMode mode) {
        ui->select_team->blockSignals(true);
        ui->select_team->setEnabled(mode == COOP);
        ui->select_team->blockSignals(false);
        ui->frame->newBoard(given, board, mode);
    });

    connect(me.get(), &Player::receivedTeamList, [=](QStringList &teams) {
        ui->select_team->blockSignals(true);
        for (auto &t : teams) {
            ui->select_team->addItem(t);
        }
        ui->select_team->blockSignals(false);
    });

    connect(me.get(), &Player::otherPlayerChangedTeam, [=](QString player, QString team) {
        QString msg = QString(R"(<strong>%1</strong> joined team: <strong>%2</strong><br/>)").arg(player).arg(team);
        ui->chat_area->appendHtml(msg);
    });

    connect(ui->select_team, &QComboBox::currentTextChanged, me.get(), &Player::changeTeam);

    connect(ui->frame, &GameFrame::sendValue, me.get(), &Player::sendValue);
    connect(ui->frame, &GameFrame::sendValues, me.get(), &Player::sendValues);

    connect(ui->frame, &GameFrame::sendFocusedSquare, me.get(), &Player::sendFocusedSquare);
    connect(ui->frame, &GameFrame::setGameMode, [=](GameMode mode) {
        if (mode == COOP) {
            ui->puzzle_coop->setChecked(true);
        } else {
            ui->puzzle_versus->setChecked(true);
        }
    });
    connect(disconnectAction.get(), &QAction::triggered, me.get(), &Player::disconnectFromServer);
    connect(me.get(), &Player::otherPlayerFocus, ui->frame, &GameFrame::otherPlayerFocus);
    connect(me.get(), &Player::badVersion, this, &MainWindow::badVersion);

    connect(me.get(), &Player::gameOverWinner, [=](QString winner) {
        ui->frame->gameOverWinner();
        QString msg = QString("<strong>%1</strong> has won the game !<br/>").arg(winner);
        ui->chat_area->appendHtml(msg);
    });

    connect(ui->frame, &GameFrame::sendNotes, me.get(), &Player::sendNotes);
    connect(me.get(), &Player::receivedNotes, ui->frame, &GameFrame::receivedNotes);
    connect(me.get(), &Player::clearNotes, ui->frame, &GameFrame::clearNotes);
    connect(ui->frame, &GameFrame::toggleTakingNotes, [=](QString str) { ui->status->showMessage(str); });
}

void MainWindow::changeName() {
    QString name = ui->nickname->text().trimmed();
    if (!name.isEmpty()) {
        if (me && me->getName() != name) {
            me->changeName(name);
        }
        settings.setName(name);
    }
}

void MainWindow::sendChatMessage() {
    QString send = ui->chat_text->text().trimmed().toHtmlEscaped();
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

void MainWindow::setupServer() {
    bool acceptConnections = true;
    startServer(acceptConnections);
    hostGameAction->setEnabled(false);
    connectToServer("localhost");
}

void MainWindow::badVersion(int server_version, int client_version) {
    ui->status->showMessage(QString("Bad version. Server: %1 -- Client: %2").arg(server_version).arg(client_version));
}

void MainWindow::configureColors() {
    ColorTheme theme = settings.getColorTheme();
    ColorThemeDialog dialog(theme, ui->frame);
    if (dialog.exec() == QDialog::Accepted) {
        theme = dialog.getColorTheme();
    }

    settings.setColorTheme(theme);
    ui->frame->setColorTheme(theme);
    ui->frame->repaint();
}
}
