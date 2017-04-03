//
//  crossword_type.cc
//  CrosswordCreator
//
//  Created by Hunter Knepshield on 4/2/17.
//  Copyright © 2017 Hunter Knepshield. All rights reserved.
//

#include "crossword_type.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

const char Crossword::WILDCARD = '.';
const char Crossword::BLACK_SQUARE = '_';
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
			wordMap[beginningLocation] = word;
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

std::pair<bool, Crossword> Crossword::Solve(
	Crossword puzzle, const std::vector<std::string>& wordlist,
	bool randomWordlistSelection, int verbosity) {
	Word mostConstrained;
	if (!puzzle.mostConstrained(&mostConstrained)) return {true, puzzle};

	switch (verbosity) {
		case 2:
		case 1:
			std::cout << "Attempting to fill in " << mostConstrained
					  << std::endl;
		default:
			break;
	}

	int row, column;
	WordDirection direction;
	std::tie(row, column, direction) = std::get<0>(mostConstrained);
	const std::vector<char>& characters = std::get<1>(mostConstrained);
	// First, throw out any words that:
	// - Are the wrong length,
	// - Don't match the current wildcard pattern, or
	// - Already appear elsewhere in the puzzle.
	std::vector<std::string> possibilities(wordlist.size());
	// Easiest filter: length
	auto wordLength = characters.size();
	// Next filter: regex
	std::string pattern(characters.begin(), characters.end());
	std::regex regex(pattern, std::regex::icase);
	// Final filter: existing words
	std::set<std::string> existingWords;
	for (const auto& beginningAndWord : puzzle.words_) {
		const auto& wordCharacters = std::get<1>(beginningAndWord.second);
		if (std::find(wordCharacters.begin(), wordCharacters.end(), WILDCARD) ==
			wordCharacters.end())
			existingWords.insert(
				std::string(wordCharacters.begin(), wordCharacters.end()));
	}
	auto filterIter = std::copy_if(
		wordlist.begin(), wordlist.end(), possibilities.begin(),
		[&](const std::string& s) {
			return s.size() == wordLength && std::regex_match(s, regex) &&
				   existingWords.find(s) == existingWords.end();
		});
	possibilities.resize(std::distance(possibilities.begin(), filterIter));

	switch (verbosity) {
		case 2:
			for (const auto& possibility : possibilities) {
				std::cout << possibility << std::endl;
			}
		case 1:
			std::cout << "Found " << possibilities.size() << " possibilit"
					  << (possibilities.size() == 1 ? "y" : "ies")
					  << " in the word list." << std::endl;
		default:
			break;
	}
	if (possibilities.size() == 0) return {false, puzzle};

	if (randomWordlistSelection) {
		std::random_device randomDevice;
		std::mt19937 generator(randomDevice());
		std::shuffle(possibilities.begin(), possibilities.end(), generator);
	}

	for (const auto& possibility : possibilities) {
		// Set the word in the puzzle. Undo if we fail anywhere along the way.
		std::vector<bool> undo(wordLength, false);
		for (int i = 0; i < wordLength; i++) {
			if (characters[i] == WILDCARD) {
				undo[i] = true;
				bool result = puzzle.setCharacter(
					possibility[i], direction == ACROSS ? row : row + i,
					direction == ACROSS ? column + i : column);
				if (!result) {
					switch (verbosity) {
						case 2:
						case 1:
							std::cout << "Failed to set character " << i + 1
									  << " of '" << possibility
									  << "' to fill in " << mostConstrained
									  << std::endl;
							std::cout << puzzle << std::endl;
						default:
							break;
					}
					goto possibilityFailed;
				}
			}
			// If this isn't a wildcard, we're guaranteed a match because of the
			// filtering from above.
		}
		std::cout << "Used '" << possibility << "'" << std::endl;
		std::cout << puzzle << std::endl;
		// We've successfully set every character for this possibility. Recurse.
		{
			// Inner scope to prevent issues with the compiler complaining about
			// initialization being skipped by the goto.
			const auto& solvedWithPuzzle =
				Solve(puzzle, wordlist, randomWordlistSelection, verbosity);
			if (solvedWithPuzzle.first) return solvedWithPuzzle;
		}
	possibilityFailed:
		// Undo everything in this word that used to be a wildcard.
		for (int i = 0; i < wordLength; i++) {
			if (undo[i])
				puzzle.clearCharacter(
					direction == ACROSS ? row : row + i,
					direction == ACROSS ? column + i : column);
		}
		switch (verbosity) {
			case 2:
			case 1:
				std::cout << "Failed to use '" << possibility << "' to fill in "
						  << mostConstrained << std::endl;
				std::cout << "Reverted to previous puzzle state:" << std::endl;
				std::cout << puzzle << std::endl;
			default:
				break;
		}
	}
	// We've exhausted all possibilities at this level, backtrack.
	return {false, puzzle};
}

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
