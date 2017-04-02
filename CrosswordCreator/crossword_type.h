//
//  crossword_type.h
//  CrosswordCreator
//
//  Created by Hunter Knepshield on 4/2/17.
//  Copyright © 2017 Hunter Knepshield. All rights reserved.
//

#ifndef crossword_type_h
#define crossword_type_h

#include <tuple>
#include <utility>
#include <vector>

class Crossword {
   public:
	/// A specification for a word's direction.
	enum WordDirection { ACROSS, DOWN };
	/// A tuple denoting a starting row and column, direction, and current
	/// values of a particular word in the puzzle.
	typedef std::tuple<int, int, WordDirection, std::vector<char>> Word;
	/// A tuple denoting the across word and down word that this cell is a part
	/// of. If either value is nullptr, that implies an absence of word in a
	/// particular direction.
	typedef std::pair<Word*, Word*> Cell;

	/// Takes a list of words, copies them into the instance and generates a
	/// grid from the words.
	Crossword(const std::vector<Word>& words);

   private:
	std::vector<Word> words;
	std::vector<std::vector<Cell>> grid;
};

#endif /* crossword_type_h */
