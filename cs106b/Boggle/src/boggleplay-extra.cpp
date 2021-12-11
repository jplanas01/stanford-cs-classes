/* boggleplay-extra.cpp
 * --------------
 * Implements the Boggle user interface and initializes the Boggle game.
 */

#include "lexicon.h"
#include "Boggle-extra.h"
#include <ctype.h>
#include "strlib.h"
#include "simpio.h"
#include "console.h"
#include "bogglegui.h"

/* Determines whether the given string contains any invalid characters */
bool isValidBoard(string &temp) {
	for (int i = 0; i < temp.length(); i++) {
		if (!isalpha(temp[i])) {
			return false;
		}
	}
	return true;
}

/* Gets a valid 16-letter long string representing the state of the grid. */
void getBoardString(string &text) {
	string temp;
	bool valid = false;
	do {
		temp = getLine("Type the 16 letters to appear on the board: ");
		valid = isValidBoard(temp);
		if (!valid) {
			cout << "This is not a valid 16-letter board string. Try again." << endl;
		}
	} while (!valid);
	text = toUpperCase(temp);
}

/* Prints out the current human found words, grid, and score */
void printState(Boggle &boggle) {
	Set<string> found = boggle.getFoundWords();
	cout << boggle << endl << endl;
	cout << "Your words: (" << found.size() << "): ";
	cout << found << endl;
	cout << "Your score: " << boggle.getScoreHuman() << endl;
}

/* Prints to both the GUI status message and the console */
void dualPrint(string msg) {
	BoggleGUI::setStatusMessage(msg);
	cout << msg << endl;
}

/* Runs the human turn loop. Prints the current state of the human game, Asks
 * for a word, determines if it is valid and displays a message depending on if
 * the word was valid or not
 */
void humanTurn(Boggle &boggle) {
	clearConsole();
	cout << "It's your turn!" << endl;
	printState(boggle);
	string word;

	while ((word = toUpperCase(getLine("Type a word (or Enter to stop): "))) != "") {
		clearConsole();

		if (!boggle.checkWord(word)) {
			dualPrint("You must enter an unfound 4+ letter word from the dictionary.");
		} else if (!boggle.humanWordSearch(word)) {
			dualPrint("That word can't be formed on this board.");
		} else {
			dualPrint("You found a new word! \"" + word + "\"");
		}

		printState(boggle);
	}
}

/* Runs the computer turn. Finds all the possible words, displays them, and
 * prints a message depending on who won.
 */
void computerTurn(Boggle &boggle) {
	Set<string> found = boggle.computerWordSearch();

	cout << endl;
	cout << "It's my turn!" << endl;
	cout << "My words (" << found.size() << "): " << found << endl;
	cout << "My score: " << boggle.getScoreComputer() << endl;
	if (boggle.getScoreHuman() >= boggle.getScoreComputer()) {
		dualPrint("WOW, you defeated me! Congratulations!");
	} else {
		dualPrint("Ha ha ha, I destroyed you. Better luck next time, puny human");
	}
}

/* Plays a single game of Boggle. Initializes the board and GUI and runs the
 * computer and human turns.
 */
void playOneGame(Lexicon& dictionary) {

	if (!BoggleGUI::isInitialized()) {
		BoggleGUI::initialize(Boggle::SIZE, Boggle::SIZE);
	} else {
		BoggleGUI::reset();
		BoggleGUI::setStatusMessage("");
	}

	string boardText = "";
	if (!getYesOrNo("Do you want to generate a random board?" )) {
		getBoardString(boardText);
	}
	Boggle boggle(dictionary, boardText);
	
	humanTurn(boggle);
	computerTurn(boggle);
}
