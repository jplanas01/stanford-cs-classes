/*
 * CS 106B Recursion Problems
 * This client program contains a text menu for running your
 * assignment's various recursion problems.
 * You don't need to modify this file.
 * 
 * @author Marty Stepp
 * @version 2014/10/13
 * - modified for 14au version, including new GBufferedImage version of floodFill
 */

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include "console.h"
#include "filelib.h"
#include "gbufferedimage.h"
#include "gevents.h"
#include "ginteractors.h"
#include "gobjects.h"
#include "grid.h"
#include "gwindow.h"
#include "map.h"
#include "random.h"
#include "simpio.h"
#include "strlib.h"
#include "recursionproblems.h"
using namespace std;

// constants for Karel goes home
const int KAREL_GRID_SIZE = 10;
const int KAREL_FIELD_WIDTH = 7;

// constants for human pyramid
const int HUMAN_PYRAMID_MIN_WEIGHT = 50;
const int HUMAN_PYRAMID_MAX_WEIGHT = 150;
//#define HUMAN_PYRAMID_RANDOM_SEED 106   // uncomment to 'rig' random number generator

// constants for Sierpinski triangle fractal
const int SIERPINSKI_WINDOW_WIDTH = 700;
const int SIERPINSKI_WINDOW_HEIGHT = 450;
const int SIERPINSKI_FIGURE_SIZE = 400;
const int SIERPINSKI_FIGURE_Y = 20;
const string SIERPINSKI_WINDOW_TITLE = "CS 106B Sierpinski Triangle";

// constants for flood fill
const int FLOOD_FILL_WINDOW_WIDTH = 500;
const int FLOOD_FILL_WINDOW_HEIGHT = 400;
const int FLOOD_FILL_NUM_SHAPES = 100;
const int FLOOD_FILL_MIN_RECT_SIZE = 20;
const int FLOOD_FILL_MAX_RECT_SIZE = 100;
const string FLOOD_FILL_WINDOW_TITLE = "CS 106B Flood Fill";
//#define FLOOD_FILL_RANDOM_SEED 42   // uncomment to 'rig' random number generator

//// colors for flood fill
//#define COLOR_BLACK      0x000000
//#define COLOR_DARK_GRAY  0x595959
//#define COLOR_GRAY       0x999999
//#define COLOR_LIGHT_GRAY 0xBFBFBF
//#define COLOR_WHITE      0xFFFFFF
//#define COLOR_RED        0xFF0000
//#define COLOR_YELLOW     0xFFFF00
//#define COLOR_GREEN      0x00FF00
//#define COLOR_CYAN       0x00FFFF
//#define COLOR_BLUE       0x0000FF
//#define COLOR_MAGENTA    0xFF00FF
//#define COLOR_ORANGE     0xFFC800
//#define COLOR_PINK       0xFFAFAF

//// provided helpers
//int getPixelColor(int x, int y);
//void setPixelColor(int x, int y, int color);

// testing function prototype declarations
void test_karelGoesHome();
void test_convertStringToInteger();
void test_balancingParentheses();
void test_humanPyramid();
void test_sierpinskiTriangle();
void test_floodFill();
void test_grammarSolver();

int main() {
    cout << "CS 106B Recursion Problems" << endl;
    while (true) {
        cout << endl;
        cout << "Choose a problem:" << endl;
        cout << "1) Karel Goes Home" << endl;
        cout << "2) Convert String to Integer" << endl;
        cout << "3) Balancing Parentheses" << endl;
        cout << "4) Human Pyramid" << endl;
        cout << "5) Sierpinski Triangle" << endl;
        cout << "6) Flood Fill" << endl;
        cout << "7) Grammar Solver" << endl;
        int choice = getInteger("Enter your choice (or 0 to quit): ");
        cout << endl;

        //Why not a case statement?
		if (choice == 0)      { break; }
        else if (choice == 1) { test_karelGoesHome(); }
        else if (choice == 2) { test_convertStringToInteger(); }
        else if (choice == 3) { test_balancingParentheses(); }
        else if (choice == 4) { test_humanPyramid(); }
        else if (choice == 5) { test_sierpinskiTriangle(); }
        else if (choice == 6) { test_floodFill(); }
        else if (choice == 7) { test_grammarSolver(); }
    }

    cout << "Exiting." << endl;
    return 0;
}

/*
 * This function runs and tests your countKarelPaths function.
 * Prints a grid of various street/avenue locations and their numbers of paths.
 */
void test_karelGoesHome() {
    int street = getInteger("Street (row)? ");
    int avenue = getInteger("Avenue (col)? ");
    try {
        cout << "Karel's path count is: " << countKarelPaths(street, avenue) << endl;
    } catch (string) {
        cout << "Invalid parameters! (threw an exception)" << endl;
    } catch (const char*) {
        cout << "Invalid parameters! (threw an exception)" << endl;
    } catch (ErrorException) {
        cout << "Invalid parameters! (threw an exception)" << endl;
    }
    
    cout << endl;
    cout << "Karel's path count for several possible locations:" << endl;
    for (street = KAREL_GRID_SIZE; street > 0; street--) {
        for (avenue = 1; avenue <= KAREL_GRID_SIZE; avenue++) {
            int nPaths = countKarelPaths(street, avenue);
            cout << right << setw(KAREL_FIELD_WIDTH) << setfill(' ') << nPaths;
        }
        cout << endl;
    }
}

