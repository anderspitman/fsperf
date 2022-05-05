#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

//#define BUF_SIZE 4096
//#define BUF_SIZE 1048576
#define BUF_SIZE 1

void readDirect(char *filePath, size_t size) {
    FILE *fp = fopen(filePath, "r");
    char buffer[BUF_SIZE];

    int numChunks = size / BUF_SIZE;

    for (int i = 0; i < numChunks + 1; i++) {
        fread(buffer, BUF_SIZE, 1, fp);
        //printf("%s", buffer);
    }

    fclose(fp);
}

void readMmap(char *filePath, size_t size) {
    char buf[BUF_SIZE];
    int fd = open(filePath, O_RDONLY);
    void* ptr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (ptr == MAP_FAILED) {
        fprintf(stderr, "mmap failed\n");
        return;
    }

    close(fd);

    int numChunks = size / BUF_SIZE;

    //int outPath = open("/dev/null", O_WRONLY);
    //int outPath = open("./out.bin", O_CREAT|O_WRONLY);

    for (size_t i = 0; i < size; i += BUF_SIZE) {
        size_t readSize = BUF_SIZE;
        //fprintf(stdout, "%ld, %ld, %ld\n", i, readSize, size);
        if (i + readSize > size) {
            readSize = size - i;
        }

        memcpy(buf, ptr, readSize);

        //int res = write(outPath, buf, readSize);
        //if (res < 0) {
        //    fprintf(stderr, "Error write\n");
        //}

        ptr += readSize;
    }
}

int main (int argc, char** argv) {

    char* command = argv[1];
    char* filePath = argv[2];


    struct stat st;

    if (stat(filePath, &st)) {
        fprintf(stderr, "Failed to stat\n");
        return 1;
    }

    fprintf(stdout, "Size: %ld\n", st.st_size);

    if (0 == strcmp(command, "fread")) {
        fprintf(stdout, "Using fread\n");
        readDirect(filePath, st.st_size);
    }
    else if (0 == strcmp(command, "mmap")) {
        fprintf(stdout, "Using mmap\n");
        readMmap(filePath, st.st_size);
    }
    else {
        fprintf(stderr, "Invalid command: %s\n", command);
        return 1;
    }

   
    return(0);
}
