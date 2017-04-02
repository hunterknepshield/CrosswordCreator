//
//  main.cc
//  CrosswordCreator
//
//  Created by Hunter Knepshield on 4/2/17.
//  Copyright © 2017 Hunter Knepshield. All rights reserved.
//

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "crossword_type.h"

int main(void) {
	int height, width;
	std::cout << "Input puzzle height and width..." << std::endl;
	std::cin >> height;
	std::cin >> width;

	std::cout << std::endl;

	int r, c, l;
	char dir;
	std::vector<Crossword::Word> words;
	std::cout << "Input word as row, column, length, A|D (-1 to stop)..."
			  << std::endl;
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
				return 1;
		}

		words.push_back(Crossword::MakeWord(r, c, direction, characters));
	}

	auto crossword = Crossword::Create(height, width, words);
	if (crossword) {
		std::cout << "Created a Crossword instance:" << std::endl;
		std::cout << crossword.get();
	} else {
		std::cout << "Failed to create Crossword instance." << std::endl;
		return 1;
	}

	return 0;
}
