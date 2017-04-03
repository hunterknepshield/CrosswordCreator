//
//  crossword_type.h
//  CrosswordCreator
//
//  Created by Hunter Knepshield on 4/2/17.
//  Copyright © 2017 Hunter Knepshield. All rights reserved.
//

#ifndef crossword_type_h
#define crossword_type_h

#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

class Crossword {
   public:
	/// A specification for a word's direction.
	enum WordDirection { ACROSS, DOWN };
	/// A character in a word that is unknown as of right now.
	static const char WILDCARD;
	/// The character representing a black square in the grid.
	static const char BLACK_SQUARE;
	/// A type representing a starting location and
	typedef std::tuple<int, int, WordDirection> WordBeginning;
	/// A tuple denoting a starting row and column, direction, and current
	/// values of a particular word in the puzzle.
	typedef std::tuple<WordBeginning, std::vector<char>> Word;
	/// A tuple denoting the across word and down word that this cell is a part
	/// of. If either value is INVALID_{DIRECTION}, that implies an absence of
	/// word in that particular direction.
	typedef std::tuple<char, WordBeginning, WordBeginning> Cell;
	/// An value indicating no across word in this cell.
	static const WordBeginning INVALID_ACROSS;
	/// An value indicating no down word in this cell.
	static const WordBeginning INVALID_DOWN;

	static Word MakeWord(int row, int column, WordDirection direction,
						 std::vector<char> characters) {
		return std::make_tuple(std::make_tuple(row, column, direction),
							   characters);
	}

	/// Takes a list of words, copies them into the instance and generates a
	/// grid from the words.
	static std::unique_ptr<Crossword> Create(int height, int width,
											 const std::vector<Word>& words);

	friend std::ostream& operator<<(std::ostream& os, const Crossword& cw);

	//   private:  // TODO uncomment this once ready
	/// An intermediate value that is the default for a not-yet-populated grid.
	/// Should never appear in a valid Crossword instance.
	static const Cell DEFAULT_CELL;

	Crossword(int height, int width, const std::map<WordBeginning, Word>& words,
			  const std::vector<std::vector<Cell>>& grid)
		: height_(height), width_(width), words_(words), grid_(grid) {}

	/// Finds the word that is the most constrained in the given puzzle. If no
	/// unconstrained words are present in the puzzle, returns INVALID_ACROSS.
	Word mostConstrained();

	int height_, width_;
	std::map<WordBeginning, Word> words_;
	std::vector<std::vector<Cell>> grid_;
};

std::ostream& operator<<(std::ostream& os,
						 const Crossword::WordBeginning& wordBeginning);
std::ostream& operator<<(std::ostream& os, const Crossword::Word& word);

#endif /* crossword_type_h */
