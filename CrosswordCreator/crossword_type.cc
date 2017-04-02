//
//  crossword_type.cc
//  CrosswordCreator
//
//  Created by Hunter Knepshield on 4/2/17.
//  Copyright © 2017 Hunter Knepshield. All rights reserved.
//

#include "crossword_type.h"

#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

const char Crossword::WILDCARD = '?';
const char Crossword::BLACK_SQUARE = '.';
const Crossword::WordBeginning Crossword::INVALID_ACROSS = {-1, -1, ACROSS};
const Crossword::WordBeginning Crossword::INVALID_DOWN = {-1, -1, DOWN};
const Crossword::Cell Crossword::DEFAULT_CELL = std::make_tuple(
	Crossword::WILDCARD, Crossword::INVALID_ACROSS, Crossword::INVALID_DOWN);

std::unique_ptr<Crossword> Crossword::Create(int height, int width,
											 const std::vector<Word>& words) {
	std::map<WordBeginning, Word> wordMap;
	std::vector<std::vector<Cell>> grid;
	for (int i = 0; i < height; i++) {
		grid.emplace_back(width, DEFAULT_CELL);
	}

	{
		WordBeginning beginningLocation;
		std::vector<char> characters;
		int r, c;
		WordDirection direction;
		for (const auto& word : words) {
			std::tie(beginningLocation, characters) = word;
			// There's already a word that starts here.
			if (wordMap.find(beginningLocation) != wordMap.end())
				return nullptr;
			std::tie(r, c, direction) = beginningLocation;
			for (const auto& character : characters) {
				auto& cell = grid[r][c];
				auto& cellChar = std::get<0>(cell);
				auto& cellWord =
					direction == ACROSS ? std::get<1>(cell) : std::get<2>(cell);
				// Is there an unexpected character here already?
				if (cellChar != WILDCARD && cellChar != character)
					return nullptr;
				cellChar = character;
				// Is there an unexpected word here already?
				if (cellWord !=
					(direction == ACROSS ? INVALID_ACROSS : INVALID_DOWN))
					return nullptr;
				cellWord = beginningLocation;
				(direction == ACROSS ? c : r)++;
			}
		}
	}

	{
		char c;
		WordBeginning across, down;
		for (auto& row : grid) {
			for (auto& cell : row) {
				std::tie(c, across, down) = cell;
				if (across == INVALID_ACROSS && down == INVALID_DOWN) {
					if (c != WILDCARD) return nullptr;
					std::get<0>(cell) = BLACK_SQUARE;
				}
			}
		}
	}

	return std::unique_ptr<Crossword>(
		new Crossword(height, width, wordMap, grid));
}

const char Crossword::operator[](const std::pair<int, int>& loc) {
	int r, c;
	std::tie(r, c) = loc;
	const auto& cell = grid_[r][c];
	return std::get<0>(cell);
}

std::ostream& operator<<(std::ostream& os, const Crossword& cw) {
	for (const auto& row : cw.grid_) {
		for (const auto& cell : row) {
			os << std::get<0>(cell) << ' ' << std::flush;
		}
		os << std::endl;
	}
	return os;
}
