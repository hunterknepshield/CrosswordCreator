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
#include <string>
#include <tuple>
#include <utility>
#include <vector>

/// A representation of a crossword puzzle.
class Crossword {
   public:
	/// A specification for a word's direction.
	enum WordDirection { ACROSS, DOWN };
	/// A character in a word that is unknown as of right now.
	static const char WILDCARD;
	/// The character representing a black square in the grid.
	static const char BLACK_SQUARE;
	/// A type representing a starting location and direction of a word.
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

	/// Wrapper around the nasty nested tuple initialization.
	static Word MakeWord(int row, int column, WordDirection direction,
						 const std::vector<char>& characters) {
		return std::make_tuple(std::make_tuple(row, column, direction),
							   characters);
	}

	/// Takes a list of words, copies them into the instance and generates a
	/// grid from the words.
	static std::unique_ptr<Crossword> Create(int height, int width,
											 const std::vector<Word>& words);
	/// Takes a pre-populated grid and derives the word locations from that.
	static std::unique_ptr<Crossword> Create(
		const std::vector<std::string>& rawGrid);

	/// Takes a partially solved instance and uses a heuristic that attempts to
	/// fill in the most-constrained word first using the supplied word list.
	static std::pair<bool, Crossword> Solve(
		Crossword puzzle, const std::vector<std::string>& wordlist,
		bool randomWordlistSelection, int verbosity = 0);

	friend std::ostream& operator<<(std::ostream& os, const Crossword& cw);

   private:
	/// An intermediate value that is the default for a not-yet-populated grid.
	/// Should never appear in a valid Crossword instance.
	static const Cell DEFAULT_CELL;

	Crossword(int height, int width, const std::map<WordBeginning, Word>& words,
			  const std::vector<std::vector<Cell>>& grid)
		: height_(height), width_(width), words_(words), grid_(grid) {}

	/// Finds the word that is the most constrained in the given puzzle. If no
	/// unconstrained words are present in the puzzle, returns false.
	bool mostConstrained(Word* out);

	/// Sets the specified cell in the board to be the specified character.
	/// Returns false if the operation failed for some reason. Normalizes all
	/// characters to uppercase.
	bool setCharacter(char value, int row, int column);
	/// Resets the specified cell in the board to be the wildcard character.
	inline bool clearCharacter(int row, int column) {
		return setCharacter(WILDCARD, row, column);
	}

	int height_, width_;
	std::map<WordBeginning, Word> words_;
	std::vector<std::vector<Cell>> grid_;
};

// Custom stream operators to make debugging prettier.
std::ostream& operator<<(std::ostream& os,
						 const Crossword::WordBeginning& wordBeginning);
std::ostream& operator<<(std::ostream& os, const Crossword::Word& word);

#endif /* crossword_type_h */
