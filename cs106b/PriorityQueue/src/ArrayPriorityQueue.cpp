/* ArrayPriorityQueue
 * ------------------
 * By Juan Planas (TA: Eric Yu)
 * A priority queue implemented with an unsorted array. Has fast insertion, but
 * slow peeking and removal, and is easy to implement.
 */

/* Portions of code below taken from lecture slide on ArrayList
 */
#include "ArrayPriorityQueue.h"
#include <climits>

/* Constructor for the list */
//O(1)
ArrayPriorityQueue::ArrayPriorityQueue() {
	numElems = 0;
	capacity = 10;
	elements = new PQEntry[10];
}

/* Destructor for the list, prevents memory leaks */
//O(1)
ArrayPriorityQueue::~ArrayPriorityQueue() {
	delete [] elements;
}

/* Changes the priority of a given value that exists in the queue. An exception
 * is thrown if the value is not in the list or if the new priority is larger
 * than the existing priority. Only the first matching value found is changed.
 */
//O(N)
void ArrayPriorityQueue::changePriority(string value, int newPriority) {
	for (int i = 0; i < numElems; i++) {
		if (elements[i].value == value) {
			if (newPriority > elements[i].priority) {
				throw "Tried to make element less urgent";
			}
			elements[i].priority = newPriority;
			return;
		}
	}
	throw "Element with desired value not found";
}

/* Clears the priority queue */
void ArrayPriorityQueue::clear() {
	numElems = 0;
}

/* Finds the index of the most urgent element in the queue. Assumes a non-empty
 * queue.
 */
//O(N)
int ArrayPriorityQueue::findMostUrgentIndex() const {
	int mostUrgent = INT_MAX;
	int urgentElem = -1;
	for (int i = 0; i < numElems; i++) {
		if (elements[i].priority < mostUrgent) {
			urgentElem = i;
			mostUrgent = elements[i].priority;
		}
	}
	return urgentElem;
}

/* Dequeues the most urgent element in the queue and returns its value. An
 * exception is thrown if the queue is empty.
 */
//O(N)
string ArrayPriorityQueue::dequeue() {
	if (isEmpty()) {
		throw "Attempted to dequeue with no elements";
	}

	int urgentElem = findMostUrgentIndex();
	string val = elements[urgentElem].value;

	for (int i = urgentElem; i < numElems; i++) {
		elements[i] = elements[i + 1];
	}

	numElems--;
	return val;
}

/* Adds a value with the specified priority. Duplicates are allowed, any value
 * is legal, and any integer is valid. Lower numbers are more urgent.
 */
//O(1)
void ArrayPriorityQueue::enqueue(string value, int priority) {
	if (numElems == capacity) {
		growElements();
	}

	elements[numElems].value = value;
	elements[numElems].priority = priority;
	numElems++;
}

/* Grows the array used to hold the values and their priorities.
 */
//O(N)
void ArrayPriorityQueue::growElements() {
	PQEntry *temp = new PQEntry[2 * capacity];
	for (int i = 0; i < capacity; i++) {
		temp[i] = elements[i];
	}

	delete [] elements;
	elements = temp;
	capacity *= 2;
}

/* Returns true if the number of elements in the queue is 0 */
//O(1)
bool ArrayPriorityQueue::isEmpty() const {
    return numElems == 0;
}

/* Returns the value of the most urgent element in the queue without altering
 * the queue. An exception is thrown if the queue is empty.
 */
//O(N)
string ArrayPriorityQueue::peek() const {
	if (isEmpty()) {
		throw "Empty queue.";
	}
	return elements[findMostUrgentIndex()].value;
}

/* Returns the value of the most urgent priority without changing the queue. A
 * string exception is thrown if the queue is empty.
 */
//O(N)
int ArrayPriorityQueue::peekPriority() const {
	if (isEmpty()) {
		throw "Empty queue.";
	}
	return elements[findMostUrgentIndex()].priority;
}

/* Returns the number of elements in the queue.
 */
//O(1)
int ArrayPriorityQueue::size() const {
	return numElems;
}

/* Prints out the contents of the queue without changing the queue. */
//O(N)
ostream& operator<<(ostream& out, const ArrayPriorityQueue& queue) {
	out << "{";
	for (int i = 0; i < queue.numElems - 1; i++) {
		out << queue.elements[i] << ", ";
	}
	out << queue.elements[queue.numElems-1] << "}";

    return out;
}
