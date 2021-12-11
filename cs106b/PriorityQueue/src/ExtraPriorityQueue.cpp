/* ExtraPriorityQueue
 * ------------------
 * Implementation of a Map and Queue priority queue, where the priority is the
 * key and the values are the Queues which hold the string values of the
 * elements.
 * Big-O runtimes of changePriority and << are my best guess.
 */

#include "ExtraPriorityQueue.h"

//O(1)
ExtraPriorityQueue::ExtraPriorityQueue() {
	numElems = 0;
}

//O(1)
ExtraPriorityQueue::~ExtraPriorityQueue() {
}

//O(M N log N)
void ExtraPriorityQueue::changePriority(string value, int newPriority) {
	Vector<int> allValues = elements.keys();
	for (int i = 0; i < allValues.size(); i++) {
		Queue<string> temp1 = elements.get(allValues[i]);
		Queue<string> temp2;
		while (!temp1.isEmpty()) {
			string val = temp1.dequeue();
			if (val == value) {
				if (newPriority > allValues[i]) {
					throw "Attempted to lower priority of value";
				} else if (newPriority == allValues[i]) {
					return;
				} else {
					enqueue(value, newPriority);
					while (!temp1.isEmpty()) {
						temp2.enqueue(temp1.dequeue());
					}
					if (temp2.isEmpty()) {
						elements.remove(allValues[i]);
					} else {
						elements.put(allValues[i], temp2);
					}
					return;
				}
			}
			temp2.enqueue(val);
		}
	}
	throw "Value not in queue.";

}

//O(N)
void ExtraPriorityQueue::clear() {
	elements.clear();
	numElems = 0;
}

//O(N)
string ExtraPriorityQueue::dequeue() {
	if (isEmpty()) {
		throw "Tried to dequeue an empty queue.";
	}

	int index = elements.keys()[0];
	Queue<string> temp = elements.get(index);
	string val = temp.dequeue();
	if (temp.isEmpty()) {
		elements.remove(index);
	} else {
		elements.put(index, temp);
	}
	numElems--;
	return val;
}

//O(log N)
void ExtraPriorityQueue::enqueue(string value, int priority) {
	Queue<string> temp = elements.get(priority);
	temp.enqueue(value);
	elements.put(priority, temp);
	numElems++;
}

//O(1)
bool ExtraPriorityQueue::isEmpty() const {
	return elements.isEmpty();
}

//O(N)
string ExtraPriorityQueue::peek() const {
    if (isEmpty()) {
		throw "Empty queue.";
	}
	int index = elements.keys()[0];
	Queue<string> temp = elements.get(index);
	return temp.peek();
}

//O(N)
int ExtraPriorityQueue::peekPriority() const {
    if (isEmpty()) {
		throw "Empty queue.";
	}
	return elements.keys()[0];
}

//O(1)
int ExtraPriorityQueue::size() const {
	return numElems;
}

//O(M N log N)
ostream& operator<<(ostream& out, const ExtraPriorityQueue& queue) {
	out << "{";
	Vector<int> allValues = queue.elements.keys();
	int length = allValues.size();
	Queue<string> temp1;
	for (int i = 0; i < length - 1; i++) {
		temp1 = queue.elements.get(allValues[i]);
		while (!temp1.isEmpty()) {
			out << "\"" << temp1.dequeue() << "\"" << ":";
			out << allValues[i] << ", ";
		}
	}
	temp1 = queue.elements.get(allValues[length - 1]);
	while (!temp1.isEmpty()) {
		string val = temp1.dequeue();
		if (temp1.isEmpty()) {
			out << "\"" << val << "\"" << ":";
			out << allValues[length-1] << "}";
		} else {
			out << "\"" << val << "\"" << ":";
			out << allValues[length-1] << ", ";
		}
	}
	
	return out;
}
