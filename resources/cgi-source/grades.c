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
    free(list[i]);
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
int grades_search(char *name, char *subject) {
    int fd, res = 0;
    char ***data = NULL;
    fd = open(DATA, O_RDONLY, S_IRUSR | S_IWUSR);
    data = get_data(fd);
    for (int i = 0; data[i] != NULL; i++) {
        if (str_cmp(name, data[i][0]) == 0) {
            for (int j = 0; data[0][j] != NULL; j++) {
                if (str_cmp(data[0][j], subject) == 0) {
                    printf("%s's grade on %s is %s\n",
                            data[i][0], data[0][j], data[i][j]);
                    break;
                }
            }
            res = 1;
            break;
        }
    }
    for (int i = 0; data[i] != NULL; i++) {
        free_list(data[i]);
    }
    free(data);
    if (res == 0) {
        printf("there is no such student or subject\n");
    }
    return res;;
}
int main(int argc, char **argv) {
    printf("%d arguments received\n", argc - 1);
    char *name = NULL;
    char *subject = NULL;
    for (int i = 1; i < argc; i += 2) {
        if (str_cmp(argv[i], "name") == 0) {
            name = argv[i + 1];
        }
        if (str_cmp(argv[i], "subject") == 0) {
            subject = argv[i + 1];
        }
    }
    if (subject == NULL || name == NULL) {
        printf("Wrong arguments\n");
    } else {
        grades_search(name, subject);
    }
    return 0;
}

