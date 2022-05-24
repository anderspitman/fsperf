#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>

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

    fprintf(stdout, "Size: %ld\n", st.st_size);

    FILE *outFile = fopen(outPath, "w");

    if (0 == strcmp(command, "fread")) {
        fprintf(stdout, "Using fread\n");
        readDirect(filePath, st.st_size, bufSize, outFile);
    }
    else if (0 == strcmp(command, "mmap")) {
        fprintf(stdout, "Using mmap\n");
        readMmap(filePath, st.st_size, bufSize, outFile);
    }
    else {
        fprintf(stderr, "Invalid command: %s\n", command);
        return 1;
    }

    fclose(outFile);
   
    return(0);
}
