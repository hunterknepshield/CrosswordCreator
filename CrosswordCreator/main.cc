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
#include <string>
#include <vector>

#include "crossword_type.h"

// Verbosity settings. 0 = silent, 1 = print some things, 2 = print everything.
int verbosity = 1;

int main(void) {
	int height, width;
	std::cout << "Input puzzle height and width..." << std::endl;
	std::cin >> height;
	std::cin >> width;
	std::cout << std::endl;

	std::vector<Crossword::Word> words;
	std::cout << "Input word as row, column, length, A|D (-1 to stop)..."
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

		words.push_back(Crossword::MakeWord(r, c, direction, characters));
	}
	std::cout << std::endl;
	// Make sure we can actually create this crossword before continuing.
	auto crosswordPtr = Crossword::Create(height, width, words);
	if (!crosswordPtr) {
		std::cout << "Failed to create a Crossword instance." << std::endl;
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
	std::cout << "Read " << wordlist.size() << " words from the wordlist."
			  << std::endl;
	std::cout << std::endl;

	//	bool result = crossword.setCharacter('A', 0, 0);
	//	if (!result) {
	//		std::cout << "Failed to set the character." << std::endl;
	//	}
	std::cout << crossword;

	Crossword::Solve(crossword, wordlist, verbosity);

	return 0;
}
