QT += core gui widgets network

CONFIG += c++14

TARGET = sudoqu
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    gameframe.cpp \
    game.cpp \
    player.cpp \
    sudoku.cpp \
    network.cpp \
    connectdialog.cpp

HEADERS  += mainwindow.h \
    gameframe.h \
    game.h \
    player.h \
    sudoku.h \
    network.h \
    connectdialog.h

FORMS    += mainwindow.ui \
    connectdialog.ui

CONFIG += link_pkgconfig
PKGCONFIG += qqwing
