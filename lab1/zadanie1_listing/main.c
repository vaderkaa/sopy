#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE: %s -p PATH -o FILE\n", pname);
    exit(EXIT_FAILURE);
}

void listing(char *path, FILE *out)
{
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;

    fprintf(out, "ŚCIEŻKA:\n");
    fprintf(out, "%s\n", path);
    if ((dirp = opendir(path)) == NULL)
    {
        perror(path);
        return;
    }
    fprintf(out, "LISTA PLIKÓW:\n");

    do
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
        {
            if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))
            {
                char new_path[PATH_MAX];
                snprintf(new_path, sizeof(new_path), "%s/%s", path, dp->d_name);

                if (lstat(new_path, &filestat) == -1)
                {
                    perror("lstat"); // brak uprawnien
                    continue;
                }
                fprintf(out, "%s %ld\n", dp->d_name, filestat.st_size);
            }
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");

    if (closedir(dirp))
        ERR("closedir");
}

int main(int argc, char **argv)
{
    if (argc < 2)
        usage(argv[0]);

    int c;
    char *path = NULL;
    FILE *out_stream = stdout;
    bool flag = false;

    while ((c = getopt(argc, argv, ":p:o:")) != -1)
    {
        if (c == 'o')
        {
            if (flag == true)
                usage(argv[0]);

            if ((out_stream = fopen(optarg, "w")) == NULL)
                ERR("fopen");

            flag = true;
        }
    }

    if (optind < argc) // podano jakies smieci
        usage(argv[0]);

    optind = 0;
    while ((c = getopt(argc, argv, ":p:o:")) != -1)
    {
        switch (c)
        {
        case 'p':
            path = optarg;
            listing(path, out_stream);
            break;
        case 'o':
            break;
        case ':': // brak argumentu po fladze
        case '?': // zla flaga
        default:
            usage(argv[0]);
        }
    }

    // zapomnialam znowu o tym
    if (out_stream != stdout)
        if (fclose(out_stream))
            ERR("fclose");

    return EXIT_SUCCESS;
}