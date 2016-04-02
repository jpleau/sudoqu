/*
 * sudoku.cpp
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

#include "sudoku.h"

#include <qqwing.hpp>

#include <QDebug>

#include <ctime>
#include <vector>

namespace Sudoqu {

Sudoku::Sudoku() {
}

void Sudoku::generate(SB::Difficulty difficulty) {
    bool boardDone = false;
    srand(unsigned(time(0)));
    board.setRecordHistory(true);

    while (!boardDone) {
        boardDone = board.generatePuzzle();
        board.solve();
        if (board.getDifficulty() != difficulty) {
            boardDone = false;
        }
    }

    const int *_puzzle = board.getPuzzle();
    const int *_solution = board.getSolution();

    puzzle.assign(_puzzle, _puzzle + qqwing::BOARD_SIZE);
    solution.assign(_solution, _solution + qqwing::BOARD_SIZE);
}

const std::vector<int> &Sudoku::get_puzzle() const {
    return puzzle;
}

const std::vector<int> &Sudoku::get_solution() const {
    return solution;
}

void Sudoku::print() {
    const int *puzzle = board.getPuzzle();
    int size = qqwing::BOARD_SIZE;
    QVector<int> list = QVector<int>::fromStdVector({puzzle, puzzle + size});
}
}