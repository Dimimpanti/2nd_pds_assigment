#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


/**
 * @brief Compare function for qsort
 * 
 * @param a 
 * @param b 
 * @return int 
 */
int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}


void sortDataFile(char *fileName, int numOfInts) {

    FILE *file = fopen(fileName, "r");

    if (file == NULL) {
        printf("Can not open file \"%s\".\n", fileName);
        exit(EXIT_FAILURE);
    }

    int *values = (int *)malloc(numOfInts * sizeof(int));

    for (int i = 0; i < numOfInts; ++i) {
        fscanf(file, "%d", &values[i]);
    }

    fclose(file);

    // Sort the array using quicksort
    qsort(values, numOfInts, sizeof(int), compare);

    // Write the sorted array to a new file with name "sorted_<fileName>"
    char *sortedFileName = (char *)malloc(1000 * sizeof(char));
    
    strcpy(sortedFileName, fileName);

    int i = 0; // length of the string
    while (sortedFileName[i] != '\0') {
        ++i;
    }

    // remove the .txt part
    i -= 4;

    // add "_sorted.txt" to the end of the string
    strcpy(sortedFileName + i, "_sorted.txt");

    // add \0 to the end of the string
    sortedFileName[i + 11] = '\0';

    file = fopen(sortedFileName, "w");

    if (file == NULL) {
        printf("Can not open file \"%s\".\n", sortedFileName);
        exit(EXIT_FAILURE);
    }

    // Write the sorted array to the file one number per line
    for (int i = 0; i < numOfInts; ++i) {
        fprintf(file, "%d: %d\n", i + 1, values[i]);
    }

    fclose(file);

    free(values);
    free(sortedFileName);
}

/**
 * @brief This is a program that creates random integers and writes them to a file
 * 
 * @param argc  The number of arguments
 * @param argv  The arguments of the program 
 *                  argv[1] is the name of the file,
 *                  argv[2] is the number of integers to be generated
 * @return int 
 */
int main(int argc, char **argv) {
    
    if (argc != 3) {
        printf("Invalid number of arguments.\n");
        exit(EXIT_FAILURE);
    }

    char *fileName = argv[1];

    int powerOfTwo = atoi(argv[2]);

    int numOfInts = 1 << powerOfTwo;

    FILE *file = fopen(fileName, "w");

    if (file == NULL) {
        printf("Can not open file \"%s\".\n", fileName);
        exit(EXIT_FAILURE);
    }

    // Seed for random numbers
    srand(time(NULL));

    // Generate random integers and write them to the file. The integers are between -99999999 and 99999999
    for (int i = 0; i < numOfInts; ++i) {
        fprintf(file, "%d ", rand() % 199999999 - 99999999);
    }

    fclose(file);

    // Sort the file
    sortDataFile(fileName, numOfInts);

    return 0;
}