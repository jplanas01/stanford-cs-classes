/* ArrayPriorityQueue
 * ------------------
 * Class and function definitions for the array priority queue.
 */

#ifndef _arraypriorityqueue_h
#define _arraypriorityqueue_h

#include <iostream>
#include <string>
#include "PQEntry.h"
using namespace std;

// TODO: comment
class ArrayPriorityQueue {
public:
    ArrayPriorityQueue();
    ~ArrayPriorityQueue();
    void changePriority(string value, int newPriority);
    void clear();
    string dequeue();
    void enqueue(string value, int priority);
    bool isEmpty() const;
    string peek() const;
    int peekPriority() const;
    int size() const;
    friend ostream& operator <<(ostream& out, const ArrayPriorityQueue& queue);

private:
	void growElements();
	int findMostUrgentIndex() const;
	int numElems;
	int capacity;
	PQEntry *elements;	
};

#endif
