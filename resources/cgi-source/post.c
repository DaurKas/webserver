#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
const char DATA[] = "resources/database/grades.csv";
int str_cmp(char *str1, char *str2) {  //  0 - eq, 1 - s1 < s2, 2 - s1 > s2
    int i = -1;
    do {
        i++;
        if (str1[i] < str2[i])
            return 1; 
        if (str1[i] > str2[i])
            return 2;
    } while (str1[i] && str2[i]);
    return 0;
}
char *get_word(char *last_ch, int client_socket) {
    char ch, *word = NULL;
    int size = 1;
    size = read(client_socket, &ch, 1);
    if (size <= 0) {
        exit(0);
    }
    int len = 0;
    word = malloc(sizeof(char));
    while (ch != ' ' && ch != '\n' && ch != ',') {
        word = realloc(word, (len + 1) * sizeof(char));
        word[len] = ch;
        len++;
        size = read(client_socket, &ch, 1);
        if (size <= 0) {
            exit(0);
        }           
    }
    *last_ch = ch;
    word = realloc(word, (len + 1) * sizeof(char));
    word[len] = '\0';
    return word;
}
char **get_list(int client_socket, char *last) {
    char **list = NULL, last_ch;
    int count = 1;
    list = malloc(sizeof(char*));
    if (!list )
        return NULL;
    list[0] = get_word(&last_ch, client_socket);
    if (list[0][0] == '\0') {
        free(list[0]);
        free(list);
        return NULL;
    }
    while (last_ch != '\n') {
        list = realloc(list, (count + 1) * sizeof(char*));
        list[count] = get_word(&last_ch, client_socket);
        count++;
    }
    list = realloc(list, (count + 1) * sizeof(char*));
    list[count] = NULL;
    *last = last_ch;
    return list;
}
void free_list(char **list) {
    int i;
    if (list == NULL)
        return;
    for (i = 0; list[i] != NULL; i++) {
        free(list[i]);
    }
    free(list);
    return;
}
void print_cmd(char **cmd) {
    int i = 0;
    for (i = 0; cmd[i] != NULL; i++) {
        printf("%s ", cmd[i]);
    }
}
char ***get_data(int fd) {
    char ***data = malloc(sizeof(char**));
    int cnt = 0;
    char last_ch;
    while(1) {
        data = realloc(data, (cnt + 1) * sizeof(char**));
        data[cnt] = get_list(fd, &last_ch);
        if (data[cnt] == NULL) 
            break;
        cnt++;
    }
    return data;
}
int write_data(int fd, char ***new_data) {
    for (int i = 0; new_data[i] != NULL; i++) {
        for (int j = 0; new_data[i][j] != NULL; j++) {
            if (j != 0)
                write(fd, ",", 1);
            write(fd, new_data[i][j], strlen(new_data[i][j]));
        }
        write(fd, "\n", 1);
    }
    write(fd, "\n", 1);
    return 0;
}
int grades_post(char *file, char *name, char *subject, char *new_grade) {
    int fd, res = 0;
    char ***data = NULL;
    fd = open(file, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) 
        printf("GG\n");
    data = get_data(fd);
    for (int i = 0; data[i] != NULL; i++) {
        if (str_cmp(name, data[i][0]) == 0) {
            for (int j = 0; data[0][j] != NULL; j++) {
                if (str_cmp(data[0][j], subject) == 0) {
                    printf("%s's  new grade on %s is %s\n",
                            data[i][0], data[0][j], new_grade);
                    //data[i][j] = realloc(data[i][j], strlen(new_grade));
                    //memcpy(data[i][j], new_grade, strlen(new_grade));
                    data[i][j][0] = new_grade[0];
                    break;
                }
            }
            res = 1;
            break;
        }
    }
    close(fd);
    int out = open(file, O_WRONLY, S_IRUSR | S_IWUSR);
    write_data(out, data);
    for (int i = 0; data[i] != NULL; i++) {
        free_list(data[i]);
    }
    free(data);
    if (res == 0) {
        printf("there is no such student or subject\n");
    }
    return res;;
}
int args_check(int argc, char *name, char *subject,
                            char *new_grade, char *file) {
    int num_check = ((argc - 1) % 2 != 0);
    int null_check = (subject == NULL || name == NULL
                        || file == NULL || new_grade == NULL);
    return num_check || null_check;
}
int main(int argc, char **argv) {
    printf("%d arguments received\n", argc - 1);
    char *name = NULL, *file = NULL;
    char *subject = NULL;
    char *new_grade = NULL;
    for (int i = 1; i < argc; i += 2) {
        if (str_cmp(argv[i], "file") == 0) {
            file = argv[i + 1];
        }
        if (str_cmp(argv[i], "name") == 0) {
            name = argv[i + 1];
        }
        if (str_cmp(argv[i], "subject") == 0) {
            subject = argv[i + 1];
        }
        if (str_cmp(argv[i], "grade") == 0) {
            new_grade = argv[i + 1];
        }
    }
    if (args_check(argc, name, subject, new_grade, file)) {
        printf("Wrong arguments\n");
    } else {
        printf("%s %s %s\n", name, subject, new_grade);
        grades_post(file, name, subject, new_grade);
    }
    return 0;
}

