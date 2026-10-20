​/*Działa to w następujących krokach:
​Tworzenie pliku tymczasowego: Generuje ścieżkę do pliku o nazwie temp wewnątrz katalogu środowiska i otwiera go do zapisu (fp_temp).
​Przepisywanie zawartości: Otwiera oryginalny plik requirements (fp) i czyta go linijka po linijce za pomocą getline. Pętla przepisuje do pliku tymczasowego wszystkie linie, opijając tylko tę jedną dotyczącą usuwananego pakietu (rev_name).
​Usuwanie fizycznego pliku pakietu: Skleja ścieżkę do "fałszywego" pliku reprezentującego zainstalowany pakiet (np. moje_srodowisko/numpy) i fizycznie usuwa go z dysku za pomocą funkcji unlink(remove_path).
​Podmiana plików: Zamyka oba pliki, następnie usuwa stary, nieaktualny plik requirements (unlink(path)) i zmienia nazwę pliku tymczasowego na requirements (rename(temp_path, path)).*/

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
