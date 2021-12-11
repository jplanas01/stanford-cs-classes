/* recursionproblems.cpp
 * ---------------------
 * By Juan Planas (TA: Eric Yu)
 * Solves a number of problems using recursion
 */

#include "recursionproblems.h"
#include <cmath>
#include <iostream>
#include "hashmap.h"
#include "map.h"
#include "random.h"
using namespace std;

//Function prototypes
int countPaths(int street, int avenue, int pathCount);
int countKarelPaths(int street, int avenue);
int convertStringHelp(string exp, int total);
int convertStringToInteger(string exp);
size_t findMatch(const string &exp, string match);
bool isBalanced(string exp);
double weightOnKnees(int row, int col, Vector<Vector<double> >& weights);
void drawDownTriangle(GWindow &gw, double x, double y, int length);
void drawUpTriangle(GWindow &gw, double x, double y, int length);
void recurTriangle(GWindow& gw, double x, double y, int size, int order);
void drawSierpinskiTriangle(GWindow& gw, double x, double y, int size, int order);
int floodFillHelp(GBufferedImage &image, int x, int y, int newColor, int oldColor, int count);
int floodFill(GBufferedImage& image, int x, int y, int color);
void buildGrammarMap(istream &input, Map<string, Vector<string> > &map);
Vector<string> grammarGenerate(istream& input, string symbol, int times);
string genRandomSentence(Map<string, Vector<string> > &map, string symbol);


/* Counts the number of paths from the given position to (1,1) recursively by
 * exploring paths south and west of the current position.
 */
int countPaths(int street, int avenue, int pathCount) {
	if (street == 1 || avenue == 1) {
		return 1;
	} else if (street < 1 || avenue < 1) {
		return -1;
	}

	int c1 = countPaths(street - 1, avenue, pathCount + 1);
	int c2 = countPaths(street, avenue - 1, pathCount + 1);

	return c1 + c2; 
}

/* Validates input parameters and calls the recursive function that does the
 * heavy lifting.
 */
int countKarelPaths(int street, int avenue) {
	if (street < 1 || avenue < 1) {
		throw "illegal street or avenue!";
	}
	
	return countPaths(street, avenue, 0);
}

/* Helper function for converting a string to an integer. Goes from the
 * leftmost digit to the rightmost digit, multiplies the previous result by 10
 * and adds the current digit.
 */
int convertStringHelp(string exp, int total) {
	
	if (exp == "") {
		return total;
	}

	int ones;
	if (isdigit(exp[0])) {
	   ones = exp[0] - '0';
	} else {
		throw "error, found non-number in string";
	}

	string newExp = exp.substr(1, exp.size() - 1);

	return convertStringHelp(newExp, total * 10 + ones);
}

/* Checks for negative numbers and calls convertStringHelp to get the integer
 * representation of the string.
 */
int convertStringToInteger(string exp) {
	if (exp[0] == '-') {
		return -convertStringHelp(exp.substr(1, exp.size() - 1), 0);
	} else {
		return convertStringHelp(exp, 0);
	}
}

/* Returns the location of the first matching pair of opening/closing brackets
 * specified in match. Assumes that match is a string of corresponding brackets
 * (eg, "(){}[]<>"). If the first pair of brackets isn't found, it tries
 * to find the next pair until the search is exhausted.
 * If no match is found, -1 is returned.
 */
size_t findMatch(const string &exp, string match) {
	if (match == "") {
		return -1;
	}

	//Find first pair of brackets in given expression. If they were not found,
	//remove those two from the string and try again with the remaining ones.
	size_t loc = exp.find(match.substr(0,2));
	if (loc == -1) {
		return findMatch(exp, match.substr(2, match.length() - 2));
	}
	return loc;
}

/* Recursively checks if a string has balanced brackets. The first pair of
 * matching brackets is removed and the resulting string is checked again. If
 * the search is exhausted with no characters remaining, the string is balanced
 * and true is returned.
 */
bool isBalanced(string exp) {
	if (exp == "") {
		return true;
	}

	const string to_match = "()[]{}<>";
	size_t loc = findMatch(exp, to_match);

	if (loc != -1) {
		string newExp = exp;
		newExp.erase(loc, 2);
		return isBalanced(newExp);
	}
	return false;
}

/* Calculates the weight on the given person's knees by calculating the weights
 * of the people above him plus his own.
 * Assumes a properly formatted weights vector as defined in the spec.
 */
double weightOnKnees(int row, int col, Vector<Vector<double> >& weights) {
	if (row > weights.size() - 1 || row < 0) {
		return 0.0;
	} else if (col > weights[row].size() - 1 || col < 0) {
		return 0.0;
	}

	double weightA = weightOnKnees(row - 1, col, weights) / 2;
	double weightB = weightOnKnees(row - 1, col - 1, weights) / 2;

	return weightA + weightB + weights[row][col];
}

/* Draws a downward pointing triangle bounded by a square of specified length
 * and origin at (x, y). 
 * Assumes that x and y are valid and that the square is in the bounds of
 * GWindow
 */
void drawDownTriangle(GWindow &gw, double x, double y, int length) {
	double height = length / 2.0 * sqrt(3.0);
	gw.drawLine(x, y, x + length, y);
	gw.drawLine(x, y, x + length / 2.0, y + height);
	gw.drawLine(x + length, y, x + length / 2.0, y + height);
}

