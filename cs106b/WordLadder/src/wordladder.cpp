/* WordLadder
 * ----------
 * By Juan Planas (TA: Eric Yu)
 * Finds the shortest word ladder between two words by changing one letter at a
 * time with valid intermediary words.
 */

#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include "console.h"
#include "lexicon.h"
#include "filelib.h"
#include "simpio.h"
#include "queue.h"
#include "hashset.h"
#include "stack.h"

using namespace std;

/* Function prototypes */
void printIntro();
bool validateInput(string &word1, string &word2, Lexicon &dict);
void lowerString(string &str);
void generateNeighbors(const string &word, Vector<string> &wordVect, Lexicon &dict);
void findWordLadder (const string &word1, const string &word2, Lexicon &dict);


int main() {
	printIntro();
	ifstream dictionaryFile;
	promptUserForFile(dictionaryFile, "Dictionary file name? ");
	
	Lexicon dictionary(dictionaryFile);

	string word1, word2;
	bool validWords = false;
	bool endLoop = false;

	while (!endLoop) {
		cout << endl;

		word1 = getLine("Word #1 (or Enter to quit): ");
		if (word1 == "") {
			endLoop = true;
			continue;
		}
		lowerString(word1);

		word2 = getLine("Word #2 (or Enter to quit): ");
		if (word2 == "") {
			endLoop = true;
			continue;
		}
		lowerString(word2);

		validWords = validateInput(word1, word2, dictionary);
		if (validWords) {
			findWordLadder(word1, word2, dictionary);
		}
	}
    
	cout << "Have a nice day." << endl;
    return 0;
}

/* Prints a short intro */
void printIntro() {
	cout << "Welcome to CS 106B Word Ladder." << endl;
	cout << "Please give me two English words, and I will change the" << endl;
	cout << "first into the second by changing one letter at a time." << endl;
	cout << endl;
}

/* Validates the two inputted words according to the spec. Returns true if the
 * two words are valid.
 */
bool validateInput(string &word1, string &word2, Lexicon &dict) {
	if (word1 == "" || word2 == "") {
	} else if (word1 == word2) {
		cout << "The two words must be different." << endl;
	} else if (word1.length() != word2.length()) {
		cout << "The two words must be the same length." << endl;
	} else if (!dict.contains(word1) || !dict.contains(word2)) {
		cout << "The two words must be found in the dictionary." << endl;
	} else {
		return true;
	}
	return false;
}

/* Converts all characters in a string to lower case.
 * Taken from:
 * http://stackoverflow.com/questions/2661766/c-convert-a-mixed-case-string-to-all-lower-case
*/
void lowerString(string &str) {
	for (int i = 0; str[i]; i++) {
		str[i] = tolower(str[i]);
	}
}

/* Finds words that are "neighbors" (one letter away) from the given word and
 * adds them to the given word vector.
 * Assumes an initialized lexicon containing valid words.
 */
void generateNeighbors(const string &word, Vector<string> &wordVect, Lexicon &dict) {
	string tmpWord = word;
	for (int i = 0; i < word.length(); i++) {
		char origLetter = tmpWord[i];
		for (int j = 0; j < 26; j++) {
			tmpWord[i] = 'a' + j;
			if (dict.contains(tmpWord) && tmpWord != word) {
				wordVect.add(tmpWord);
			}
		}
		tmpWord[i] = origLetter;
	}
}

/* Finds a word ladder between two valid words of the same length using the
 * words in the initialized Lexicon as intermediaries.
 * Implements the pesudo-code in the spec.
 */
void findWordLadder (const string &word1, const string &word2, Lexicon &dict) {
	Queue<Stack<string> > stackQueue;
	Stack<string> scratchStack;
	HashSet<string> usedWords;
	Stack<string>tmpStack;

	scratchStack.push(word1);
	stackQueue.enqueue(scratchStack);
	
	while (!stackQueue.isEmpty()) {
		scratchStack =  stackQueue.dequeue();
		Vector<string> neighbors;

		neighbors.clear();
		generateNeighbors(scratchStack.peek(), neighbors, dict);

		for (string s : neighbors) {
			if (!usedWords.contains(s)) {
				if (s == word2) {
					cout << "A ladder from " << word2 << " back to " << word1 << ":" << endl;
					cout << word2 << " ";

					while (!scratchStack.isEmpty()) {
						cout << scratchStack.pop() << " ";
					}

					cout << endl;
					return;
				} else {
					tmpStack = scratchStack;
					tmpStack.push(s);
					usedWords.add(s);
					stackQueue.enqueue(tmpStack);
				}
			}
		}
	}
	cout << "No word ladder found from " << word2 << " back to " << word1 << "." << endl;
}


