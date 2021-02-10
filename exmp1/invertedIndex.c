// COMP2521 (20T3): Assignment 1
// Information Retrieval
// Written by Min-Sik Son (z5310901)

#include "invertedIndex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define MAX_LENGTH 100

//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Declartation of functions here /////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// STAGE 1 /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
char *normaliseWord(char *str);
InvertedIndexBST generateInvertedIndex(char *collectionFilename);
void printInvertedIndex(InvertedIndexBST tree);
// STAGE 1 HELPER FUNCTIONS //
InvertedIndexBST insertWord(InvertedIndexBST q, char *filename, char *str);
FileList updateFileList(FileList n, char *filename, char *word);
double termFrequencyCalculation (char *filename, char *word);
FileList insertFileNode (char *filename, double tf);
void helperPrintFunction(InvertedIndexBST tree, FILE *fp);

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// STAGE 2 /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
TfIdfList calculateTfIdf(InvertedIndexBST tree, char *searchWord, int D);
TfIdfList retrieve(InvertedIndexBST tree, char *searchWords[], int D);
// STAGE 2 HELPER FUNCTIONS //
InvertedIndexBST locateWord(InvertedIndexBST q, char *str);
TfIdfList insertTfidfNode (char *filename, double tf, double idf);
TfIdfList addToReturnList(TfIdfList curr, TfIdfList insertingNode);
int checkExistence(TfIdfList retrieveList, char *filename);
TfIdfList insertRetrieveNode (char *filename, double tfIdfSum);
TfIdfList sortList(TfIdfList unsortedList);
TfIdfList findMax(TfIdfList list);




/////////////////////////////////////////////////////////////////////////////////
///////////////////////////// FUNCTIONS FOR STAGE 1 /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// Normalise the given string:
// * removing leading and trailing spaces
// * converting all characters to lowercase
// * remove the following punctuation marks . , ; ?
char *normaliseWord(char *str) {
    int i = 0;
    // Let us first convert the string to lower case
    while (str[i]) {
        str[i] = tolower(str[i]);
        i++;
    }

    // Removing the unwanted characters
    if (str[i-1] == '.'
        || str[i-1] == ','
        || str[i-1] == ';'
        || str[i-1] == '?') {
        str[i-1] = '\0';
    }
    
    // Return the updated string
    return str;
}

InvertedIndexBST generateInvertedIndex(char *collectionFilename) {
    // Root of tree
    InvertedIndexBST q = NULL;
    
    // Brief plan for this function
    // (1) We will open the collection file and open each file listed inside
    // (2) For every word we increment we will normalise the word, insert and
    //     update the file linked list
    
    // Initialising the following strings
    char *currFile = malloc(sizeof(char)*100);
    char *currWord = malloc(sizeof(char)*100);

    FILE *mainFile = fopen(collectionFilename, "r");

    while (fscanf(mainFile, "%s", currFile) != EOF) {
        FILE *file = fopen(currFile, "r");
        while (fscanf(file, "%s", currWord) != EOF) {
            currWord = normaliseWord(currWord);
            q = insertWord(q, currFile, currWord);
        }
        fclose(file);
    }
    fclose(mainFile);
    
    return q;
}