/* Same as above, except with an upwards pointing triangle.
 */
void drawUpTriangle(GWindow &gw, double x, double y, int length) {
	double height = length / 4.0 * sqrt(3.0);
	gw.drawLine(x + length / 2.0, y + 2.0 * height, x + length * 3.0 / 2.0, y + 2.0 * height);
	gw.drawLine(x + length / 2.0, y + 2.0 * height, x + length, y);
	gw.drawLine(x + length, y, x + length * 3.0 / 2.0, y + 2.0 * height);
}

/* Recursively draw a Sierpinski triangles of the given order by drawing an
 * upwards pointing triangle and recursing into each of the 3 created downward
 * pointing triangles.
 * Assumes a proper GWindow and sane parameters and that the given triangle
 * will fit in the window.
 */
void recurTriangle(GWindow& gw, double x, double y, int size, int order) {
	if (order == 0) {
		return;
	}

	double height = size * sqrt(3.0) / 2;
	drawUpTriangle(gw, x, y, size);

	//Recur into the 3 created triangles. Top left corner of each is passed as
	//new x and y.
	recurTriangle(gw, x, y, size / 2, order - 1);	
	recurTriangle(gw, x + size, y, size / 2, order - 1);
	recurTriangle(gw, x + size / 2, y + height, size / 2, order - 1);
}

/* Draws the initial outer downward-pointing triangle and calls recurTriangle
 * to fill in the remaining triangles.
 * Assumes initialized GWindow and that it is big enough to hold the resulting
 * triangles.
 */
void drawSierpinskiTriangle(GWindow& gw, double x, double y, int size, int order) {
	if (x < 0 || y < 0 || order < 0 || size < 0) {
		throw "error, one or more parameters invalid.";
	} else if (order == 0) {
		return;
	}
	drawDownTriangle(gw, x, y, size);
	recurTriangle(gw, x, y, size / 2, order - 1);
}

/* Changes the pixel color of a polygon by exploring recursively the 4 pixels
 * adjacent to the one given. Implements the optimization given in the spec.
 * Assumes an initialized GBufferedImage
 */
int floodFillHelp(GBufferedImage &image, int x, int y, int newColor, int oldColor, int count) {
	//Base case and optimizations
	if (!image.inBounds(x, y) || image.getRGB(x, y) == newColor ||
			image.getRGB(x, y) != oldColor) {
		return 0;
	}

	image.setRGB(x, y, newColor);

	int up, down, left, right;
	up = floodFillHelp(image, x, y + 1, newColor, oldColor, count);
	down = floodFillHelp(image, x, y - 1, newColor, oldColor, count);
	left = floodFillHelp(image, x - 1, y, newColor, oldColor, count);
	right = floodFillHelp(image, x + 1, y, newColor, oldColor, count);
	return up + down + left + right + 1;
}

/* Verifies the given coordinates and calls floodFillHelp to do the heavy
 * lifting of changing the clicked polygon's color.
 * Assumes and initialized GBufferedImage.
 */
int floodFill(GBufferedImage& image, int x, int y, int color) {
	if (!image.inBounds(x, y)) {
		throw "Error, x and y given are outside bounds of image.";
	}
	return floodFillHelp(image, x, y, color, image.getRGB(x, y), 0);
}

/* Generates a map of left sides to right sides in an NBF grammar
 * specification. Assumes an initialized istream and Map.
 */
void buildGrammarMap(istream &input, Map<string, Vector<string> > &map) {
	string line;
	while (getline(input, line)) {
		// Set left of ::= as key, everything separated by | as value.
		Vector<string> lineParts = stringSplit(line, "::=");
		Vector<string> terminals = stringSplit(lineParts[1], "|");

		if (map.containsKey(lineParts[0])) {
			throw "Error, grammar file contains duplicated symbols.";
		} else {
			map.put(trim(lineParts[0]), terminals);
		}
	}
}

/* Recursively generates a random phrase given a symbol.
 * Assumes a Map initialized by buildGrammarMap.
 */
string genRandomSentence(Map<string, Vector<string> > &map, string symbol) {
	//If the symbol isn't a key in the map, it's a terminal symbol.
	if (!map.containsKey(symbol)) {
		return symbol;
	}
	
	//Get possible expansions for current symbol, pick one.
	Vector<string>values = map.get(symbol);
	string chosen = values[randomInteger(0, values.size() - 1)];
	string result = "";

	//If there are multiple parts to a phrase, generate random phrases for
	//non-terminal ones and join them together.
	for (string i : stringSplit(chosen, " ")) {
		result += genRandomSentence(map, i);
		result += " ";
	}
	return trim(result);
}

/* Returns a vector of sentences generated according to the given NBF grammar
 * in input for the given symbol.
 * Assumes an initialized istream.
 */
Vector<string> grammarGenerate(istream& input, string symbol, int times) {
	if (symbol == "") {
		throw "Error, empty symbol.";
	}

    Map<string, Vector<string> > grammar;
	buildGrammarMap(input, grammar);
	
	Vector<string> v;
	while (times) {
	   	string sentence = genRandomSentence(grammar, symbol);
		v.add(sentence);
		times--;
	}
    return v;
}
