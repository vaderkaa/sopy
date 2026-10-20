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

void scan_dir(char* path) {

    DIR* dir;
    struct dirent* dp;
    struct stat st;

    if ((dir = opendir(path)) == NULL) {

        ERR("opendir");

    }

    errno = 0;

    while ((dp = readdir(dir)) != NULL) {

        if ((strcmp(dp->d_name, ".")) == 0 || strcmp(dp->d_name, "..") == 0) {

            continue;

        }

        char file_path[1024] = "";
        strcat(file_path, path);
        strcat(file_path, "/");
        strcat(file_path, dp->d_name);

        if (lstat(file_path, &st) == -1) {

            ERR("lstat");

        }

        if (S_ISREG(st.st_mode)) {

          /*  
          char* s_k = strstr(file_path, "ksiegowosc");

            if (s_k != NULL) {

                char link[1024] = "archiwum/ksiegowe/";
                strcat(link, dp->d_name);

                char target[1024] = "../../";
                strcat(target, file_path);

                if (symlink(target, link) == -1) {

                    ERR("symlink");

                }

            */

            }

            /*
      
            char* s_z = strstr(file_path, "zarzad");

            if (s_z != NULL) {

                char link[1024] = "archiwum/zarzadowe/";
                strcat(link, dp->d_name);

                char target[1024] = "../../";
                strcat(target, file_path);

                if (symlink(target, link) == -1) {

                    ERR("symlink");

                }

            }

            */

        }

        else if (S_ISDIR(st.st_mode)) {

            scan_dir(file_path);

        }

        errno = 0;

    }

    if (errno != 0) {

        ERR("readdir");

    }

    if (closedir(dir) == -1) {

        ERR("closedir");

    }

}

int main(int argc, char** argv) {

  /*  
  if (mkdir("archiwum", 0755) == -1 && errno != EEXIST) {

        ERR("mkdir");

    }

    if (mkdir("archiwum/ksiegowe", 0755) == -1 && errno != EEXIST) {

        ERR("mkdir");

    }

    if (mkdir("archiwum/zarzadowe", 0755) == -1 && errno != EEXIST) {

        ERR("mkdir");

    }
*/

    scan_dir("biuro");

}
