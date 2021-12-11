/* HeapPriorityQueue
 * -----------------
 * Class and function definitions for the heap priority queue.
 */

#ifndef _heappriorityqueue_h
#define _heappriorityqueue_h

#include <iostream>
#include <string>
#include "PQEntry.h"
using namespace std;

class HeapPriorityQueue {
public:
    HeapPriorityQueue();
    ~HeapPriorityQueue();
    void changePriority(string value, int newPriority);
    void clear();
    string dequeue();
    void enqueue(string value, int priority);
    bool isEmpty() const;
    string peek() const;
    int peekPriority() const;
    int size() const;
    friend ostream& operator <<(ostream& out, const HeapPriorityQueue& queue);

private:
	void resizeStorage();
	void swapElements(PQEntry *first, PQEntry *second);
	PQEntry *findMoreUrgentElem(PQEntry *elemA, PQEntry *elemB);
	void percolateUp(int start);	
	int capacity;
	int numElems;
	PQEntry *elements;
};

#endif
