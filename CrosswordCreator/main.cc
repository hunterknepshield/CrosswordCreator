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

// Verbosity settings. 0 = silent, 1 = print some things, 2 = print everything.
int verbosity = 1;
// Randomness settings. If false, the first valid word from the wordlist is
// inserted, resulting in a puzzle that has lots of 'A' words.
bool randomWordlistSelection = true;

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
	// Dedupe with a set.
	if (verbosity > 0) {
		std::cout << "Read " << wordlist.size()
				  << " words from the wordlist. Deduping..." << std::endl;
	}
	std::set<std::string> dedupedWordlist;
	for (const auto& word : wordlist) {
		std::string copy = word;
		for (auto& character : copy) {
			if (character >= 'a' && character <= 'z')
				character = (character - 'a') + 'A';
		}
		dedupedWordlist.insert(copy);
	}
	wordlist.clear();
	wordlist.assign(dedupedWordlist.begin(), dedupedWordlist.end());
	std::cout << "Read " << wordlist.size() << " words from the wordlist."
			  << std::endl;
	std::cout << std::endl;

	std::cout << "Initial puzzle:" << std::endl;
	std::cout << crossword << std::endl;

	Crossword::Solve(crossword, wordlist, randomWordlistSelection, verbosity);

	return 0;
}
