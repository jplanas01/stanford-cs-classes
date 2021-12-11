/* Boggle-extra.cpp
 * ------
 * By Juan Planas (TA: Eric Yu)
 * Implementation of the classic Boggle game with a simple GUI.
 * Extensions:
 *	-5x5 Boggle (various lines)
 */

#include "Boggle-extra.h"
#include "shuffle.h"
#include "grid.h"
#include "lexicon.h"
#include <math.h>
#include "bogglegui.h"

// letters on all 6 sides of every cube
static string CUBES[16] = {
    "AAEEGN", "ABBJOO", "ACHOPS", "AFFKPS",
    "AOOTTW", "CIMOTU", "DEILRX", "DELRVY",
    "DISTTY", "EEGHNW", "EEINSU", "EHRTVW",
    "EIOSST", "ELRTTY", "HIMNQU", "HLNNRZ"
};

// letters on every cube in 5x5 "Big Boggle" version (extension)
static string BIG_BOGGLE_CUBES[25] = {
   "AAAFRS", "AAEEEE", "AAFIRS", "ADENNN", "AEEEEM",
   "AEEGMU", "AEGMNN", "AFIRSY", "BJKQXZ", "CCNSTW",
   "CEIILT", "CEILPT", "CEIPST", "DDLNOR", "DDHNOT",
   "DHHLOR", "DHLNOR", "EIIITT", "EMOTTT", "ENSSSU",
   "FIPRSY", "GORRVW", "HIPRRY", "NOOTUW", "OOOTTU"
};

/* Adds all the strings in the CUBES array to cubeVect */
void Boggle::initCubeVector(Vector<string> &cubeVect) {
	string *cubeArr = BIG_BOGGLE_CUBES;
	for (int i = 0; i < SIZE * SIZE; i++) {
		cubeVect.add(cubeArr[i]);
	}
}

/* Generates a 16-letter string that represents the game board by picking a
 * random letter from each cube string.
 */
void Boggle::genBoardText(string &text) {
	Vector<string> cubeVect;
	initCubeVector(cubeVect);
	shuffle(cubeVect);
	for (int i = 0; i < cubeVect.size(); i++) {
		text += cubeVect[i][randomInteger(0, cubeVect[i].length() - 1)];
	}
}

/* Fills the state Grid with the 16-letter string in boardText
 * Assumes that boardText is of length 16, 25, or 36 and does not check for the
 * validity of characters in boardText.
 */
void Boggle::genStateGrid(string &boardText) {
	//Since grid is square, length of each side is square root of number of
	//elements in it
	cubeGrid.resize(SIZE, SIZE);

	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j <  SIZE; j++) {
			cubeGrid[i][j] = boardText[SIZE * i + j];
		}
	}
}

/* Constructor for Boggle class, initializes the game Grid (randomly if
 * necessary) and initializes the GUI.
 * Assumes boardText is either blank or a valid representation of the board of
 * length 16 or 25
 */
Boggle::Boggle(Lexicon& dictionary, string boardText) {
	if (boardText == "") {
		genBoardText(boardText);
	}
	genStateGrid(boardText);
	wordList = dictionary;

	BoggleGUI::setAnimationDelay(100);
	BoggleGUI::labelAllCubes(boardText);
	computerScore = 0;
	humanScore = 0;
	foundWords.clear();
}

/* Returns the letter at the specified row and column.
 * Assumes that the internal Grid has been initialized.
 */
char Boggle::getLetter(int row, int col) {
	if (!cubeGrid.inBounds(row, col)) {
		throw -1;
	}
	return cubeGrid.get(row, col);
}

/* Checks if a current word is valid, i.e., that it is contained in the
 * dictionary, is at least 4 characters long, and hasn't been found yet.
 */
bool Boggle::checkWord(string word) {
	if (wordList.contains(word) && word.length() >= 4 && !foundWords.contains(word)) {
		return true;
	}
	return false;
}

/* Verify that the word inputted by the human is formable on the current
 * grid.
 * Assumes properly initialized visited (ie, as large or larger than the game
 * Grid) and game Grids.
 */