/*
 * This function runs and tests your convertStringToInteger function.
 * Prompts for a string and then reports its integer equivalent.
 */
void test_convertStringToInteger() {
    while (true) {
        cout << "Enter a string representing an integer (or Q to quit): ";
        string exp = toLowerCase(trim(getLine()));
        if (exp == "q") {
            break;
        }

        try {
            int converted = convertStringToInteger(exp);
            cout << "string \"" << exp << "\" converted into int "
                 << dec << converted << endl;
        } catch (string) {
            cout << "Invalid integer string! (threw an exception)" << endl;
        } catch (const char*) {
            cout << "Invalid integer string! (threw an exception)" << endl;
        } catch (ErrorException) {
            cout << "Invalid integer string! (threw an exception)" << endl;
        }
    }
}

/*
 * This function runs and tests your isBalanced function.
 * Prompts for a string and then reports whether it is balanced.
 */
void test_balancingParentheses() {
    while (true) {
        cout << "Enter a string (or Q to quit): ";
        string exp = toLowerCase(getLine());
        if (exp == "q") {
            break;
        }
        if (isBalanced(exp)) {
            cout << "Yes, that string is balanced!" << endl;
        } else {
            cout << "No, that string is not balanced." << endl;
        }
    }
}


/*
 * Runs and tests your humanPyramid function.
 */
void test_humanPyramid() {
    int cols = getInteger("How many people are on the bottom row? ");

    // possibly rig the random generator's output
#ifdef HUMAN_PYRAMID_RANDOM_SEED
    setRandomSeed(HUMAN_PYRAMID_RANDOM_SEED);
#endif // HUMAN_PYRAMID_RANDOM_SEED

    // populate vector of weights
    Vector<Vector<double> > weights;
    for (int row = 0; row < cols; row++) {
        Vector<double> currentRow;
        for (int col = 0; col <= row; col++) {
            double weight = randomReal(HUMAN_PYRAMID_MIN_WEIGHT, HUMAN_PYRAMID_MAX_WEIGHT);
            currentRow.add(weight);
        }
        weights.add(currentRow);
    }

    // print weights
    cout << "Each person's own weight:" << endl;
    cout << fixed << setprecision(2);
    for (int row = 0; row < weights.size(); row++) {
        for (int col = 0; col < weights[row].size(); col++) {
            cout << weights[row][col] << " ";
        }
        cout << endl;
    }
    cout << endl;

    // print weight on knees for each person in pyramid
    cout << "Weight on each person's knees:" << endl;
    for (int row = 0; row < weights.size(); row++) {
        for (int col = 0; col < weights[row].size(); col++) {
            double result = weightOnKnees(row, col, weights);
            cout << result << " ";
        }
        cout << endl;
    }
    cout << resetiosflags(ios::fixed | ios::floatfield);
}

/*
 * Runs and tests your drawSierpinskiTriangle function.
 * Prompts the user for the fractal's order, then displays it.
 */
void test_sierpinskiTriangle() {
    int order = getInteger("Fractal order? ");
    GWindow gw(SIERPINSKI_WINDOW_WIDTH, SIERPINSKI_WINDOW_HEIGHT);
    gw.setWindowTitle(SIERPINSKI_WINDOW_TITLE);
    int x0 = (SIERPINSKI_WINDOW_WIDTH - SIERPINSKI_FIGURE_SIZE) / 2;  // center
    int y0 = SIERPINSKI_FIGURE_Y;

    try {
        drawSierpinskiTriangle(gw, x0, y0, SIERPINSKI_FIGURE_SIZE, order);
    } catch (string) {
        cout << "Invalid parameters passed! (threw an exception)" << endl;
        gw.close();
    } catch (const char*) {
        cout << "Invalid parameters passed! (threw an exception)" << endl;
        gw.close();
    } catch (ErrorException) {
        cout << "Invalid parameters passed! (threw an exception)" << endl;
        gw.close();
    }
}

/*
 * Runs and tests your floodFill function.
 */
