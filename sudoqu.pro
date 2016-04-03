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
            src/connectdialog.cpp

HEADERS  += src/mainwindow.h \
            src/gameframe.h \
            src/game.h \
            src/player.h \
            src/sudoku.h \
            src/network.h \
            src/connectdialog.h

FORMS    += mainwindow.ui \
    connectdialog.ui

CONFIG += link_pkgconfig
PKGCONFIG += qqwing
