//
//  crossword_type.cc
//  CrosswordCreator
//
//  Created by Hunter Knepshield on 4/2/17.
//  Copyright © 2017 Hunter Knepshield. All rights reserved.
//

// This file contains implementations and definitions for all other Crossword
// methods and data.

#include "crossword_type.h"

#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <tuple>
#include <vector>

const char Crossword::WILDCARD = '.';
const char Crossword::BLACK_SQUARE = '_';
const Crossword::WordBeginning Crossword::INVALID_ACROSS = {-1, -1, ACROSS};
const Crossword::WordBeginning Crossword::INVALID_DOWN = {-1, -1, DOWN};
const Crossword::Cell Crossword::DEFAULT_CELL = std::make_tuple(
	Crossword::WILDCARD, Crossword::INVALID_ACROSS, Crossword::INVALID_DOWN);

bool Crossword::mostConstrained(Word* out) {
	int minimumUnknowns = std::numeric_limits<int>::max();
	for (const auto& beginningAndWord : words_) {
		const auto& word = beginningAndWord.second;
		const auto& characters = std::get<1>(word);
		// Count all wildcards currently in the word.
		int unknownCount = std::accumulate(characters.begin(), characters.end(),
										   0, [&](int soFar, const char& c) {
											   if (c == WILDCARD)
												   return soFar + 1;
											   return soFar;
										   });
		if (unknownCount == 0) continue;  // This word is already filled in.
		if (unknownCount < minimumUnknowns) {
			minimumUnknowns = unknownCount;
			*out = word;
		}
	}
	return minimumUnknowns != std::numeric_limits<int>::max();
}

bool Crossword::setCharacter(char value, int row, int column) {
	// Normalize to uppercase
	if (value >= 'a' && value <= 'z') value = (value - 'a') + 'A';
	char existingChar;
	WordBeginning across, down;
	std::tie(existingChar, across, down) = grid_[row][column];
	if (existingChar == value) return true;
	if (existingChar != WILDCARD && value != WILDCARD) {
		std::cerr << "Attempting to overwrite existing " << existingChar
				  << " at (" << row + 1 << ", " << column + 1 << ") with "
				  << value << "." << std::endl;
		return false;
	}
	std::get<0>(grid_[row][column]) = value;
	if (across != INVALID_ACROSS) {
		auto& existing = words_[across];
		std::get<1>(existing)[column - std::get<1>(across)] = value;
	}
	if (down != INVALID_DOWN) {
		auto& existing = words_[down];
		std::get<1>(existing)[row - std::get<0>(down)] = value;
	}
	return true;
}

std::ostream& operator<<(std::ostream& os, const Crossword& cw) {
	for (const auto& row : cw.grid_) {
		for (const auto& cell : row) {
			os << std::get<0>(cell) << ' ';
		}
		os << std::endl;
	}
	return os;
}

std::ostream& operator<<(std::ostream& os,
						 const Crossword::WordBeginning& wordBeginning) {
	os << "(" << std::get<0>(wordBeginning) + 1 << ", "
	   << std::get<1>(wordBeginning) + 1 << ") ";
	switch (std::get<2>(wordBeginning)) {
		case Crossword::ACROSS:
			os << "across";
			break;
		case Crossword::DOWN:
			os << "down";
			break;
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const Crossword::Word& word) {
	os << std::get<0>(word) << ": '";
	for (const auto& c : std::get<1>(word)) os << c;
	os << "'";
	return os;
}
