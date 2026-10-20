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
