#ifndef _boggle_h
#define _boggle_h

#include <iostream>
#include <string>
#include "lexicon.h"
#include "grid.h"
using namespace std;

class Boggle {
public:
	//Functions defined by spec
    Boggle(Lexicon& dictionary, string boardText = "");
    char getLetter(int row, int col);
    bool checkWord(string word);
    bool humanWordSearch(string word);
    Set<string> computerWordSearch();
    int getScoreHuman();
    int getScoreComputer();
    friend ostream& operator<<(ostream& out, Boggle& boggle);

	//Useful functions
	Set<string> getFoundWords() const;
	string toString2D() const;
	static const int SIZE = 4;

private:
	//Private helper functions for recursion, etc.
	void initCubeVector(Vector<string> &cubeVect);
	void genBoardText(string &text);
	void genStateGrid(string &boardText);
	bool humanWordSearchHelper(string word, int row, int col, Grid<bool> &visited);
	void computerFindWords(string word, int row, int col, Grid<bool> &visited, Set<string> &found, Lexicon &dict);

	//Data structures
	Lexicon wordList;
	Grid<char> cubeGrid;
	Set<string> foundWords;
	int humanScore;
	int computerScore;
};

#endif // _boggle_h
