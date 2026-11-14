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
char *join_paths(const char *path1, const char *path2)
{
    char *res;
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
        res = malloc(strlen(path1) + strlen(path2) + 2); // additional space for "/"
        if (!res)
            ERR("malloc");
        strcpy(res, path1);
        res[l1] = '/';
        res[l1 + 1] = 0;
    }
    return strcat(res, path2);
}

void usage(int argc, char **argv)
{
    (void)argc;
    fprintf(stderr, "USAGE: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
}

void get_book_data(char *path)
{
    FILE *stream;
    if ((stream = fopen(path, "r")) == NULL)
        ERR("fopen");

    char *buf = NULL;
    size_t size = 0;
    ssize_t read_bytes = 0;
    errno = 0;

    char *separator = NULL;
    char *author = NULL;
    char *title = NULL;
    char *genre = NULL;

    while ((read_bytes = getline(&buf, &size, stream)) != -1)
    {
        if ((separator = strchr(buf, ':')) == NULL)
        {
            errno = 0;
            continue;
        }

        // Zamiana entera na koncu na \0
        if (read_bytes > 0 && buf[read_bytes - 1] == '\n')
            buf[read_bytes - 1] = '\0';

        // Po dwukropku nic nie ma
        if (*(separator + 1) == '\0')
        {
            errno = 0;
            continue;
        }

        *(separator) = '\0';
        if (strcmp("author", buf) == 0)
            author = strdup(separator + 1); // Samo alokuje pamiec malloc
        else if (strcmp("title", buf) == 0)
            title = strdup(separator + 1);
        else if (strcmp("genre", buf) == 0)
            genre = strdup(separator + 1);

        errno = 0;
    }
    if (errno != 0)
        ERR("getline");
    free(buf);

    if (fclose(stream))
        ERR("fclose");

    printf("author: %s\n", author == NULL ? "missing!" : author);
    printf("title: %s\n", title == NULL ? "missing!" : title);
    printf("title: %s\n", genre == NULL ? "missing!" : genre);
    free(author);
    free(title);
    free(genre);
}

int main(int argc, char **argv)
{
    if (argc < 2)
        usage(argc, argv);

    get_book_data(argv[1]);
}
