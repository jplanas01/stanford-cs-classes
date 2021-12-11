/* life-extra.cpp
 * --------
 * By Juan Planas (Eric Yu)
 * Implementation of Conway's Game of Life. Given an initial colony of
 * "bacteria", evolve each one according to a simple set of rules.
 * This program has the following extensions:
 *	-Wrapping (line 197)
 *	-GUI (line 165)
 */

#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "console.h"
#include "filelib.h"
#include "grid.h"
#include "gwindow.h"
#include "simpio.h"
#include "lifegui.h"
using namespace std;

void printIntro();
void initBoard(Grid<bool> &board, ifstream &file);
void printBoard(const Grid<bool> &board);
void userInputLoop(Grid<bool> &board);
unsigned int countNeighbors(const Grid<bool> &board, int row, int col);
void evolveBoard(Grid<bool> &board, Grid<bool> &tempBoard);
void animateNFrames(Grid<bool> &board, Grid<bool> &tempBoard, LifeGUI &gui, int frames);
void initGui(Grid<bool> &board, LifeGUI &gui);
void printGuiBoard(Grid<bool> &board, LifeGUI &gui);

int main() {
	Grid<bool> board(0, 0, false);
	ifstream colonyFile;

	printIntro();
	promptUserForFile(colonyFile, "Grid input file name? ");

	initBoard(board, colonyFile);
	colonyFile.close();
	userInputLoop(board);

    cout << "Have a nice Life!" << endl;
	exitGraphics();
    return 0;
}

/* printIntro
 * ----------
 * Prints out the initial introductory message.
 */
void printIntro() {
	cout << "Welcome to the CS 106B Game of Life," << endl;
	cout << "a simulation of the lifecycle of a bacteria colony." << endl;
	cout << "Cells (X) live and die by the following rules:" << endl;
	cout << "- A cell with 1 or fewer neighbors dies." << endl;
	cout << "- Locations with 2 neighbors remain stable." << endl;
	cout << "- Locations with 3 neighbors will create life." << endl;
	cout << "- A cell with 4 or more neighbors dies." << endl;
	cout << endl;
	cout << "cocks" << endl;
}

/* initBoard
 * ---------
 * Using an open ifstream that points to the colony file as defined in the
 * spec. It reads in the number of rows, columns, resizes the board to be that
 * size, and fills in the necessary cells. It is not necessary to set cells as
 * empty because resize takes care of that. Closing the ifstream is left to the
 * calling function.
 */
void initBoard(Grid<bool> &board, ifstream &file) {
	int rows, columns;
	string tempStr;

	getline(file, tempStr);
	rows = stringToInteger(tempStr);

	getline(file, tempStr);
	columns = stringToInteger(tempStr);

	board.resize(rows, columns);

	for (int i = 0; i < rows; i++) {
		getline(file, tempStr);
		for (int j = 0; j < columns; j++) {
			if (tempStr[j] == 'X') {
				board[i][j] = true;
			}
		}
	}
}

/* printBoard
 * ----------
 * Given a board, prints a '-' if a cell is dead and 'X' if it is alive.
 */
void printBoard(const Grid<bool> &board) {
	for (int i = 0; i < board.height(); i++) {
		for (int j = 0; j < board.width(); j++) {
			if (board[i][j]) {
				cout << 'X';
			} else {
				cout << '-';
			}
		}
		cout << endl;
	}
}

/* animateNFrames
 * --------------
 * Given a certain number of frames, evolves the given board using tempBoard as
 * a scratch board that number of times. The console is cleared between
 * each frame.
 */
void animateNFrames(Grid<bool> &board, Grid<bool> &tempBoard, LifeGUI &gui, int frames) {
	for (int i = 0; i < frames; i++) {
		clearConsole();
		evolveBoard(board, tempBoard);
		printBoard(board);
		printGuiBoard(board, gui);
		pause(50);
	}
}

/* userInputLoop
 * -------------
 * Prompts the user for their choice in animating a number of frames, a single
 * frame, or quitting the program. If the user inputs a wrong choice, he is
 * informed and reprompted.
 * Takes in an already initialized board.
 */
void userInputLoop(Grid<bool> &board) {
	string choice = "";
	Grid<bool> tempBoard = board;
	LifeGUI gui;

	initGui(board, gui);
	printBoard(board);

	while (choice != "q" && choice != "Q") {
		choice = getLine("a)nimate, t)ick, q)uit? ");
		if (choice == "a" || choice == "A") {
			animateNFrames(board, tempBoard, gui, getInteger("How many frames? "));
		} else if (choice == "t" || choice == "T") {
			evolveBoard(board, tempBoard);
			printBoard(board);
			printGuiBoard(board, gui);
		} else if (choice != "q" && choice != "Q") {
			cout << "Invalid choice; please try again." << endl;
		}
	}
}

/* printGuiBoard
 * -------------
 * Updates the Life GUI using the given board.
 * Assumes that the GUI is of the same size as board (ie, properly
 * initialized).
 */
void printGuiBoard(Grid<bool> &board, LifeGUI &gui) {
	for (int i = 0; i < board.height(); i++) {
		for (int j = 0; j < board.width(); j++) {
			if (board[i][j]) {
				gui.drawCell(i, j, true);
			} else {
				gui.drawCell(i, j, false);
			}
		}
	}
}

/* initGui
 * -------
 * Initializes and displays the LifeGUI
 */
void initGui(Grid<bool> &board, LifeGUI &gui) {
	gui.resize(board.height(), board.width());
	printGuiBoard(board, gui);
}

/* countNeighbors
 * --------------
 * Given a specific cell (ie, the row and column of that cell) in an initialized
 * board, count the nieghbors around that cell has without wrapping around and
 * return that number.
 */
unsigned int countNeighbors(const Grid<bool> &board, int row, int col) {
	unsigned int count = 0;

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			int currRow = (row + i) % board.height();
			int currCol = (col + j) % board.width();

			if (currRow < 0) {
				currRow += board.height();
			}
			if (currCol < 0) {
				currCol += board.width();
			}

			if (board[currRow][currCol]) {
				count++;
			}
		}
	}

	//Don't count the given cell if it's alive
	//It's less efficient to skip the cell in the loops above than it is to do
	//this.
	if (board[row][col]) {
		count--;
	}
	return count;
}

/* evolveBoard
 * -----------
 * Given an initialized board and a scratch board, update the status (alive or
 * dead) of each cell by a single step according to the number of neighbors it
 * has and the rules of Life. The board is updated to contain the evolved
 * colony.
 */
void evolveBoard(Grid<bool> &board, Grid<bool> &tempBoard) {
	unsigned int neighbors;
	for (int i = 0; i < board.height(); i++) {
		for (int j = 0; j < board.width(); j++) {	
			neighbors = countNeighbors(board, i, j);
			if (neighbors < 2) {
				tempBoard[i][j] = false;
			} else if (neighbors >= 4) {
				tempBoard[i][j] = false;
			} else if (neighbors == 3) {
				tempBoard[i][j] = true;
			} else {
				tempBoard[i][j] = board[i][j];
			}
		}
	}
	board = tempBoard;
}
