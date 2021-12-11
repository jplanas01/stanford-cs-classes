/* N-grams
 * -------
 * By Juan Planas (TA: Eric Yu)
 * Creates a map of N-grams from a body of text and generates random text
 * that sounds like the original (a Markov chain).
 */


#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <random.h>
#include "console.h"
#include "filelib.h"
#include "map.h"
#include "simpio.h"
#include "queue.h"

using namespace std;


void printIntro();
string vecToStr(Vector<string> &vec);
void addWord(Vector<string> &windowVec, Map<string, Vector<string> > &phraseMap,
		const string &word);
void buildPhraseMap(ifstream &input, int n, Map<string, Vector<string> > &phraseMap);
string pickRandomElem(const Vector<string> &vec);
void slideWindow(string &window, string &word);
void genRandomText(Map<string, Vector<string> > &phraseMap, int numWords, int N);

int main() {
	printIntro();

	ifstream inputFile;
	promptUserForFile(inputFile, "Input file name? ");

	int N = 0;
	while (N < 2) {
		N = getInteger("Value of N? ");
		if (N < 2) {
			cout << "N needs to be greater than 2." << endl;;
		}
	}

	Map<string, Vector<string> > phraseMap;
	buildPhraseMap(inputFile, N, phraseMap);

	cout << endl;
	
	int numWords = 99;
	while (numWords > 0) {
		numWords = getInteger("# of random words to generate (0 to quit)? ");
		if (numWords < N && numWords != 0) {
			cout << "Must be at least " << N << " words." << endl;
			continue;
		} else if (numWords == 0) {
			continue;
		}
		genRandomText(phraseMap, numWords, N);
		cout << endl;
	}
	cout << "Exiting." << endl;

	return 0;
}

/* Prints an introductory message. */
void printIntro() {
	cout << "Welcome to CS 106B Random Writer ('N-Grams')." << endl;
	cout << "This program makes random text based on a document." << endl;
	cout << "Give me an input file and an 'N' value for groups" << endl;
	cout << "of words, and I'll create random text for you." << endl;
	cout << endl;
}

/* Joins the words in a string vector into a space-separated string, and returns that.
 */
string vecToStr(Vector<string> &vec) {
	string tmp = "";
	int size = vec.size();
	for (int i = 0; i < size - 1; i++) {
		tmp += (vec[i] + " ");
	}
	tmp += vec[size-1];
	return tmp;
}

void addWord(Vector<string> &windowVec, Map<string, Vector<string> > &phraseMap,
		const string &word) {
	string windowStr = vecToStr(windowVec);

	Vector<string> tmpVec = phraseMap.get(windowStr);
	tmpVec.add(word);
	phraseMap.put(windowStr, tmpVec);

	windowVec.remove(0);
	windowVec.add(word);
	
}

/* Reads the words in the given source file and creates a mapping of phrases to
 * words as defined in the spec to create the N-grams. Assumes phraseMap is
 * empty and a properly initialized ifstream.
 * Code from http://stackoverflow.com/questions/20372661/read-word-by-word-from-file-in-c
 * used.
 */
void buildPhraseMap(ifstream &input, int n, Map<string, Vector<string> > &phraseMap) {
	string word;
	Vector<string> windowVec;

	for (int i = 0; i < n-1; i++) {
		input >> word;
		windowVec.add(word);
	}


	while (input >> word) {
		addWord(windowVec, phraseMap, word);
	}

	//Necessary for wrap around
	//next 2 lines taken from http://cboard.cprogramming.com/cplusplus-programming/134024-so-how-do-i-get-ifstream-start-top-file-again.html
	input.clear();
	input.seekg(0, input.beg);

	for (int i = 0; i < n - 1; i++) {
		input >> word;
		addWord(windowVec, phraseMap, word);
	}

}

/* Picks a random element from a string vector and returns it. */
string pickRandomElem(const Vector<string> &vec) {
	int i = randomInteger(0,vec.size() - 1);
	return vec[i];
}

/* Given a space-separated string of words and an additional word, drop the
 * first word from the string and append the given word to the end. Assumes
 * that "window" has at least 2 words.
 * Eg, "abc def ghi" and "jkl" would result in "def ghi jkl".
 */
void slideWindow(string &window, string &word) {
	vector<string> tmp = stringSplit(window, " ");
	vector<string> joined(&tmp[1], &tmp[tmp.size()]);
	window = stringJoin(joined, " ") + " " + word;
}

/* Given a properly initialized phrase map (generated by the funciton above),
 * print out required amount of words following the spec (ie, for a phrase
 * print out a random following word, slide the phrase window, and repeat.)
 */
void genRandomText(Map<string, Vector<string> > &phraseMap, int numWords, int N) {
	string window = pickRandomElem(phraseMap.keys());
	cout << "... " << window;

	for (int i = 0; i < numWords - N + 1; i++) {
		string word = pickRandomElem(phraseMap.get(window));
		cout << " " << word;
		cout.flush();
		slideWindow(window, word);
	}

	cout << " ..." << endl;
}
