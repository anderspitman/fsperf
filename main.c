// Needed for clock_gettime when compiling for C11, see here:
// https://stackoverflow.com/a/40515669
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>


// Taken from here: https://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
struct timespec diff(struct timespec start, struct timespec end) {
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}


void readDirect(char *filePath, size_t size, size_t bufSize, FILE *outFile) {
    FILE *fp = fopen(filePath, "r");

    char *buffer = (char *)malloc(bufSize);

    for (size_t i = 0; i < size; i += bufSize) {
        size_t readSize = bufSize;
        //fprintf(stdout, "%ld, %ld, %ld\n", i, readSize, size);
        if (i + readSize > size) {
            readSize = size - i;
        }

        fread(buffer, readSize, 1, fp);

        fwrite(buffer, sizeof(char), readSize, outFile);
    }

    free(buffer);

    fclose(fp);
}

void readMmap(char *filePath, size_t size, size_t bufSize, FILE *outFile) {

    int fd = open(filePath, O_RDONLY);

    void* ptr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "mmap failed\n");
        return;
    }

    close(fd);

    for (size_t i = 0; i < size; i += bufSize) {
        size_t readSize = bufSize;
        //fprintf(stdout, "%ld, %ld, %ld\n", i, readSize, size);
        if (i + readSize > size) {
            readSize = size - i;
        }

        fwrite(ptr, sizeof(char), readSize, outFile);

        ptr += readSize;
    }
}

int main (int argc, char** argv) {

    if (argc != 5) {
        fprintf(stderr, "Usage: %s COMMAND BUF_SIZE INFILE OUTFILE\n", argv[0]);
        exit(1);
    }

    char* command = argv[1];
    size_t bufSize = atoi(argv[2]);
    char* filePath = argv[3];
    char* outPath = argv[4];

    struct stat st;

    if (stat(filePath, &st)) {
        fprintf(stderr, "Failed to stat\n");
        return 1;
    }

    //fprintf(stdout, "Size: %ld\n", st.st_size);

    FILE *outFile = fopen(outPath, "w");

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    if (0 == strcmp(command, "fread")) {
        fprintf(stderr, "Using fread\n");
        readDirect(filePath, st.st_size, bufSize, outFile);
    }
    else if (0 == strcmp(command, "mmap")) {
        fprintf(stderr, "Using mmap\n");
        readMmap(filePath, st.st_size, bufSize, outFile);
    }
    else {
        fprintf(stderr, "Invalid command: %s\n", command);
        return 1;
    }

    fclose(outFile);

    clock_gettime(CLOCK_MONOTONIC, &end);

    struct timespec elapsed = diff(start, end);
    double fractionalSeconds = (double)elapsed.tv_nsec / 1000000000.0;
    double elapsedSeconds = (double)elapsed.tv_sec + fractionalSeconds;

    double gbps = ((double)st.st_size / 1000000000.0) / elapsedSeconds;

    fprintf(stderr, "%lu bytes copied in %.3lf seconds (%.3lf GB/s)\n", st.st_size, elapsedSeconds, gbps);
    //fprintf(stdout, "command\tbufSize\tfileSize\telapsedSeconds\tGB/s\n");
    fprintf(stdout, "%s\t%d\t%lu\t%lf\t%.3lf\n", command, bufSize, st.st_size, elapsedSeconds, gbps);

    return(0);
}
