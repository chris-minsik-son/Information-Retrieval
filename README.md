# Information-Retrieval

Part 1 implements the required functions in the file invertedIndex.c that reads data from a given collection of files in collection.txt and generates an "inverted index" that
provides a sorted list (set) of filenames for every word in a given collection of files. Using binary search ADT to implement the inverted index.

Part 2 implements an information retrieval function that finds files with one or more query terms, and uses the summation of tf-idf values of all matching query terms for
ranking such files. You need to calculate the tf-idf value for each matching query term in a file, and rank files based on the summation of tf-idf values for all matching query
terms present in that file.
