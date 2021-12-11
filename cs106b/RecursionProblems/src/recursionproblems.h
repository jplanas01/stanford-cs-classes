/*
 * CS 106B Recursion Problems
 * This file declares necessary function prototypes, so that other files
 * are able to call the recursive functions you will write.
 * You should not modify this file.
 * 
 * (If you want to declare your own helper function prototypes,
 *  do that near the top of your .cpp file, not here.)
 */

#ifndef _recursionproblems_h
#define _recursionproblems_h

#include <iostream>
#include <string>
#include "gbufferedimage.h"
#include "gwindow.h"
#include "vector.h"

// function prototype declarations
// (If you want to declare your own helper function prototypes, do that near
//  the top of your .cpp file, not here.  You should NOT modify this file.)
int countKarelPaths(int street, int avenue);
int convertStringToInteger(std::string exp);
bool isBalanced(std::string exp);
double weightOnKnees(int row, int col, Vector<Vector<double> >& weights);
void drawSierpinskiTriangle(GWindow& gw, double x, double y, int size, int order);
int floodFill(GBufferedImage& image, int x, int y, int color);
Vector<std::string> grammarGenerate(std::istream& input, std::string symbol, int times);

#endif
