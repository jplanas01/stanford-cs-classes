/* HeapPriorityQueue
 * -----------------
 * Implements a priority queue using a binary heap. Has the advantage that the
 * most urgent element is at the front of the internal array, and has decent
 * enqueuing and dequeing performance.
 * See ArrayPriorityQueue for description of the member functions.
 */


#include "HeapPriorityQueue.h"
#include <climits>

//O(1)
HeapPriorityQueue::HeapPriorityQueue() {
	elements = new PQEntry[10];
	capacity = 10;
	numElems = 0;
}

//O(1)
HeapPriorityQueue::~HeapPriorityQueue() {
	delete [] elements;
}

//O(log N)
void HeapPriorityQueue::changePriority(string value, int newPriority) {
	for (int i = 1; i <= numElems; i++) {
		if (elements[i].value == value) {
			if (elements[i].priority < newPriority) {
				throw "Tried to make element less urgent.";
			}

			elements[i].priority = newPriority;
			percolateUp(i);
			
			return;
		}
	}
	throw "Value not found in queue.";
}

//O(N)
void HeapPriorityQueue::clear() {
	//Make sure elements at edges of used elements never get percolated
	for (; numElems > 0; numElems--) {
		elements[numElems].priority = INT_MAX;
	}
}

/* Returns the more urgent of the two parameters. */
//O(1)
PQEntry *HeapPriorityQueue::findMoreUrgentElem(PQEntry *elemA, PQEntry *elemB) {
	if (elemA->priority > elemB->priority) {
		return elemB;
	} else if (elemB->priority > elemA->priority) {
		return elemA;
	} else {
		//Both have same priority
		if (elemA->value > elemB->value) {
			return elemB;
		}
	}
	//If execution reaches here, indexA has more priority or they're
	//duplicates. In second case, doesn't matter which one is returned.
	return elemA;
}

//O(log N)
string HeapPriorityQueue::dequeue() {
	if (numElems == 0) {
		throw "Attempted to dequeue empty queue";
	}

	string val = elements[1].value;

	//Prevent removed values from getting percolated upwards
	elements[1].priority = INT_MAX;

	swapElements(&elements[1], &elements[numElems]);
	numElems--;


	int curIndex = 1;
	PQEntry *child1 = &elements[curIndex * 2];
	PQEntry *child2 = &elements[curIndex * 2 + 1];
	PQEntry *current = &elements[curIndex];

	//Start percolating the value downwards
	while (curIndex <= numElems / 2) {
		PQEntry *urgent = findMoreUrgentElem(child1, child2);
		PQEntry *other = (urgent == child1) ? child2 : child1;

		if (findMoreUrgentElem(current, urgent) == current) {
			//Current one is more urgent than more urgent child, nothing to do.
			break;
		} else {
			if (findMoreUrgentElem(current, urgent) == urgent) {
				//Case where most urgent child is more urgent
				swapElements(current, urgent);
				if (urgent == child1) {
					curIndex *= 2;
				} else {
					curIndex = curIndex * 2 + 1;
				}
			} else {
				//Case where less urgent child is more urgent
				swapElements(current, other);
				if (other == child1) {
					curIndex *= 2;
				} else {
					curIndex = curIndex * 2 + 1;
				}
			}
		}

		child1 = &elements[curIndex * 2];
		child2 = &elements[curIndex * 2 + 1];
		current = &elements[curIndex];
	}
	

	return val;	
}

//O(N)
void HeapPriorityQueue::resizeStorage() {
	PQEntry *temp = new PQEntry[2 * capacity];
	for (int i = 0; i < capacity; i++) {
		temp[i] = elements[i];
	}

	delete [] elements;
	elements = temp;
	capacity *= 2;
}

/* Swaps two elements in the internal array representation.
 */
//O(1)
void HeapPriorityQueue::swapElements(PQEntry *first, PQEntry *second) {
	PQEntry temp;
	temp.value = first->value;
	temp.priority = first->priority;

	first->priority = second->priority;
	first->value = second->value;
	second->value = temp.value;
	second->priority = temp.priority;
}

/* Implements the bubbling up used when changing a priority or enqueing.
 * Assumes the internal array isn't empty and that start refers to a valid
 * index.
 */
//O(log N)
void HeapPriorityQueue::percolateUp(int start) {
	PQEntry *current = &elements[start];
	PQEntry *next = &elements[start / 2];

	//Loop while the current element is the urgent one and prevent infinite
	//swapping of the most urgent element with itself
	while (findMoreUrgentElem(next, current) == current && start != 1) { 
		swapElements(current, next);
		start /= 2;
		current = &elements[start];
		next = &elements[start / 2];
	}
}

//O(log N)
void HeapPriorityQueue::enqueue(string value, int priority) {
	if (numElems + 1 == capacity) {
		resizeStorage();
	}

	int curIndex = numElems + 1;
	elements[curIndex].priority = priority;
	elements[curIndex].value = value;

	percolateUp(curIndex);

	numElems++;
}

//O(1)
bool HeapPriorityQueue::isEmpty() const {
	return numElems == 0;
}

//O(1)
string HeapPriorityQueue::peek() const {
	if (isEmpty()) {
		throw "Empty queue.";
	}
	return elements[1].value;
}

//O(1)
int HeapPriorityQueue::peekPriority() const {
	if (isEmpty()) {
		throw "Empty queue.";
	}
	return elements[1].priority;
}

//O(1)
int HeapPriorityQueue::size() const {
	return numElems;
}

//O(N)
ostream& operator<<(ostream& out, const HeapPriorityQueue& queue) {
	out << "{";
	for (int i = 1; i < queue.numElems; i++) {
		out << queue.elements[i] << ", ";
	}
	if (queue.numElems > 0) {
		out << queue.elements[queue.numElems] << "}";
	}

    return out;
}
