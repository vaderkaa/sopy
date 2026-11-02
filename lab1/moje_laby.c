#include <errno.h>
#define _XOPEN_SOURCE 500
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#define MAX_PATH 256
#define INGREDIENT_COUNT 30
#define INGREDIENT_LENGTH 10
#define INGREDIENT_MAXAMOUNT 100
#define EXIT_INVENTORY_FAILURE 28

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char* name)
{
    fprintf(stderr, "USAGE: %s directory\n", name);  // katalog ze spisem przepisow
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    if (argc != 2)
        usage(argv[0]);

    srand(time(NULL));
    char* path = argv[1];

    if (access(path, F_OK) == 0)  // istnieje
    {
        if (ENOTDIR == errno)  // to nie katalog
            usage(argv[0]);

        if (access(path, W_OK) != 0 || access(path, X_OK))  // nie ma edycji lub wykonania
        {
            printf("Hehehe...\n");  // gobliny
        }
        else
        {
            printf("For order and wisdom!\n");  // rycerz
        }
    }
    else  // nie istnieje
    {
        if (mkdir(path, S_IRWXU))
            ERR("mkdir");

        DIR* dirp;
        if ((dirp = opendir(path)) == NULL)
            ERR("opendir");

        FILE* recipe;
        umask(0000);

        for (int j = 0; j < 30; j++)
        {
            char* name = malloc(2);
            name[0] = 'a' + rand() % ('z' - 'a' + 1);
            name[1] = '\0';
            char file_path[MAX_PATH] = "";
            strcat(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, name);

            if ((recipe = fopen(file_path, "a+")) == NULL)
                ERR("fopen");

            for (int i = 0; i < 10; i++)
                fprintf(recipe, "%c", 'a' + rand() % ('z' - 'a' + 1));
            fprintf(recipe, ":%d\n", rand() % 100 + 1);

            if (fclose(recipe))
                ERR("fclose");

            free(name);
        }

        chmod(path, S_IRUSR | S_IXUSR | S_IWGRP | S_IXGRP);

        if (closedir(dirp))
            ERR("closedir");
    }
}
