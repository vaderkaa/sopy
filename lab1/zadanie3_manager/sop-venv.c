#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE: %s\n", pname);
    fprintf(stderr, "   -c [ENVIRONMENT_NAME]\n");
    fprintf(stderr, "   -v <ENVIRONMENT_NAME>\n");
    fprintf(stderr, "   -i <PACKAGE_NAME>==<VERSION>\n");
    exit(EXIT_FAILURE);
}

void create(char *env_name)
{
    // Tworzenie katalogu
    if (mkdir(env_name, S_IRWXU))
        ERR("mkdir");

    // Dodanie pliku requirements
    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s/%s", env_name, "requirements");

    FILE *requirements;
    if ((requirements = fopen(file_path, "w")) == NULL)
        ERR("fopen");
    if (fclose(requirements))
        ERR("fclose");
}

void install_package(char *package_name, char *package_version, char *env_name, char *program_name)
{
    // Chce miec file descriptor katalogu, zeby uzywac openat
    int dir_fd;
    if ((dir_fd = open(env_name, O_RDONLY | O_DIRECTORY)) == -1)
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "Environment %s does not exist\n", env_name);
            usage(program_name);
        }
        ERR("open");
    }

    // Tworze plik pakietu
    int fd;
    if ((fd = openat(dir_fd, package_name, O_CREAT | O_EXCL | O_WRONLY, 0444)) == -1)
    {
        if (errno == EEXIST) // Nie ubijam programu
            fprintf(stderr, "ERROR: Package %s in %s already exists\n", package_name, env_name);
        else
            ERR("openat");
    }
    else // Udalo sie stworzyc plik
    {
        int letters_count = (rand() % 50) + 1; // Aby tablica nie miala dlugosci 0
        char buffer[letters_count];
        for (int i = 0; i < letters_count; i++)
            buffer[i] = 'a' + rand() % 26;
        if (write(fd, buffer, letters_count) == -1)
            ERR("write");

        // Edytuje requirements
        int req;
        if ((req = openat(dir_fd, "requirements", O_WRONLY | O_APPEND)) == -1)
            ERR("openat");
        dprintf(req, "%s %s\n", package_name, package_version);

        if (close(req))
            ERR("close");

        if (close(fd))
            ERR("close");
    }

    if (close(dir_fd))
        ERR("close");
}

