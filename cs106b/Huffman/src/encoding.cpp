/* encoding.cpp
 * ------------
 * Juan Planas (TA: Eric Yu)
 * Implementation of Huffman coding to compress files and decompress files.
 */

#include "encoding.h"
#include "bitstream.h"
#include "pqueue.h"
#include "filelib.h"

/* Builds the Huffman character frequency table, mapping every character that
 * appeared in the input to its frequency.
 * Assumes that input is initialized and can be read.
 */
Map<int, int> buildFrequencyTable(istream& input) {
    Map<int, int> freqTable;
	int temp;
	
	while (input.good()) {
		temp = input.get();
		int count = freqTable.get(temp);
		freqTable.put(temp, count + 1);
	}

	freqTable.put(PSEUDO_EOF, 1);
	freqTable.remove(-1);

    return freqTable;
}

/* Builds the Huffman encoding tree, which creates a binary tree that
 * ultimately relates each character with a binary sequence. More frequent
 * characters have shorter sequences.
 * Assumes an initialized freqTable Map.
 */
HuffmanNode* buildEncodingTree(const Map<int, int>& freqTable) {
	PriorityQueue<HuffmanNode *> queue;
	for (int key : freqTable) {
		HuffmanNode *temp = new HuffmanNode(key, freqTable.get(key), NULL, NULL);
		queue.enqueue(temp, freqTable.get(key));
	}

	HuffmanNode *root1 = queue.dequeue();
	while (!queue.isEmpty()) {
		HuffmanNode *root2 = queue.dequeue();
		HuffmanNode *temp = new HuffmanNode(NOT_A_CHAR, root1->count + root2->count, root1, root2);
		queue.enqueue(temp, temp->count);
		root1 = queue.dequeue();
	}
	return root1;
}

/* Helper function for buildEncodingMap that recursively associates each
 * character in the string with its binary sequence, represented as a string.
 * Assumes that root is NULL or a valid pointer to a HuffmanNode, and that
 * encodingMap has been initialized. Also assumes that the root pointer indeed
 * points to a proper Huffman encoding tree, created with buildEncodingTree.
 */
void recursiveMapBuild(HuffmanNode *root, string repr, Map<int, string> &encodingMap) {
	if (root == NULL) {
		return;
	}
	if (root->character != NOT_A_CHAR) {
		encodingMap.put(root->character, repr);
	}
	recursiveMapBuild(root->zero, repr + "0", encodingMap);
	recursiveMapBuild(root->one, repr + "1", encodingMap);
}

/* Builds the Huffman encoding map, associating each character in the map with
 * its binary sequence. 
 * Assumes a properly initialized encoding tree, created with buildEncodingTree.
 */
Map<int, string> buildEncodingMap(HuffmanNode* encodingTree) {
    Map<int, string> encodingMap;
	recursiveMapBuild(encodingTree, "", encodingMap);
    return encodingMap;
}

/* Huffman encodes the input stream using the Huffman encoding map in
 * encodingMap and writes the result to the bit stream output.
 * Assumes a properly initialized encodingMap (initialized with
 * buildEncodingMap), and initialized input and output streams that can be
 * written to or read from.
 */
void encodeData(istream& input, const Map<int, string>& encodingMap, obitstream& output) {
	int temp = input.get();
	while (temp != -1) {
		string repr = encodingMap.get(temp);
		for (int i = 0; i < repr.size(); i++) {
			output.writeBit(repr[i] - '0');
		}
		temp = input.get();
	}
	string repr = encodingMap.get(PSEUDO_EOF);
	for (int i = 0; i < repr.size(); i++) {
		output.writeBit(repr[i] - '0');
	}
}

/* Decodes the previously encoded data (encoded using encodeData above) using
 * the given encoding tree.
 * Assumes an initialized encoding tree, and input and output streams that can
 * be written to and read from.
 */
void decodeData(ibitstream& input, HuffmanNode* encodingTree, ostream& output) {
	int temp = NOT_A_CHAR;
	HuffmanNode *node = encodingTree;
	while (true) {	
		while (!node->isLeaf()) {
			int bit = input.readBit();
			if (bit == 1) {
				node = node->one;
			} else {
				node = node->zero;
			}
		}
		temp = node->character;
		if (temp == PSEUDO_EOF) {
			break;
		}
		output << (char)temp;
		node = encodingTree;
	}
}

/* Compresses the input stream using Huffman encoding. Assumes that input and
 * output are initialized and can be read from or written to.
 */
void compress(istream& input, obitstream& output) {
	Map<int, int> frequencyTable = buildFrequencyTable(input);
	HuffmanNode *root = buildEncodingTree(frequencyTable);
	Map<int, string> encodingMap = buildEncodingMap(root);
	output << frequencyTable;
	rewindStream(input);
	encodeData(input, encodingMap, output);
	freeTree(root);
}

/* Decompresses the input stream using Huffman encoding. Assumes that there is
 * a valid header containing a frequency table as the first line of the file,
 * and that input and output are initialized and can be read from and written
 * to.
 */
void decompress(ibitstream& input, ostream& output) {
	Map<int, int> frequencyTable;
	input >> frequencyTable;
	HuffmanNode *root = buildEncodingTree(frequencyTable);
	Map<int, string> encodingMap = buildEncodingMap(root);
	decodeData(input, root, output);
	freeTree(root);
}

/* Recursively frees the memory used by the HuffmanNode tree under the given
 * node.
 */
void freeTree(HuffmanNode* node) {
	if (node == NULL) {
		return;
	}
	freeTree(node->zero);
	freeTree(node->one);
	delete node;
}
