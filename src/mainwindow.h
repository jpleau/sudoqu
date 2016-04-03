/*
 * mainwindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QActionGroup>
#include <QMainWindow>

#include <memory>

namespace Ui {
class MainWindow;
}

namespace Sudoqu {

enum NewGameType : int {
    CONNECT,
    HOST,
};

class Game;
class Player;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<Game> game;
    std::unique_ptr<Player> me;

    void sendChatMessage();
    void connectPlayer();
    void disconnectPlayer();
    void closeEvent(QCloseEvent *);
    void newBoard(std::vector<int> &);
    void setupMenu();
    void setupNewGame(NewGameType);
    void startServer(bool);
    void stopServer();
    void connectToServer(QString host);
    void changeName();

    // Menu
    std::unique_ptr<QMenu> newGameMenu;
    std::unique_ptr<QActionGroup> newGameGroup;
    std::unique_ptr<QAction> hostGameAction;
    std::unique_ptr<QAction> connectGameAction;
    std::unique_ptr<QAction> quitAction;
    std::unique_ptr<QAction> stopServerAction;
    std::unique_ptr<QAction> disconnectAction;
};
}

#endif