// Prints out the tree in alphabetical order
// This will print each word in the tree as well as the file linked list that contains filenames and tf values
void printInvertedIndex(InvertedIndexBST tree) {
    if (tree == NULL) {
        return;
    } else {
        FILE *fp = fopen("invertedIndex.txt", "w");
        
        helperPrintFunction(tree, fp);
        
        fclose(fp);
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////
///////////////////////////// FUNCTIONS FOR STAGE 2 /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

TfIdfList calculateTfIdf(InvertedIndexBST q, char *searchWord, int D) {
    // Find the given word in the tree
    InvertedIndexBST givenWord = locateWord(q, searchWord);
    
    // For the given search word, we will now count the number of files in the list
    FileList currList = givenWord->fileList;
    double containWord = 0;
    while (currList != NULL) {
        containWord++;
        currList = currList->next;
    }

    // Since the total number of documents is already given
    double idf = log10(D / containWord);

    // Now we will increment through the list of that given word and add a new node to the new list
    FileList searchList = givenWord->fileList;
    // Make the new linked list we want to return, and initialising it here
    TfIdfList returnList = NULL;
    // Add a new node to the new list we just made as we increment through searchList
    while (searchList != NULL) {
        TfIdfList insertingNode = insertTfidfNode(searchList->filename, searchList->tf, idf);
        returnList = addToReturnList(returnList, insertingNode);
        searchList = searchList->next;
    }

    return returnList;
}

// This function returns an ordered list where each node contains a filename and the summation of tf-idf values
// of all the matching searchWords for that file
TfIdfList retrieve(InvertedIndexBST tree, char *searchWords[], int D) {
    // Let us first get the length of the given string
    int searchWordsLength = 0;
    int count = 0;
    while (searchWords[count] != NULL) {
        searchWordsLength++;
        count++;
    }
    // Make the new linked list we want to return, and initialising it here
    TfIdfList retrieveList = NULL;
    
    int i = 0;
    while (i < searchWordsLength) {
        // This will return the list containing the filenames and tfIdfSum
        TfIdfList q = calculateTfIdf(locateWord(tree, searchWords[i]), searchWords[i], D);
        // Base case if list is empty we just add
        if (retrieveList == NULL) {
            retrieveList = q;

        } else {
            // Looping through the linked list to insert its info into the retrieveList
            while (q != NULL) {
                // The head of the list we want to return
                // We will reset the head everytime a new loop begins
                TfIdfList head = retrieveList;
                // Checking to see if there are duplicates with the filename we add to the list
                // If so, only update the tfIdSum
                if (checkExistence(head, q->filename) == 1) {
                    // If we go inside this if statement we know 100% there is a duplicate filename
                    while (strcmp(head->filename, q->filename) != 0) {
                        head = head->next;
                    }
                    // So now when we exit this while loop we arrive at the node with the duplicate filename
                    // Update the tfIdfSum of this node
                    head->tfIdfSum = head->tfIdfSum + q->tfIdfSum;
                } else {
                    // Just insert to the end of the list and we will sort it at the very end after we
                    // complete all insertions
                    while (head->next != NULL) {
                        head = head->next;
                    }
                    head->next = insertRetrieveNode(q->filename, q->tfIdfSum);
                }

                q = q->next;
            }
        }
        i++;
    }
    // By now we should have a linked list to return but it is not yet sorted
    retrieveList = sortList(retrieveList);
    return retrieveList;
}

/////////////////////////////////////////////////////////////////////////////////
///////////////////////////// HELPER FUNCTIONS FOR STAGE 1 //////////////////////
/////////////////////////////////////////////////////////////////////////////////

// Inserts a unique word into binary tree, checks for duplicates as well as alphabetical insertion
InvertedIndexBST insertWord(InvertedIndexBST q, char *filename, char *str) {
    if (q == NULL) { // If tree is empty
        // Create rootNode
        InvertedIndexBST rootNode = malloc(sizeof(struct InvertedIndexNode));
        rootNode->word = malloc(sizeof(char)*100);
        // Initialise the elements of this new node
        strcpy(rootNode->word, str);
        rootNode->fileList = NULL;
        rootNode->left = NULL;
        rootNode->right = NULL;
        // And we will now update the file linked list of this node
        rootNode->fileList = updateFileList(rootNode->fileList, filename, str);
        return rootNode;
    } else if (strcmp(q->word, str) == 0) { // If the inserting word is equal to the existing word
        q->fileList = updateFileList(q->fileList, filename, str);
    } else if (strcmp(q->word, str) > 0) { // If the existing word is larger than the inserting word
        q->left = insertWord(q->left, filename, str);
    } else if (strcmp(q->word, str) < 0) { // If the existing word is smaller than the inserting word
        q->right = insertWord(q->right, filename, str);
    }
    // Then we return the tree
    return q;
}

// Updates the file linked list for the word
// Here, we need the filename and the relative term frequency to update the list
FileList updateFileList(FileList currFileList, char *filename, char *word) {
    // First calculate the term frequency for the required word
    double termFreq = termFrequencyCalculation (filename, word);
    // Alphabetical insertion by recursion
    if (currFileList == NULL) {
        return insertFileNode(filename, termFreq);
    } else if (strcmp(currFileList->filename, filename) == 0) { // If the filename already exists in the list
        return currFileList;
    } else if (strcmp(currFileList->filename, filename) > 0) { // If the inserting filename is less than the current
        // We will make a new node and connect its next to the current node
        // Afterwards, we return what we just inserted
        FileList addFile = insertFileNode(filename, termFreq);
        addFile->next = currFileList;
        currFileList = addFile;
    } else { // Otherwise we use recursion here on the current's next
        currFileList->next = updateFileList(currFileList->next, filename, word);
    }
    return currFileList;
}

// Calculates relative term frequency
double termFrequencyCalculation (char *filename, char *word){
    FILE *file = fopen(filename, "r");
    
    char *currWord = malloc(sizeof(char)*100);
    // We need two things, the frequency of the term in the document and the
    // number of words inside the document
    double freqOfTerm = 0; // Frequency of term in document
    double numWords = 0; // Number of words in document
    
    while (fscanf(file, "%s", currWord) != EOF) {
        currWord = normaliseWord(currWord);
        if (strcmp(currWord, word) == 0) {
            freqOfTerm++;
            numWords++;
        } else {
            numWords++;
        }
    }
    
    // Now we have the values to calculate the relative termf frequency
    double relTermFreq = freqOfTerm / numWords;
    fclose(file);
    return relTermFreq;
}

// Function to create a node with the given filename and term frequency value
FileList insertFileNode (char *filename, double tf) {
    FileList newNode = malloc(sizeof(struct FileListNode));
    newNode->filename = malloc(sizeof(char)*100);
    strcpy(newNode->filename, filename);
    newNode->next = NULL;
    newNode->tf = tf;
    return newNode;
}

// Used for the printInvertedIndex function
void helperPrintFunction(InvertedIndexBST tree, FILE *fp) {
    // Base case, if the tree is empty
    if (tree == NULL) {
        return;
    }

    // We already have a sorted tree so we navigate to the left until it is NULL to print lowest to highest
    helperPrintFunction(tree->left, fp);
    
    fprintf(fp, "%s ",tree->word);
    FileList curr = tree->fileList;
    // Printing out the file linked list which is already sorted alphabetically
    while (curr != NULL) {
        fprintf(fp,"%s ",curr->filename);
        fprintf(fp,"(%lf) ",curr->tf);
        curr = curr->next;
    }
    fprintf(fp,"\n");

    helperPrintFunction(tree->right, fp);
}

/////////////////////////////////////////////////////////////////////////////////
///////////////////////////// HELPER FUNCTIONS FOR STAGE 2 //////////////////////
/////////////////////////////////////////////////////////////////////////////////

// This function will search the word in the tree for us to calculate the TfIdf
// This will also be used in stage two for the second function
InvertedIndexBST locateWord(InvertedIndexBST q, char *str){
    if (q == NULL) {
        return NULL;
    } else if (strcmp(q->word, str) == 0) {
        return q;
    } else if (strcmp(q->word, str) < 0) {
        return locateWord(q->right, str);
    }else {
        return locateWord(q->left, str);
    }
}

// Creating a node for the new linked list
TfIdfList insertTfidfNode (char *filename, double tf, double idf) {
    TfIdfList addNode = malloc(sizeof(TfIdfList));
    addNode->filename = malloc(sizeof(char)*100);
    strcpy(addNode->filename, filename);
    addNode->tfIdfSum = (tf * idf);
    addNode->next = NULL;
    return addNode;
}

// Adding to the new list we will return
TfIdfList addToReturnList(TfIdfList curr, TfIdfList insertingNode) {
    if (curr == NULL) {
        return insertingNode;
    } else if (insertingNode->tfIdfSum == curr->tfIdfSum) { // If tfIdfSum's are equal we have to consider a few things
        // Since we have to insert the the files in alphabetical order
        if (strcmp(insertingNode->filename, curr->filename) > 0) { // insertingNode is greater than current
            insertingNode->next = curr->next;
            curr->next = insertingNode;
        } else if (strcmp(insertingNode->filename, curr->filename) < 0) { // // insertingNode is less than current
            insertingNode->next = curr;
            return insertingNode;
        } else { // Otherwise we return current
            return curr;
        }
    } else if (insertingNode->tfIdfSum > curr->tfIdfSum) { // If the insertinNode tfIdfSum is greater than current
        insertingNode->next = curr;
        return insertingNode;
    } else {
        curr->next = addToReturnList(curr->next, insertingNode); // Otherwise we use recursion on the next
    }
    return curr;
}

// Checks if there are duplicates
int checkExistence(TfIdfList retrieveList, char *filename) {
    TfIdfList curr = retrieveList;
    // Increment through the retrieveList
    while (curr != NULL) {
        if (strcmp(curr->filename, filename) == 0) {
            return 1; // Exit this function since we found a duplicate
        }
        curr = curr->next;
    }
    // We searched the entire list and found no duplicates
    return 0;
}

// Creating a node for the retrieve linked list
TfIdfList insertRetrieveNode (char *filename, double tfIdfSum) {
    TfIdfList addNode = malloc(sizeof(TfIdfList));
    addNode->filename = malloc(sizeof(char)*100);
    strcpy(addNode->filename, filename);
    addNode->tfIdfSum = tfIdfSum;
    addNode->next = NULL;
    return addNode;
}

// We will use recursion to find the maximum of the list and make it the head
// For example: 2 8 6 4 -> 8 2 6 4 -> 8 6 2 4 -> 8 6 4 2
TfIdfList sortList(TfIdfList unsortedList) {
    if (unsortedList == NULL || unsortedList->next == NULL) {
        return unsortedList;
    } else {
        // printf("TESTING MOVEMAX\n");
        TfIdfList moveMax = findMax(unsortedList);
        TfIdfList count = unsortedList;
        // If the node we need to move is at the end of the list
        if (moveMax->next == NULL) {
            // Navigate to the node before moveMax since count will be the new tail of the list
            while(count->next != moveMax) {
                count = count->next;
            }
            moveMax->next = unsortedList;
            count->next = NULL;
            // The new head is now moveMax
            unsortedList = moveMax;
        } else if (count == moveMax) { // If the max node is already at the head
            unsortedList->next = sortList(unsortedList->next);
        } else if (moveMax->next != NULL) {
            // Navigate to the node before moveMax
            while(count->next != moveMax) {
                count = count->next;
            }
            count->next = moveMax->next;
            moveMax->next = unsortedList;
            unsortedList = moveMax;
        }
        // Using recursion to sort the remaining part of the list
        unsortedList->next = sortList(unsortedList->next);
    }
    return unsortedList;
}

// Finds the maximum node in the linked list
TfIdfList findMax(TfIdfList list) {
    TfIdfList curr = list;
    TfIdfList max = curr;
    // Base case if the list is empty, return NULL
    if (curr == NULL) {
        return NULL;
    } else {
        // As we increment through the list we will update the max node
        while (curr != NULL) {
            if (max->tfIdfSum < curr->tfIdfSum) {
                max = curr;
            } else if (max->tfIdfSum == curr->tfIdfSum) {
                // If values are equal we have to sort filenames alphabetically
                if (strcmp(max->filename, curr->filename) > 0) {
                    max = curr;
                }
            }
            curr = curr->next;
        }
    }
    return max;
}