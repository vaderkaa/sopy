#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname) {
    
    fprintf(stderr, "USAGE:%s -p PATH\n", pname);
    exit(EXIT_FAILURE);

}

void scan_directory(const char* path_name, const char* extension, int depth, FILE* f) {

    if (depth == 0) { return; }
    
    DIR* dir;
    struct dirent* dp;

    if ((dir = opendir(path_name)) == NULL) {

        //ERR("opendir");
        perror("opendir");
        return;

    }

    fprintf(f, "path: %s\n", path_name);

    errno = 0;

    while ((dp = readdir(dir)) != NULL) {
        
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {

            continue;

        }
        
        char file_path[1024] = "";
        struct stat f_buf;
        int ret = 0;

        strcat(file_path, path_name);
        strcat(file_path, "/");
        strcat(file_path, dp->d_name);

        if ((ret = lstat(file_path, &f_buf)) == -1) {

            ERR("lstat");

        }

        if (S_ISDIR(f_buf.st_mode)) {

            scan_directory(file_path, extension, depth - 1, f);

        }

        else if (S_ISREG(f_buf.st_mode)) {
            
            if (extension == NULL || strstr(dp->d_name, extension) != NULL) {
                
                fprintf(f, "%s %ld\n", dp->d_name, (long)f_buf.st_size);

            }

        }

        errno = 0;

    }

    if (errno != 0) {

        ERR("readdir");

    }

    
    closedir(dir);

}

int main(int argc, char** argv) {

    int opt;
    char* arr[argc];
    int index = 0;
    char* extension = NULL;
    int depth = 1;
    char* file = NULL;
    const char* var = "L1_OUTPUTFILE";

    while ((opt = getopt(argc, argv, "p:d:e:o")) != -1) {

        switch (opt) {

            case 'p':

                arr[index++] = optarg;
                break;
            
            case 'e':

                extension = optarg;
                break;
            
            case 'd':

                depth = strtol(optarg, NULL, 10);
                break;

            case 'o':

                file = getenv(var);
                break;

            
            case '?':
                
                default:
                    usage(argv[0]);

        }

    }

    FILE* f = stdout;

    if (file != NULL) {

        if ((f = fopen(file, "w")) == NULL) {

            ERR("fopen");

        }

    }

    int i;
    for (i = 0; i < index; i++) {

        scan_directory(arr[i], extension, depth, f);

    }

    if (f != stdout) {

        fclose(f);

    }
    
    return EXIT_SUCCESS;

}
