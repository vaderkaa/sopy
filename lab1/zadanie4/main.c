#define _XOPEN_SOURCE 700

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

// join 2 path. returned pointer is for newly allocated memory and must be freed
char* join_paths(const char* path1, const char* path2)
{
    char* res;
    const int l1 = strlen(path1);
    if (path1[l1 - 1] == '/')
    {
        res = malloc(strlen(path1) + strlen(path2) + 1);
        if (!res)
            ERR("malloc");
        strcpy(res, path1);
    }
    else
    {
        res = malloc(strlen(path1) + strlen(path2) + 2);  // additional space for "/"
        if (!res)
            ERR("malloc");
        strcpy(res, path1);
        res[l1] = '/';
        res[l1 + 1] = 0;
    }
    return strcat(res, path2);
}

void usage(int argc, char** argv)
{
    (void)argc;
    fprintf(stderr, "USAGE: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
}

void scan_book(char* path, char* author, char* title, char* genre) {

    FILE* f;

    if ((f = fopen(path, "r")) == NULL) {

        ERR("fopen");

    }

    char* buf = NULL;
    size_t buf_size = 0;

    /*
    char author[1024] = "";
    char title[1024] = "";
    char genre[1024] = "";
    */

    char line[1024] = "";

    while (getline(&buf, &buf_size, f) != -1) {

        char* s = strchr(buf, ':');

        if (s != NULL) {

            int len = s - buf;

            strncpy(line, buf, len);
            line[len] = '\0';

            if (strcmp("author", line) == 0) {

                strcpy(author, s + 1);

            }

            else if (strcmp("title", line) == 0) {

                strcpy(title, s + 1);

            }

            else if (strcmp("genre", line) == 0) {

                strcpy(genre, s + 1);

            }

        }

        /*
        else {

            free(buf);
            fclose(f);
            return;

        }
        */

    }

    /*
    if (strlen(author) > 0) {

        printf("author: %s", author);

    }

    else {

        printf("author: missing");

    }

    if (strlen(title) > 0) {

        printf("title: %s", title);

    }

    else {

        printf("title: missing");

    }

    if (strlen(genre) > 0) {

        printf("genre: %s", genre);

    }

    else {

        printf("genre: missing");

    }
    */

    free(buf); // wazne
    fclose(f);

}

void scan_library(char* path) {

    DIR* dir;
    struct dirent* dr;
    struct stat st;
    
    if ((dir = opendir(path)) == NULL) {

        ERR("opendir");

    }

    while ((dr = readdir(dir)) != NULL) {

        if (strcmp(dr->d_name, ".") == 0 || strcmp(dr->d_name, "..") == 0) {

            continue;

        }

        char file_path[1024] = "";
        strcat(file_path, path);
        strcat(file_path, "/");
        strcat(file_path, dr->d_name);

        if (lstat(file_path, &st) == -1) {

            ERR("lstat");

        }

        if (S_ISREG(st.st_mode)) {

            char author[1024] = "";
            char title[1024] = "";
            char genre[1024] = "";
            scan_book(file_path, author, title, genre);
            
            char target[1024] = "../../";
            strcat(target, file_path);

            char link[1024] = "index/by-visible-title/";
            strcat(link, dr->d_name);

            if (symlink(target, link) == -1) {

                ERR("symlink");

            }

            if (strlen(title) > 0 && title[strlen(title) - 1] == '\n') {

                title[strlen(title) - 1] = '\0';

            }

            if (strlen(title) > 0) {

                if (strlen(title) > 64) {

                    title[64] = '\0';

                }

                target[0] = '\0';
                strcat(target, "../../");
                strcat(target, file_path);

                link[0] = '\0';
                strcat(link, "index/by-title/");
                strcat(link, title);

                if (symlink(target, link) == -1) {

                    ERR("symlink");

                }

            }

            if (strlen(genre) > 0&& genre[strlen(genre) - 1] == '\n') {

                genre[strlen(genre) - 1] = '\0';

            }

            if (strlen(genre) > 0  && strlen(title) > 0 ) {

                

                if (strlen(genre) > 64) {

                    genre[64] = '\0';

                }

                link[0] = '\0';
                strcat(link, "index/by-genre/");
                strcat(link, genre);

                if (mkdir(link, 0755) == -1 && errno != EEXIST) {

                    ERR("mkdir");

                }

                strcat(link, "/");
                strcat(link, title);

                target[0] = '\0';
                strcat(target, "../../../");
                strcat(target, file_path);

                if (symlink(target, link) == -1) {

                    ERR("symlink");

                }

            }

        }

        else if (S_ISDIR(st.st_mode)) {

            scan_library(file_path);

        }

    }

    if (closedir(dir) == -1) {

        ERR("closedir");

    }

}

int main(void) { //usage(argc, argv); 
    
    //scan_book(argv[1]);

    if (mkdir("index", 0755) == -1) {

        ERR("mkdir index");

    }

    if (mkdir("index/by-visible-title", 0755) == -1) {

        ERR("mkdir by-visible-title");

    }

    if (mkdir("index/by-title", 0755) == -1) {

        ERR("mkdir by-title");

    }

    if (mkdir("index/by-genre", 0755) == -1) {

        ERR("mkdir by-genre");

    }

    scan_library("library");

    return EXIT_SUCCESS;

}
