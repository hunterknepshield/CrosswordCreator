//
//  crossword_create.cc
//  CrosswordCreator
//
//  Created by Hunter Knepshield on 4/4/17.
//  Copyright © 2017 Hunter Knepshield. All rights reserved.
//

// This file contains implementations for all Crossword::Create methods.

#include "crossword_type.h"

#include <map>
#include <memory>
#include <tuple>
#include <vector>

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

std::unique_ptr<Crossword> Crossword::Create(
	const std::vector<std::string>& rawGrid) {
	// First make sure the grid is valid.
	int height = (int)rawGrid.size();
	if (height == 0) return nullptr;
	int width = (int)rawGrid.front().size();
	for (const auto row : rawGrid)
		if (row.size() != width) return nullptr;

	// These are the data structures that get passed to the constructor.
	std::map<WordBeginning, Word> wordMap;
	std::vector<std::vector<Cell>> grid;
	for (int i = 0; i < height; i++) {
		grid.emplace_back(width, DEFAULT_CELL);
	}
	for (int r = 0; r < height; r++) {
		for (int c = 0; c < width; c++) {
			// Normalize to uppercase.
			char cellChar = rawGrid[r][c];
			if (cellChar >= 'a' && cellChar <= 'z')
				cellChar = (cellChar - 'a') + 'A';
			std::get<0>(grid[r][c]) = cellChar;
		}
	}

	// Generate all the across words first
	int beginningR = -1, beginningC = -1;
	std::vector<char> characters;
	// For convenience since this logic needs to happen in a couple places.
	auto backfillAcross = [&]() {
		if (characters.size() > 1) {
			const auto word =
				MakeWord(beginningR, beginningC, ACROSS, characters);
			const auto& wordBeginning = std::get<0>(word);
			wordMap[wordBeginning] = word;
			for (int c = beginningC; c < beginningC + characters.size(); c++) {
				std::get<1>(grid[beginningR][c]) = wordBeginning;
			}
		}
		beginningR = -1;
		beginningC = -1;
		characters.clear();
	};
	for (int r = 0; r < height; r++) {
		for (int c = 0; c < width; c++) {
			if (rawGrid[r][c] != BLACK_SQUARE) {
				// Either start a new word or continue an existing word.
				if (beginningR == -1) {
					// Starting a new word.
					beginningR = r;
					beginningC = c;
				}
				// Always record the character for both new and existing words.
				characters.push_back(rawGrid[r][c]);
			} else {
				if (beginningR != -1) {
					// We're ending a word, backfill the grid to the left.
					backfillAcross();
				}
			}
		}
		if (beginningR != -1) {
			// This word ended at the end of the row.
			backfillAcross();
		}
	}

	// Now generate all the down words.
	auto backfillDown = [&]() {
		if (characters.size() > 1) {
			const auto word =
				MakeWord(beginningR, beginningC, DOWN, characters);
			const auto& wordBeginning = std::get<0>(word);
			wordMap[wordBeginning] = word;
			for (int r = beginningR; r < beginningR + characters.size(); r++) {
				std::get<2>(grid[r][beginningC]) = wordBeginning;
			}
		}
		beginningR = -1;
		beginningC = -1;
		characters.clear();
	};
	for (int c = 0; c < width; c++) {
		for (int r = 0; r < height; r++) {
			if (rawGrid[r][c] != BLACK_SQUARE) {
				// Either start a new word or continue an existing word.
				if (beginningR == -1) {
					// Starting a new word.
					beginningR = r;
					beginningC = c;
				}
				// Always record the character for both new and existing words.
				characters.push_back(rawGrid[r][c]);
			} else {
				if (beginningR != -1) {
					// We're ending a word, backfill the grid to the left.
					backfillDown();
				}
			}
		}
		if (beginningR != -1) {
			// This word ended at the end of the row.
			backfillDown();
		}
	}

	return std::unique_ptr<Crossword>(
		new Crossword(height, width, wordMap, grid));
}
