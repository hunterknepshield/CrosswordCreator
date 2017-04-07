//
//  main.cc
//  CrosswordCreator
//
//  Created by Hunter Knepshield on 4/2/17.
//  Copyright © 2017 Hunter Knepshield. All rights reserved.
//

#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "crossword_type.h"

enum InputType { WORDS, GRID };
// Input settings. WORDS = input word tuples, GRID = input grid.
InputType inputSetting = GRID;
// Verbosity settings. 0 = silent, 1 = print some things, 2 = print everything.
int verbosity = 1;
// Randomness settings. If false, the first valid word from the wordlist is
// inserted, resulting in a puzzle that has lots of 'A' words.
bool randomWordlistSelection = true;

int main(void) {
	std::unique_ptr<Crossword> crosswordPtr = nullptr;
	switch (inputSetting) {
		case WORDS: {
			int height, width;
			std::cout << "Input puzzle height and width..." << std::endl;
			std::cin >> height;
			std::cin >> width;
			std::cout << std::endl;

			std::vector<Crossword::Word> words;
			std::cout
				<< "Input word as row, column, length, A|D (-1 to stop)..."
				<< std::endl;
			int r, c, l;
			char dir;
			while (true) {
				std::cin >> r;
				if (r == -1) break;
				std::cin >> c;
				std::cin >> l;
				std::vector<char> characters(l, Crossword::WILDCARD);
				std::cin >> dir;
				Crossword::WordDirection direction;
				switch (dir) {
					case 'a':
					case 'A':
						direction = Crossword::ACROSS;
						break;
					case 'd':
					case 'D':
						direction = Crossword::DOWN;
						break;
					default:
						std::cerr << "Invalid direction." << std::endl;
						continue;
				}

				words.push_back(
					Crossword::MakeWord(r, c, direction, characters));
			}
			std::cout << std::endl;
			crosswordPtr = Crossword::Create(height, width, words);
		}
		case GRID: {
			int height;
			std::cout << "Input puzzle height..." << std::endl;
			std::cin >> height;
			std::cout << std::endl;

			std::cout << "Input grid..." << std::endl;
			std::vector<std::string> rawGrid;
			std::string s;
			for (int i = 0; i < height; i++) {
				std::cin >> s;
				rawGrid.push_back(s);
			}
			std::cout << std::endl;
			crosswordPtr = Crossword::Create(rawGrid);
		}
	}
	// Ensure we actually generated a valid instance before continuing.
	if (!crosswordPtr) {
		std::cerr << "Failed to create a Crossword instance." << std::endl;
		return 1;
	}
	auto& crossword = *crosswordPtr;

	// Read in the wordlist.
	std::vector<std::string> wordlist;
	std::cout << "Input wordlist file (e.g. /usr/share/dict/words)..."
			  << std::endl;
	std::string filename;
	std::cin >> filename;
	std::ifstream file(filename);
	std::copy(std::istream_iterator<std::string>(file),
			  std::istream_iterator<std::string>(),
			  std::back_inserter(wordlist));
	// Dedupe with a set.
	if (verbosity > 0) {
		std::cout << "Read " << wordlist.size()
				  << " words from the wordlist. Deduping..." << std::endl;
	}
	std::set<std::string> dedupedWordlist;
	for (auto& word : wordlist) {
		for (auto& character : word) {
			if (character >= 'a' && character <= 'z')
				character = (character - 'a') + 'A';
		}
		dedupedWordlist.insert(word);
	}
	std::cout << "Read " << dedupedWordlist.size()
			  << " words from the wordlist." << std::endl;
	std::cout << std::endl;

	std::cout << "Initial puzzle:" << std::endl;
	std::cout << crossword.printEverything() << std::endl;

	const auto& result = Crossword::Solve(crossword, dedupedWordlist,
										  randomWordlistSelection, verbosity);
	if (result.first) {
		std::cout << "Generated a puzzle:" << std::endl;
		std::cout << result.second.printEverything() << std::endl;
	} else {
		std::cout << "Failed to generate a puzzle." << std::endl;
	}

	return 0;
}
