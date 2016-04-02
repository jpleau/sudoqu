/*
 * sudoku.h
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

#ifndef SUDOKU_H
#define SUDOKU_H

#include <qqwing.hpp>

#include <vector>

namespace Sudoqu {

using SB = qqwing::SudokuBoard;

class Sudoku {
public:
    Sudoku();
    void generate(SB::Difficulty = SB::EASY);
    const std::vector<int> &getPuzzle() const;
    const std::vector<int> &getSolution() const;
    void print();

private:
    qqwing::SudokuBoard board;

    std::vector<int> puzzle;
    std::vector<int> solution;
};
}

#endif