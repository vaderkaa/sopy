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

char *find_field(char *line, char *field)
{
    char *key = NULL;
    char *value = NULL;
    char *to_write_out = NULL;

    // Zabezpieczenie przed Segmentation Fault na pustej linii
    if (strlen(line) > 0 && line[strlen(line) - 1] == '\n')
        line[strlen(line) - 1] = '\0';

    // Rowna sie line sprawia, że znajdujemy pole tylko na początku (ignoruje "title" w "latin_title")
    if ((key = strstr(line, field)) == line)
    {
        // Sprawdzanie na '\0', bo usunęliśmy wyżej '\n'
        if (*(key + strlen(field)) == '\0')
        {
            fprintf(stderr, "No value provided\n");
            exit(EXIT_FAILURE);
        }

        value = line + strlen(field);                            // Wyciagam sobie wartosc
        *(line + strlen(field) - 1) = '\0';                      // Ustawiam sobie koniec slowa zamiast dwukropka
        to_write_out = malloc(strlen(line) + strlen(value) + 3); // Alokuje odpowiednia ilosc pamieci
        if (to_write_out == NULL)
            ERR("malloc");
        strcpy(to_write_out, line);
        strcat(to_write_out, ": ");
        strcat(to_write_out, value);
    }

    return to_write_out;
}

void book_data(char *path)
{
    FILE *stream;
    if ((stream = fopen(path, "r")) == NULL)
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "File does not exist\n");
            exit(EXIT_FAILURE);
        }
        ERR("fopen");
    }

    ssize_t read_bytes = 0;
    char *buf = NULL;
    size_t size = 0;
    char *author = NULL;
    char *title = NULL;
    char *genre = NULL;
    bool a_flag = false;
    bool t_flag = false;
    bool g_flag = false;

    errno = 0;
    while ((read_bytes = getline(&buf, &size, stream)) != -1)
    {
        char *line_copy = strdup(buf);
        char *temp_ptr = NULL; // Wskaznik tymczasowy

        // Uzywamy temp_ptr, żeby nie nadpisywac sobie wczesniej znalezionych zmiennych NULL-ami
        if ((temp_ptr = find_field(line_copy, "author:")) != NULL)
        {
            if (a_flag == true)
            {
                fprintf(stderr, "Too many autors provided\n");
                exit(EXIT_FAILURE);
            }
            author = temp_ptr;
            a_flag = true;
        }
        else if ((temp_ptr = find_field(line_copy, "title:")) != NULL)
        {
            if (t_flag == true)
            {
                fprintf(stderr, "Too many titles provided\n");
                exit(EXIT_FAILURE);
            }
            title = temp_ptr;
            t_flag = true;
        }
        else if ((temp_ptr = find_field(line_copy, "genre:")) != NULL)
        {
            if (g_flag == true)
            {
                fprintf(stderr, "Too many genres provided\n");
                exit(EXIT_FAILURE);
            }
            genre = temp_ptr;
            g_flag = true;
        }
        free(line_copy);
        errno = 0;
    }

    if (errno != 0)
        ERR("getline");

    free(buf);

    // Zamykanie pliku
    if (fclose(stream) != 0)
        ERR("fclose");

    if (author == NULL)
        printf("author: missing!\n");
    else
        printf("%s\n", author);
    free(author);

    if (title == NULL)
        printf("title: missing!\n");
    else
        printf("%s\n", title);
    free(title);

    if (genre == NULL)
        printf("genre: missing!\n");
    else
        printf("%s\n", genre);
    free(genre);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        usage(argc, argv);

    book_data(argv[1]);

    return EXIT_SUCCESS;
}
