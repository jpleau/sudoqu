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
    network.cpp

HEADERS  += mainwindow.h \
    gameframe.h \
    game.h \
    player.h \
    sudoku.h \
    network.h

FORMS    += mainwindow.ui

#unix: CONFIG += link_pkgconfig
#unix: PKGCONFIG += qqwing