void remove_package(char *package_name, char *env_name, char *program_name)
{
    // Chce miec file descriptor katalogu, zeby uzywac openat
    int dir_fd;
    if ((dir_fd = open(env_name, O_RDONLY | O_DIRECTORY)) == -1)
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "Environment %s does not exist\n", env_name);
            usage(program_name);
        }
        ERR("open");
    }

    if (faccessat(dir_fd, package_name, F_OK, 0) == -1)
    {
        fprintf(stderr, "ERROR: Package does not exist\n");
        usage(program_name);
    }

    // Otwieram requirements
    int req;
    if ((req = openat(dir_fd, "requirements", O_RDONLY)) == -1)
        ERR("openat");

    // Tworze requirements_temp
    int req_temp;
    if ((req_temp = openat(dir_fd, "requirements_temp", O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1)
        ERR("openat");

    // Przeksztalcam file descriptor na strumien, zeby uzyc getline
    FILE *stream_req;
    if ((stream_req = fdopen(req, "r")) == NULL)
        ERR("fdopen");

    char *buf = NULL;
    size_t size = 0;
    ssize_t read_bytes;

    errno = 0;
    while ((read_bytes = getline(&buf, &size, stream_req)) != -1)
    {
        // strlen - dlugosc stringa bez znaku konca \0
        // sizeof - dlugosc ze znakiem konca lub ilosc zarezerwowanej pamieci
        // strlen zawsze na wskaznikach na char!!!
        if (strncmp(buf, package_name, strlen(package_name)) == 0 && buf[strlen(package_name)] == ' ')
        {
            errno = 0;
            continue;
        }

        // getline zasysa wszystko, razem ze znakiem \n, wiec juz go nie daje
        dprintf(req_temp, "%s", buf);
        errno = 0;
    }
    if (errno != 0)
        ERR("getline");

    free(buf); // !!!BO GETLINE POTAJEMNIE WYWOLUJE MALLOC I REALLOC!!!

    if (close(req_temp))
        ERR("close");
    if (fclose(stream_req))
        ERR("fclose");
    // Juz bez zamykania req, wystarczy zamknac wysokopoziomowy stream_req

    // 0 - zwykly plik
    if (unlinkat(dir_fd, package_name, 0) == -1)
        ERR("unlinkat");

    if (renameat(dir_fd, "requirements_temp", dir_fd, "requirements") == -1)
        ERR("renameat");

    if (close(dir_fd))
        ERR("close");
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    int c;
    bool c_flag = false;

    char *env_names[argc];
    int env_count = 0;
    bool i_flag = false;

    // Trzeba to zadeklarowac tutaj, bo tak to po case przestanie istniec
    char *package = NULL;
    char *package_name = NULL;
    char *package_version = NULL;

    bool r_flag = false;

    while ((c = getopt(argc, argv, ":cv:i:r:")) != -1)
    {
        switch (c)
        {
        case 'c':

            c_flag = true;
            break;

        case 'v':

            env_names[env_count++] = optarg; // env_count i tak nie zblizy sie do argc
            break;

        case 'i':

            if (i_flag == true)
            {
                fprintf(stderr, "ERROR: Cannot install two packages at once\n");
                usage(argv[0]);
            }

            // Parsuje stringi
            package = optarg;
            char *separator = strstr(package, "==");
            package_name = package;
            if (separator != NULL)
            {
                *separator = '\0';
                package_version = separator + 2;

                if (*package_version == '\0')
                {
                    fprintf(stderr, "Package version is missing\n");
                    usage(argv[0]);
                }
            }
            else
            {
                fprintf(stderr, "ERROR: Wrong package format\n");
                usage(argv[0]);
            }

            i_flag = true;
            break;

        case 'r':

            if (r_flag == true)
            {
                fprintf(stderr, "ERROR: Cannot remove two packages at once\n");
                usage(argv[0]);
            }
            if (i_flag == true)
            {
                fprintf(stderr, "ERROR: Cannot install and remove package at once\n");
                usage(argv[0]);
            }

            package_name = optarg;

            r_flag = true;
            break;

        case ':': // Brak wymaganego argumentu

        case '?': // Zla flaga

        default:

            fprintf(stderr, "ERROR: No argument provided or incorrect flag\n");
            usage(argv[0]);
        }
    }

    // Jesli podano c bez v
    if (c_flag == true && env_count == 0 && optind < argc)
    {
        env_names[0] = argv[optind];
        env_count = 1;
        optind++;
    }

    // Jesli nie, to na pewno sa jakies smieciowe argumenty
    if (optind < argc)
    {
        fprintf(stderr, "ERROR: Unexpected trailing arguments\n");
        usage(argv[0]);
    }

    if (c_flag == true)
    {
        if (env_count > 1)
        {
            fprintf(stderr, "ERROR: Cannot create multiple environments at once\n");
            usage(argv[0]);
        }

        // Jesli podano c bez/z v z argumentem
        if (env_count == 1)
            create(env_names[0]);
        else
        {
            fprintf(stderr, "Environment name is not specified\n");
            usage(argv[0]);
        }
    }
    else if (i_flag == true)
    {
        if (env_count == 0)
        {
            fprintf(stderr, "Environment name is not specified\n");
            usage(argv[0]);
        }

        for (int i = 0; i < env_count; i++)
            install_package(package_name, package_version, env_names[i], argv[0]);
    }
    else if (r_flag == true)
    {
        if (env_count == 0)
        {
            fprintf(stderr, "Environment name is not specified\n");
            usage(argv[0]);
        }

        for (int i = 0; i < env_count; i++)
            remove_package(package_name, env_names[i], argv[0]);
    }

    return EXIT_SUCCESS;
}