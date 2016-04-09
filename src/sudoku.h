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

#ifndef SUDOQU_SUDOKU_H
#define SUDOQU_SUDOKU_H

#include <qqwing.hpp>

#include <vector>

namespace Sudoqu {

using SB = qqwing::SudokuBoard;

/**
 * @brief The Sudoku class, a wrapper around the qqwing library
 */
class Sudoku {
public:
    Sudoku();
    /**
     * @brief generate a new Sudoku board
     * @param difficulty how difficult we want the board to be
     */
    void generate(SB::Difficulty = SB::EASY);

    /**
     * @brief assign a puzzle to the board
     * @param board the puzzle we want to assign
     */
    void setBoard(std::vector<int> &);

    /**
     * @return the current puzzle
     */
    const std::vector<int> &getPuzzle() const;

    /**
     * @return the solution to the current puzzle
     */
    const std::vector<int> &getSolution() const;

    /**
     * @return the number of givens for the current board;
     */
    int getGivenCount();

private:
    /**
     * @brief a sudoku board (qqwing library)
     */
    qqwing::SudokuBoard board;

    /**
     * @brief the current puzzle
     */
    std::vector<int> puzzle;

    /**
     * @brief the solution to the current puzzle
     */
    std::vector<int> solution;
};
}

#endif