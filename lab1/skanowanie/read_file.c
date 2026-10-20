// Usuwanie konkretnej linijki z pliku
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
