#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATHS 256

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE: %s -p <path> -e <extension> -d <depth> -o\n", pname);
    exit(EXIT_FAILURE);
}

void scan_dir(char *path, char *extension, int depth, FILE *out)
{
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;

    fprintf(out, "path: %s\n", path);
    if ((dirp = opendir(path)) == NULL)
        ERR("opendir");

    do
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
        {
            // omijamy foldery . i ..
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
                continue;

            // tworzymy sciezke do pliku
            char new_path[PATH_MAX];
            snprintf(new_path, sizeof(new_path), "%s/%s", path, dp->d_name);

            if (lstat(new_path, &filestat))
            {
                perror("lstat");
                continue; // jesli blad to nie zabijamy programu
            }

            // czy to NIE katalog?
            if (!S_ISDIR(filestat.st_mode))
            {
                // znajduje ostatnia kropke
                char *dot = strrchr(dp->d_name, '.');
                if (extension == NULL || (dot != NULL && strcmp(dot + 1, extension) == 0))
                    fprintf(out, "%s %ld\n", dp->d_name, filestat.st_size);
            }
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");

    rewinddir(dirp); // zaczynamy od poczatku

    if (depth > 1)
    {
        do
        {
            errno = 0;
            if ((dp = readdir(dirp)) != NULL)
            {
                if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
                    continue;

                // tworzymy sciezke do pliku
                char new_path[PATH_MAX];
                snprintf(new_path, sizeof(new_path), "%s/%s", path, dp->d_name);

                // juz bez bledow, juz je wypisalismy
                if (lstat(new_path, &filestat) == 0) // 0 - sukces
                {
                    if (S_ISDIR(filestat.st_mode))
                        scan_dir(new_path, extension, depth - 1, out);
                }
            }
        } while (dp != NULL);
    }

    if (errno != 0)
        ERR("readdir");

    if (closedir(dirp))
        ERR("closedir");
}

int main(int argc, char **argv)
{
    int c;
    char *paths[MAX_PATHS];
    int path_count = 0;
    char *extension = NULL;
    bool flag = false;
    int depth = 1;
    FILE *output_file = stdout;

    while ((c = getopt(argc, argv, ":p:e:d:o")) != -1)
    {
        switch (c)
        {
        case 'p':
            if (path_count < MAX_PATHS)
                paths[path_count++] = optarg;
            break;
        case 'e':
            if (flag == false)
            {
                extension = optarg;
                flag = true;
            }
            else
                usage(argv[0]);
            break;
        case 'd':
            errno = 0;
            char *endptr;
            long val = strtol(optarg, &endptr, 10);
            if (errno == ERANGE || *endptr != '\0' || val < 1)
                usage(argv[0]);
            depth = (int)val;
            break;
        case 'o':
            char *out = getenv("L1_OUTPUTFILE");
            if (out != NULL)
            {
                if ((output_file = fopen(out, "w")) == NULL)
                    ERR("fopen");
            }
            else
            {
                fprintf(stderr, "Zmienna nie istnieje\n");
                exit(EXIT_FAILURE);
            }
            break;
        case ':': // brak argumentu
        case '?': // zla flaga
        default:
            usage(argv[0]);
        }
    }

    if (optind < argc)
        usage(argv[0]);

    for (int i = 0; i < path_count; i++)
        scan_dir(paths[i], extension, depth, output_file);

    if (output_file != stdout)
        fclose(output_file);

    return EXIT_SUCCESS;
}