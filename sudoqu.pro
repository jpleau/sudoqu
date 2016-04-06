QT += core gui widgets network

CONFIG += c++14

TARGET = sudoqu
TEMPLATE = app


SOURCES +=  src/main.cpp\
            src/mainwindow.cpp \
            src/gameframe.cpp \
            src/game.cpp \
            src/player.cpp \
            src/sudoku.cpp \
            src/network.cpp \
            src/connectdialog.cpp \
            src/chatbox.cpp \
            src/settings.cpp

HEADERS  += src/mainwindow.h \
            src/gameframe.h \
            src/game.h \
            src/player.h \
            src/sudoku.h \
            src/network.h \
            src/connectdialog.h \
            src/chatbox.h \
            src/settings.h \
            src/constants.h

FORMS    += ui/mainwindow.ui \
            ui/connectdialog.ui


VERSION = "0.0.1"

DEFINES += VERSION=\\\"$$VERSION\\\"

CONFIG(debug, debug|release){
    DEFINES += DEBUG
}

CONFIG += link_pkgconfig
PKGCONFIG += qqwing
