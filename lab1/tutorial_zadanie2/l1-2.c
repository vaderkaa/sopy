#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH 101

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void scan_dir()
{
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;
    int dirs = 0, files = 0, links = 0, other = 0;
    if ((dirp = opendir(".")) == NULL)
        ERR("opendir");
    do // LEPIEJ TAK, BO JESLI ZROBIE CONTINUE, BO CHCE OMINAC UKRYTE PLIKI, TO SIE ZEPSUJE
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) // UWAGA, MUSI BYC NAWIAS
        {
            if (lstat(dp->d_name, &filestat))
                ERR("lstat");
            if (S_ISDIR(filestat.st_mode))
                dirs++;
            else if (S_ISREG(filestat.st_mode))
                files++;
            else if (S_ISLNK(filestat.st_mode))
                links++;
            else
                other++;
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");
    if (closedir(dirp))
        ERR("closedir");
    printf("Files: %d, Dirs: %d, Links: %d, Other: %d\n", files, dirs, links, other);
}

int main(int argc, char **argv)
{
    char path[MAX_PATH];
    if (getcwd(path, MAX_PATH) == NULL) // current working directory
        ERR("getcwd");
    for (int i = 1; i < argc; i++) // argv[0] to program
    {
        if (chdir(argv[i]))
            ERR("chdir");
        printf("%s:\n", argv[i]);
        scan_dir();      // wywoluje opendir("."), ktory zmienilismy przez chdir
        if (chdir(path)) // wracamy do folderu startowego
            ERR("chdir");
    }
    return EXIT_SUCCESS;
}