bool Boggle::humanWordSearchHelper(string word, int row, int col, Grid<bool> &visited) {
	//Base cases
	if (!cubeGrid.inBounds(row, col)) {
		return false;
	} else if (word[0] != getLetter(row, col)) {
		return false;
	} else if (word.length() == 1) {
		BoggleGUI::setHighlighted(row, col, true);
		return true;
	}

	visited.set(row, col, true);
	word = word.substr(1);
	BoggleGUI::setHighlighted(row, col, true);

	//Recurse into all surrounding, non-visted letters to see if the word can
	//be formed
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			int curY = row + i;
			int curX = col + j;
			if (!visited.inBounds(curY, curX)) {
				continue;
			}
			if (!visited.get(curY, curX) &&
					getLetter(curY, curX) == word[0]) {
				if (humanWordSearchHelper(word, curY, curX, visited)) {
					return true;
				}
			}
		}
	}
	
	visited.set(row, col, false);
	BoggleGUI::setHighlighted(row, col, false);
	return false;

}

/* Verifies that the human-inputted word is formable on the current grid. If it
 * is, the score from the word is added to the human score and the word is
 * recorded in the GUI and foundWord Set.
 */
bool Boggle::humanWordSearch(string word) {
	BoggleGUI::clearHighlighting();
	word = toUpperCase(word);
	Grid<bool> visited(SIZE, SIZE, false);
    for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			BoggleGUI::setHighlighted(i, j, true);
			if (humanWordSearchHelper(word, i, j, visited)) {
				foundWords.add(word);
				humanScore += (word.length() - 3);
				BoggleGUI::recordWord(word, BoggleGUI::HUMAN);
				BoggleGUI::setScore(humanScore, BoggleGUI::HUMAN);
				return true;
			}
			BoggleGUI::setHighlighted(i, j, false);
		}
	}
	return false;
}

/* Returns the current human score. */
int Boggle::getScoreHuman() {
	return humanScore;
}

/* Finds all words formable from the current row and column with the prefix in
 * word. Adds all found words to the found Set and adds to the computer score
 * as appropriate and also records the word in the GUI.
 * Assumes properly initialized visited Grid and found Set (ie, visited is of
 * the same dimensions as the game grid or bigger.
 */
void Boggle::computerFindWords(string word, int row, int col, Grid<bool> &visited, Set<string> &found, Lexicon &dict) {
	if (!cubeGrid.inBounds(row, col)) {
		return;
	}

	word += getLetter(row, col);
	if (!dict.containsPrefix(word)) {
		//No words with this prefix, prune.
		return;
	} else if (checkWord(word)) {
		found.add(word);
		computerScore += (word.length() - 3);
		BoggleGUI::recordWord(word, BoggleGUI::COMPUTER);
	}


	visited.set(row, col, true);

	//Recurse into all non-visited adjacent squares
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			int curY = row + i;
			int curX = col + j;
			if (!visited.inBounds(curY, curX)) {
				continue;
			}
			if (!visited.get(curY, curX)) {
				computerFindWords(word, curY, curX, visited, found, dict);
			}
		}
	}
	visited.set(row, col, false);
}

/* Starts the computer word search on each cube on the grid and returns a set
 * of the found words.
 * Assumes a properly initialized game Grid.
 */
Set<string> Boggle::computerWordSearch() {
	Set<string> result;
	Grid<bool> visited(SIZE, SIZE, false);
    for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			computerFindWords("", i, j, visited, foundWords, wordList);
		}
	}
	result = foundWords;
    return result;
}

/* Returns the current computer score */
int Boggle::getScoreComputer() {
	return computerScore;
}

/* Returns the set of words found so far. */
Set<string> Boggle::getFoundWords() const {
	return foundWords;
}

/* Creates a 2D string representation of the game Grid */
string Boggle::toString2D() const {
	return cubeGrid.toString2D("","","","\n");
}

/* For cout convenience, converts the grid to a 2D string and prints it. */
ostream& operator<<(ostream& out, Boggle& boggle) {
	cout << boggle.toString2D();
    return out;
}
