#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum number of unique characters (ASCII)
#define MAX_CHARS 256
// Maximum code length (for a 256 char alphabet, worst case is 255 bits, but usually much less)
#define MAX_CODE_LENGTH MAX_CHARS

// Structure for a Huffman tree node
typedef struct HuffmanNode {
    char data;                 // Character stored in the node (for leaves)
    unsigned int frequency;    // Frequency of the character
    struct HuffmanNode *left;  // Left child
    struct HuffmanNode *right; // Right child
} HuffmanNode;

// Structure for a Min-Priority Queue node
typedef struct MinHeapNode {
    HuffmanNode *node; // Pointer to a Huffman tree node
} MinHeapNode;

// Structure for a Min-Priority Queue
typedef struct MinHeap {
    unsigned int size;     // Current size of min heap
    unsigned int capacity; // Capacity of min heap
    MinHeapNode **array;   // Array of min heap node pointers
} MinHeap;

// Global array to store Huffman codes
char huffmanCodes[MAX_CHARS][MAX_CODE_LENGTH];
// Global array to store pointers to leaf nodes for direct access during tree building
HuffmanNode* leafNodes[MAX_CHARS];

// Function to create a new Huffman tree node
HuffmanNode* createNode(char data, unsigned int frequency) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    if (!node) {
        perror("Failed to allocate HuffmanNode");
        exit(EXIT_FAILURE);
    }
    node->left = node->right = NULL;
    node->data = data;
    node->frequency = frequency;
    return node;
}

// Function to create a min-priority queue of given capacity
MinHeap* createMinHeap(unsigned int capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!minHeap) {
        perror("Failed to allocate MinHeap");
        exit(EXIT_FAILURE);
    }
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (MinHeapNode**)malloc(minHeap->capacity * sizeof(MinHeapNode*));
    if (!minHeap->array) {
        perror("Failed to allocate MinHeapNode array");
        exit(EXIT_FAILURE);
    }
    return minHeap;
}

