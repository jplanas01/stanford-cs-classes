/* ExtraPriorityQueue
 * ------------------
 * Class and function definitions for the Map/Queue priority queue.
 */

#ifndef _extrapriorityqueue_h
#define _extrapriorityqueue_h

#include <iostream>
#include <string>
#include "PQEntry.h"
#include "map.h"
#include "queue.h"
using namespace std;

class ExtraPriorityQueue {
public:
    ExtraPriorityQueue();
    ~ExtraPriorityQueue();
    void changePriority(string value, int newPriority);
    void clear();
    string dequeue();
    void enqueue(string value, int priority);
    bool isEmpty() const;
    string peek() const;
    int peekPriority() const;
    int size() const;
    friend ostream& operator <<(ostream& out, const ExtraPriorityQueue& queue);

private:
	Map<int, Queue<string> > elements;
	int numElems;
};

#endif
