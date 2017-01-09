#include <stdio.h>
#include <stdlib.h>

#define BLOCKSIZE (1024*1024)

/** 
 * Count the number of occurrences of a given character
 *
 * This function will read the given file in blocks 
 * of BLOCKSIZE and count each occurence of the 
 * given char.
 **/
int file_count_occurrences(FILE *fp, char c) {
    char buf[BLOCKSIZE];
    int count = 0;
    size_t n;

    while (!feof(fp) && (n = fread(buf, sizeof(char), BLOCKSIZE, fp)) > 0) {
        int x;
        for (x=0; x<n; x++) {
            if (buf[x] == c) {
                count ++;
            }
        }
    }

    return count;
}