// Function to swap two min heap nodes
void swapMinHeapNodes(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

// Standard min heapify function
void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size &&
        minHeap->array[left]->node->frequency < minHeap->array[smallest]->node->frequency) {
        smallest = left;
    }

    if (right < minHeap->size &&
        minHeap->array[right]->node->frequency < minHeap->array[smallest]->node->frequency) {
        smallest = right;
    }

    if (smallest != idx) {
        swapMinHeapNodes(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

// Function to check if size of heap is 1
int isSizeOne(MinHeap* minHeap) {
    return (minHeap->size == 1);
}

// Function to extract minimum value node from heap
MinHeapNode* extractMin(MinHeap* minHeap) {
    MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    minHeap->size--;
    minHeapify(minHeap, 0);
    return temp;
}

// Function to insert a new node to min heap
void insertMinHeap(MinHeap* minHeap, HuffmanNode* node) {
    MinHeapNode* minHeapNode = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    if (!minHeapNode) {
        perror("Failed to allocate MinHeapNode for insertion");
        exit(EXIT_FAILURE);
    }
    minHeapNode->node = node;

    minHeap->size++;
    int i = minHeap->size - 1;

    while (i && minHeapNode->node->frequency < minHeap->array[(i - 1) / 2]->node->frequency) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

// Function to build min heap
void buildMinHeap(MinHeap* minHeap) {
    int n = minHeap->size - 1;
    for (int i = (n - 1) / 2; i >= 0; --i) {
        minHeapify(minHeap, i);
    }
}

// Function to build the Huffman tree
HuffmanNode* buildHuffmanTree(char data[], int freq[], int size) {
    MinHeap* minHeap = createMinHeap(size);

    for (int i = 0; i < size; ++i) {
        leafNodes[(unsigned char)data[i]] = createNode(data[i], freq[i]); // Store pointer to leaf node
        insertMinHeap(minHeap, leafNodes[(unsigned char)data[i]]);
    }

    buildMinHeap(minHeap); // Build heap from all initial nodes

    while (!isSizeOne(minHeap)) {
        // Extract the two minimum frequency nodes from min heap
        HuffmanNode* left = extractMin(minHeap)->node;
        HuffmanNode* right = extractMin(minHeap)->node;

        // Create a new internal node with frequency equal to the sum of the two nodes' frequencies.
        // Make the two extracted nodes as left and right children of this new node.
        // Add this new node to the min heap. '*' is a dummy character for internal nodes.
        HuffmanNode* top = createNode('$', left->frequency + right->frequency);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }

    // The remaining node is the root node and the complete Huffman tree
    HuffmanNode* root = extractMin(minHeap)->node;

    // Clean up minHeap structure (but not the HuffmanNodes it pointed to)
    for (unsigned int i = 0; i < minHeap->capacity; ++i) {
        free(minHeap->array[i]);
    }
    free(minHeap->array);
    free(minHeap);

    return root;
}

// Function to generate Huffman codes by traversing the Huffman tree
void generateCodes(HuffmanNode* root, char* code, int top) {
    if (root->left) {
        code[top] = '0';
        generateCodes(root->left, code, top + 1);
    }

    if (root->right) {
        code[top] = '1';
        generateCodes(root->right, code, top + 1);
    }

    // If this is a leaf node, then it contains one of the input characters,
    // store its code
    if (!(root->left) && !(root->right)) {
        code[top] = '\0';
        strcpy(huffmanCodes[(unsigned char)root->data], code);
    }
}

// Function to print Huffman codes (for debugging/verification)
void printCodes() {
    printf("Huffman Codes:\n");
    for (int i = 0; i < MAX_CHARS; ++i) {
        if (huffmanCodes[i][0] != '\0') { // If a code exists for this character
            printf("'%c': %s\n", (char)i, huffmanCodes[i]);
        }
    }
}

// Function to encode the input text
char* encodeText(const char* text) {
    if (!text || text[0] == '\0') return NULL;

    // Calculate required buffer size for encoded text
    size_t encoded_len = 0;
    for (int i = 0; text[i] != '\0'; ++i) {
        unsigned char c = text[i];
        if (huffmanCodes[c][0] == '\0') {
            fprintf(stderr, "Error: Character '%c' not in Huffman tree for encoding.\n", c);
            return NULL; // Should not happen if tree is built correctly
        }
        encoded_len += strlen(huffmanCodes[c]);
    }

    char* encoded_text = (char*)malloc(encoded_len + 1);
    if (!encoded_text) {
        perror("Failed to allocate encoded_text");
        exit(EXIT_FAILURE);
    }
    encoded_text[0] = '\0'; // Ensure it's an empty string

    for (int i = 0; text[i] != '\0'; ++i) {
        unsigned char c = text[i];
        strcat(encoded_text, huffmanCodes[c]);
    }

    return encoded_text;
}

// Function to decode the encoded binary string
char* decodeText(HuffmanNode* root, const char* encoded_text) {
    if (!root || !encoded_text || encoded_text[0] == '\0') return NULL;

    size_t text_len = strlen(encoded_text);
    // Max possible decoded length is same as encoded length (if all chars had '0'/'1' code)
    // For practical text, it's usually much less, but this is a safe upper bound
    char* decoded_text = (char*)malloc(text_len + 1);
    if (!decoded_text) {
        perror("Failed to allocate decoded_text");
        exit(EXIT_FAILURE);
    }
    decoded_text[0] = '\0'; // Empty string

    HuffmanNode* current = root;
    int decoded_idx = 0;

    for (int i = 0; i < text_len; ++i) {
        if (encoded_text[i] == '0') {
            current = current->left;
        } else { // encoded_text[i] == '1'
            current = current->right;
        }

        // Found a leaf node
        if (!(current->left) && !(current->right)) {
            decoded_text[decoded_idx++] = current->data;
            current = root; // Reset to root for next character
        }
    }
    decoded_text[decoded_idx] = '\0'; // Null-terminate the decoded string

    return decoded_text;
}

// Function to free the Huffman tree
void freeHuffmanTree(HuffmanNode* node) {
    if (node == NULL) return;
    freeHuffmanTree(node->left);
    freeHuffmanTree(node->right);
    free(node);
}


int main() {
    // Initialize huffmanCodes array
    for (int i = 0; i < MAX_CHARS; ++i) {
        huffmanCodes[i][0] = '\0';
        leafNodes[i] = NULL;
    }

    // Sample input text
    const char* inputText = "this is an example for huffman encoding";
    printf("Original text: %s\n", inputText);

    // 1. Calculate character frequencies
    unsigned int frequency[MAX_CHARS] = {0};
    int unique_chars_count = 0;
    char unique_chars[MAX_CHARS];

    for (int i = 0; inputText[i] != '\0'; ++i) {
        unsigned char c = inputText[i];
        if (frequency[c] == 0) {
            unique_chars[unique_chars_count++] = c;
        }
        frequency[c]++;
    }

    char* data = (char*)malloc(unique_chars_count * sizeof(char));
    int* freq_arr = (int*)malloc(unique_chars_count * sizeof(int));
    if (!data || !freq_arr) {
        perror("Failed to allocate data/freq_arr");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < unique_chars_count; ++i) {
        data[i] = unique_chars[i];
        freq_arr[i] = frequency[(unsigned char)unique_chars[i]];
    }

    // 2. Build Huffman tree
    HuffmanNode* root = buildHuffmanTree(data, freq_arr, unique_chars_count);
    if (!root) {
        fprintf(stderr, "Failed to build Huffman tree.\n");
        free(data);
        free(freq_arr);
        return 1;
    }

    // 3. Generate Huffman codes
    char codeBuffer[MAX_CODE_LENGTH];
    generateCodes(root, codeBuffer, 0);

    printCodes();

    // 4. Encode the text
    char* encodedText = encodeText(inputText);
    if (encodedText) {
        printf("\nEncoded text: %s\n", encodedText);
        printf("Encoded length: %zu bits\n", strlen(encodedText));
        printf("Original length: %zu characters (%zu bytes if 8-bit ASCII)\n",
               strlen(inputText), strlen(inputText));
    } else {
        fprintf(stderr, "Encoding failed.\n");
        freeHuffmanTree(root);
        free(data);
        free(freq_arr);
        return 1;
    }

    // 5. Decode the text
    char* decodedText = decodeText(root, encodedText);
    if (decodedText) {
        printf("\nDecoded text: %s\n", decodedText);
        printf("Decoded length: %zu characters\n", strlen(decodedText));
    } else {
        fprintf(stderr, "Decoding failed.\n");
        free(encodedText);
        freeHuffmanTree(root);
        free(data);
        free(freq_arr);
        return 1;
    }

    // Verify decoded text matches original
    if (strcmp(inputText, decodedText) == 0) {
        printf("\nVerification: Decoded text matches original text. Success!\n");
    } else {
        printf("\nVerification: Decoded text DOES NOT match original text. Failure!\n");
    }

    // Clean up
    freeHuffmanTree(root);
    free(data);
    free(freq_arr);
    free(encodedText);
    free(decodedText);

    return 0;
}