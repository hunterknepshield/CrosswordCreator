//
//  crossword_solve.cc
//  CrosswordCreator
//
//  Created by Hunter Knepshield on 4/4/17.
//  Copyright © 2017 Hunter Knepshield. All rights reserved.
//

// This file contains implementations for all Crossword::Solve methods.

#include "crossword_type.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

std::pair<bool, Crossword> Crossword::Solve(
	Crossword puzzle, const std::set<std::string>& wordlist,
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
				// If we just completed a crossing word, make sure that it's
				// valid too.
				const auto& modifiedCell =
					puzzle.grid_[direction == ACROSS ? row : row + i]
								[direction == ACROSS ? column + i : column];
				const auto& crossingWordBeginning =
					direction == ACROSS ? std::get<2>(modifiedCell)
										: std::get<1>(modifiedCell);
				const auto& crossingWord = puzzle.words_[crossingWordBeginning];
				const auto& crossingCharacters = std::get<1>(crossingWord);
				if (std::find(crossingCharacters.begin(),
							  crossingCharacters.end(),
							  WILDCARD) == crossingCharacters.end()) {
					// No wildcards, so we've just completed this word.
					std::string crossingString(crossingCharacters.begin(),
											   crossingCharacters.end());
					if (wordlist.find(crossingString) == wordlist.end()) {
						// We generated an invalid word.
						switch (verbosity) {
							case 2:
							case 1:
								std::cout
									<< "Failed to set word " << possibility
									<< " because it broke the word crossing at "
									   "character "
									<< i + 1 << " ('" << crossingString << "')"
									<< std::endl;
								std::cout << puzzle << std::endl;
							default:
								break;
						}
						goto possibilityFailed;
					}
				}
			}
			// If this isn't a wildcard, we're guaranteed a match of
			// characters[i] and possibility[i] because of the filtering that's
			// done above.
		}
		switch (verbosity) {
			case 2:
			case 1:
				std::cout << "Used '" << possibility << "'" << std::endl;
				std::cout << puzzle << std::endl;
			default:
				break;
		}
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
