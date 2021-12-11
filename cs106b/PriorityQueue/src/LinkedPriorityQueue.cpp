/* LinkedPriorityList
 * ------------------
 * Implementation of a priority queue that uses an internal representation of a
 * linked list. The elements are kept sorted.
 * Has fast peeking, dequeuing, but slow enquing.
 * See ArrayPriorityQueue for a description of each member function.
 */

#include "LinkedPriorityQueue.h"

LinkedPriorityQueue::LinkedPriorityQueue() {
}

//O(N)
LinkedPriorityQueue::~LinkedPriorityQueue() {
	clear();
}

//O(N)
void LinkedPriorityQueue::changePriority(string value, int newPriority) {
	ListNode *curr = front;
	ListNode *prev = NULL;
	while (curr != NULL) {
		if (curr->value == value) {
			if (newPriority > curr->priority) {
				throw "Tried to lower priority";
			} else {
				//If prev is NULL, that means that we're at the front
				if (prev == NULL) {
					front = curr->next;
				} else {
					prev->next = curr->next;
				}
				delete curr;
				enqueue(value, newPriority);
				return;
			}
		}
		prev = curr;
		curr = curr->next;
	}
	throw "Element not found";
}

//O(N)
void LinkedPriorityQueue::clear() {
	ListNode *curr = front;
	while (curr != NULL) {
		ListNode *next = curr->next;
		delete curr;
		curr = next;
	}
	front = NULL;
}

//O(1)
string LinkedPriorityQueue::dequeue() {
	if (isEmpty()) {
		throw "Dequeued empty queue";
	}
	ListNode *old = front;
	string val = old->value;
	front = old->next;
	delete old;
	return val;
}

/* Returns the more urgent pointer of the two parameters
 */
//O(1)
ListNode *LinkedPriorityQueue::pickMoreUrgent(ListNode *elemA, ListNode *elemB) {
	if (elemA->priority > elemB->priority) {
		return elemB;
	} else if (elemB->priority > elemA->priority) {
		return elemA;
	} else {
		if (elemA->value > elemB->value) {
			return elemB;
		}
	}
	//Element A is more or equally urgent.
	return elemA;
}

//O(N)
void LinkedPriorityQueue::enqueue(string value, int priority) {
	ListNode *toAdd = new ListNode(value, priority, NULL);
	if (front == NULL) {
		front = toAdd;
		return;
	}

	ListNode *curr = front;
	ListNode *prev = NULL;

	while (curr != NULL && pickMoreUrgent(toAdd, curr) == curr) {
		prev = curr;
		curr = curr->next;
	}

	//Case where need to add to the front of the list
	if (prev == NULL) {
		toAdd->next = front;
		front = toAdd;
	} else {
		toAdd->next = prev->next;
		prev->next = toAdd;
	}

}

//O(1)
bool LinkedPriorityQueue::isEmpty() const {
    return front == NULL;
}

//O(1)
string LinkedPriorityQueue::peek() const {
	if (isEmpty()) {
		throw "Empty queue.";
	}
	return front->value;
}

//O(1)
int LinkedPriorityQueue::peekPriority() const {
	if (isEmpty()) {
		throw "Empty queue.";
	}
	return front->priority;
}

//O(N)
int LinkedPriorityQueue::size() const {
	int elems = 0;
	ListNode *curr = front;
	while (curr != NULL) {
		elems++;
		curr = curr->next;
	}
	return elems;
}

//O(N)
ostream& operator<<(ostream& out, const LinkedPriorityQueue& queue) {
	ListNode *curr = queue.front;
	out << "{";
	
	if (curr == NULL) {
		cout << "}";
		return out;
	}

	while (curr->next != NULL) {
		out << *curr << ", ";
		curr = curr->next;
	}

	out << *curr << "}";
    return out;
}
