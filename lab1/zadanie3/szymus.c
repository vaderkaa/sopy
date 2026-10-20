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
#include <getopt.h>
#include <stdbool.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname) {
    
    fprintf(stderr, "USAGE:%s -p PATH -o FILE\n", pname);
    exit(EXIT_FAILURE);

}

int main(int argc, char** argv) {

    int opt;
    int create = 0;
    char* env_name = NULL;
    char* package = NULL;
    char name[1024] = "";
    char version[1024] = "";
    bool install = false;
    char action[2] = "w";
    char* arr[argc];
    int index = 0;
    char* rev_name = NULL;
    bool remove = false;

    while ((opt = getopt(argc, argv, "cv:i:r:")) != -1) {

        switch (opt) {

            case 'c':

                create = 1;
                break;

            case 'v':

                env_name = optarg;
                arr[index++] = optarg;
                break;

            case 'i':

                package = optarg;
                install = true;
                
                char* s = strchr(package, '=');

                if (s != NULL) {

                    int len = s - package;
                    
                    strncpy(name, package, len);
                    name[len] = '\0';

                    strcpy(version, s + 2);

                }
                
                break;

            case 'r':

                rev_name = optarg;
                remove = true;
                break;

        }

    }

    if (create == 1) {

        if (env_name == NULL) {

            env_name = argv[argc - 1];

        }

        if (mkdir(env_name, 0755) == -1) {

            ERR("mkdir");

        }

        char path[1024] = "";
        strcat(path, env_name);
        strcat(path, "/");
        strcat(path, "requirements");

        FILE* fp;

        if ((fp = fopen(path, "w")) == NULL) {

            ERR("fopen");

        }

        if (fclose(fp) == -1) {

            ERR("fclose");

        }

    }

    else {

        int i;

        for (i = 0; i < index; i++) {

            char path[1024] = "";
            strcat(path, arr[i]);
            strcat(path, "/");
            strcat(path, "requirements");

            if (install == true) { action[0] = 'a'; }

            if (install == true) {

                FILE* fp;

                if ((fp = fopen(path, action)) == NULL) {

                    ERR("fopen");

                }


                FILE* fp_in;

                char package_path[1024] = "";

                strcat(package_path, arr[i]);
                strcat(package_path, "/");
                strcat(package_path, name);

                if ((fp_in = fopen(package_path, "w")) == NULL) {

                    ERR("fopen");

                }

                int k;
                for (k = 0; k < 10; k++) {

                    fprintf(fp_in, "%d ", rand() % 2);

                }

                fprintf(fp, "%s ", name);
                fprintf(fp, "%s\n", version);

                if (fclose(fp_in) == -1) {

                    ERR("fclose");

                }

                if (chmod(package_path, 0444) == -1) {

                    ERR("chmod");

                }

                if (fclose(fp) == -1) {

                    ERR("fclose");

                }

            }

            if (remove == true) {

                FILE* fp_temp;

                char temp_path[1024] = "";
                strcat(temp_path, arr[i]);
                strcat(temp_path, "/");
                strcat(temp_path, "temp");


                if ((fp_temp = fopen(temp_path, "w")) == NULL) {

                    ERR("fopen");

                }

                FILE* fp;

                if ((fp = fopen(path, "r")) == NULL) {

                    ERR("fopen");

                }

                char* buf = NULL;
                size_t size = 0;
                ssize_t ret;

                while ((ret = getline(&buf, &size, fp)) != -1) {

                    if (strncmp(buf, rev_name, strlen(rev_name)) == 0 && buf[strlen(rev_name)] == ' ')  {

                        continue;

                    }


                    else {

                        fprintf(fp_temp, "%s", buf);

                    }

                }

                free(buf);

                char remove_path[1024] = "";

                strcat(remove_path, arr[i]);
                strcat(remove_path, "/");
                strcat(remove_path, rev_name);

                if (unlink(remove_path) == -1) {

                    ERR("unlink");

                }

                if (fclose(fp_temp) == -1) {

                    ERR("fclose");

                }

                if (fclose(fp) == -1) {

                    ERR("fclose");

                }

                unlink(path);
                rename(temp_path, path);

            }

        }

    }

    return EXIT_SUCCESS;

}