void test_floodFill() {
    GObject::setAntiAliasing(false);
    GWindow floodFillWindow(FLOOD_FILL_WINDOW_WIDTH, FLOOD_FILL_WINDOW_HEIGHT);
    floodFillWindow.setWindowTitle(FLOOD_FILL_WINDOW_TITLE);

    Map<string, int> colorMap;
    colorMap["Red"]    = 0x8c1515;   // Stanford red
    colorMap["Yellow"] = 0xeeee00;   // yellow
    colorMap["Blue"]   = 0x0000cc;   // blue
    colorMap["Green"]  = 0x00cc00;   // green
    colorMap["Purple"] = 0xcc00cc;   // purple
    colorMap["Orange"] = 0xff8800;   // orange
    Vector<string> colorVector = colorMap.keys();

    GLabel fillLabel("Fill color:");
    /*
	GChooser colorList;
    for (string key : colorMap) {
        colorList.addItem(key);
    }
	*/
	GTextField colorList;
    floodFillWindow.addToRegion(&fillLabel, "SOUTH");
    floodFillWindow.addToRegion(&colorList, "SOUTH");

    // use buffered image to store individual pixels
    GBufferedImage floodFillPixels(
                /* x */ 0,
                /* y */ 0,
                /* width */ FLOOD_FILL_WINDOW_WIDTH,
                /* height */ FLOOD_FILL_WINDOW_HEIGHT,
                /* rgb fill */ 0xffffff);
    
    // draw several random shapes
#ifdef FLOOD_FILL_RANDOM_SEED
    setRandomSeed(FLOOD_FILL_RANDOM_SEED);
#endif // FLOOD_FILL_RANDOM_SEED

    for (int i = 0; i < FLOOD_FILL_NUM_SHAPES; i++) {
        double w = randomInteger(FLOOD_FILL_MIN_RECT_SIZE, FLOOD_FILL_MAX_RECT_SIZE);
        double h = randomInteger(FLOOD_FILL_MIN_RECT_SIZE, FLOOD_FILL_MAX_RECT_SIZE);
        double x = randomInteger(0, FLOOD_FILL_WINDOW_WIDTH  - w);
        double y = randomInteger(0, FLOOD_FILL_WINDOW_HEIGHT - h - colorList.getHeight());
        int color = colorMap[colorVector[randomInteger(0, colorVector.size() - 1)]];
        floodFillPixels.fillRegion(x, y, w, h, color);
    }
    floodFillWindow.add(&floodFillPixels);

    // main event loop to process events as they happen
    while (true) {
        GEvent e = waitForEvent(MOUSE_EVENT | WINDOW_EVENT);
        if (e.getEventClass() == MOUSE_EVENT) {
            if (e.getEventType() != MOUSE_CLICKED) { continue; }
            colorList.setEnabled(false);
            GMouseEvent mouseEvent(e);

			/* Taken from:
			 * http://stackoverflow.com/questions/1070497/c-convert-hex-string-to-signed-integer
			 */
			stringstream ss;
			ss << hex << colorList.getText();
            int color;
			ss >> color;

            int mx = (int) mouseEvent.getX();
            int my = (int) mouseEvent.getY();
            cout << "Flood fill (x=" << dec << mx << ", y=" << my << "),"
                 << " color " << hex << setw(6) << setfill('0') << color
                 << dec << ": ";
            cout.flush();
            int pixelsColored = floodFill(floodFillPixels, mx, my, color);
            cout << pixelsColored << " pixels colored." << endl;
            colorList.setEnabled(true);
        } else if (e.getEventClass() == WINDOW_EVENT) {
            if (e.getEventType() == WINDOW_CLOSED) {
                // make sure that it was the flood fill window that got closed
                if (!floodFillWindow.isOpen() || !floodFillWindow.isVisible()) {
                    break;
                }
            }
        }
    }
    cout << resetiosflags(ios::fixed | ios::floatfield);
    
    cout << endl;
    cout << "Going to test flood fill exception handling." << endl;
    try {
        floodFill(floodFillPixels, -1, -2, 0x0);
        cout << "Fill (-1, -2) did not throw a proper exception." << endl;
    } catch (string) {
        cout << "Fill (-1, -2) threw an exception!" << endl;
    } catch (const char*) {
        cout << "Fill (-1, -2) threw an exception!" << endl;
    } catch (ErrorException) {
        cout << "Fill (-1, -2) threw an exception!" << endl;
    }
    try {
        floodFill(floodFillPixels, 3210, 4567, 0x0);
        cout << "Fill (3210, 4567) did not throw a proper exception." << endl;
    } catch (string) {
        cout << "Fill (3210, 4567) threw an exception!" << endl;
    } catch (const char*) {
        cout << "Fill (3210, 4567) threw an exception!" << endl;
    } catch (ErrorException) {
        cout << "Fill (3210, 4567) threw an exception!" << endl;
    }
}

/*
 * Runs and tests your grammarGenerate function.
 * Prompts for the input file and symbol, then calls your function.
 */
void test_grammarSolver() {
    ifstream input;
    promptUserForFile(input, "Grammar file name? ");
    while (true) {
        string symbol = trim(getLine("Symbol to generate (Enter to quit)? "));
        if (symbol.empty()) {
            break;
        }

        // call student's generate function and print the vector of results
        int times = getInteger("How many to generate? ");
        rewindStream(input);
        cout << endl;
        Vector<string> result = grammarGenerate(input, symbol, times);
        for (int i = 0; i < result.size(); i++) {
            cout << setw(2) << (i+1) << ": " << result[i] << endl;
        }
        cout << endl;
    }
}
