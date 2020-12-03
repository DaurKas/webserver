#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_SETSOCKETOPT,
    ERR_BIND,
    ERR_LISTEN
};
enum file_type {
    TEXT,
    BINARY,
    MEDIA
};
char ANSWER_OK[] = "HTTP/1.1 200\ncontent-type: %s\ncontent-length: %d\n\n";
char ANSWER_WRONG[] = "HTTP/1.1 404\n\n";
#define OK_LEN sizeof(ANSWER_OK) + 20
#define WRONG_LEN sizeof(ANSWER_WRONG)
char *get_word(char *last_ch, int client_socket) {
    char ch, *word = NULL;
    int size = 1;
    size = read(client_socket, &ch, 1);
    if (size <= 0) {
        exit(0);
    }
    int len = 0;
    word = malloc(sizeof(char));
    while (ch != ' ' && ch != '\n' && ch != '\r') {
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
int init_socket(int port) {
    //open socket, return socket descriptor
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Fail: open socket");
        _exit(ERR_SOCKET);
    }

    //set socket option
    int socket_option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option, (socklen_t) sizeof(socket_option));
    if (server_socket < 0) {
        perror("Fail: set socket options");
        _exit(ERR_SETSOCKETOPT);
    }

    //set socket address
    struct sockaddr_in server_address;
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *) &server_address, (socklen_t) sizeof(server_address)) < 0) {
        perror("Fail: bind socket address");
        _exit(ERR_BIND);
    }

    //listen mode start
    if (listen(server_socket, 5) < 0) {
        perror("Fail: bind socket address");
        _exit(ERR_LISTEN);
    }
    return server_socket;
}
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
void print_header(int client_socket, char *path, int type) {
    char answer[OK_LEN];
    char *str_type;
    struct stat Stat;
    if (type == TEXT) {
        str_type = "text/html";
    } else if (type == BINARY) {
            str_type = "binary";
        } else if(type == MEDIA) {
                str_type = "media";
                }
    stat(path, &Stat);
    snprintf(answer, OK_LEN, ANSWER_OK, str_type, Stat.st_size);
    write(client_socket, answer, strlen(answer));
    return;
}
int get_text(int client_socket, char *path, int type) {
    int fd;
    struct stat Stat;
    stat(path, &Stat);
    fd = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        write(client_socket, "HTTP/1.1 404\n", 13);
        return -1;
    }
    print_header(client_socket, path, type);
    char *buf = malloc(Stat.st_size * sizeof(char));
    read(fd, buf, Stat.st_size);
    write(client_socket, buf, Stat.st_size);
    write(client_socket, "\n", 1);
    free(buf);
    return 0;
}
int run_bin(int client_socket, char *request_path) {
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        int buf = open("buf.txt", O_WRONLY | O_CREAT | O_TRUNC,
                                S_IRUSR | S_IWUSR);
        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(buf, 1);
            close(buf);
            int res = execl(request_path, request_path, NULL);
            if (res < 0) 
                exit(1);
            else 
                exit(0);
        }
        int wstatus;
        wait(&wstatus);
        if (WEXITSTATUS(wstatus) == 1) {
            write(client_socket, ANSWER_WRONG, WRONG_LEN);
        } else {
            get_text(client_socket, "./buf.txt", BINARY);
        }
        exit(0);
    }
    wait(NULL);
    return 0;
}
char *get_ext(char *request_path) {
    char *ext = NULL;
    int len = 0;
    int i;
    for (i = 0; request_path[i] != '.' && request_path[i] != '\0'; i++) {
    }
    for (; request_path[i] != '\0'; i++) {
        ext = realloc(ext, (len + 1) * sizeof(char));
        ext[len] = request_path[i];
        len++;
    }
    if (len > 0) {
        ext = realloc(ext, (len + 1) * sizeof(char));
        ext[len] = '\0';
    } else 
        ext = NULL;
    return ext;
}
int get_type(char *request_path) {
    enum file_type type;
    char *ext = NULL;
    ext = get_ext(request_path);
    if (ext == NULL) {
        type = BINARY;
        return type;
    }
    if (str_cmp(ext, ".c") == 0)
        type = TEXT;
    if (str_cmp(ext, ".html") == 0)
        type = TEXT;
    if (str_cmp(ext, ".jpeg") == 0)
        type = MEDIA;
    free(ext);;
    return type;
}
int wait4connection(int server_socket) {
    struct sockaddr_in client_address;
    socklen_t size = sizeof(client_address);
    int client_socket;
    bzero((char *) &client_address, sizeof(struct sockaddr_in));
    client_socket = accept(server_socket,
                         (struct sockaddr *) &client_address,
                                           &size);
    if (client_socket < 0) {
        printf("FAILED ACCEPT\n");
        return -1;
    } else {
        printf("fd %d connected: %s %d\n", client_socket, 
                                    inet_ntoa((client_address).sin_addr),
                                    ntohs((client_address).sin_port));
    }
    return client_socket;
}
char ***receive_req(int client_socket) {
    char ***request = malloc(sizeof(char**));
    int cnt = 0;
    char last_ch;
    while(1) {
        request = realloc(request, (cnt + 1) * sizeof(char**));
        request[cnt] = get_list(client_socket, &last_ch);
        if (request[cnt] == NULL) 
            break;
        print_cmd(request[cnt]);
        putchar('\n');
        cnt++;
    }
    return request;
}
char *rm_slash(char *request_path) {
    char *new = NULL;
    new = malloc(strlen(request_path));
    memcpy(new, request_path + 1, strlen(request_path) - 1);
    new[strlen(request_path) - 1] = '\0';
    return new;
}
int client_service(int client_socket) {
    char *request_path = NULL, ***request = NULL;
    enum file_type type;
    request = receive_req(client_socket);
    request_path = request[0][1];
    request_path = rm_slash(request_path);
    printf("%s \n", request_path);
    type = get_type(request_path);
    if (type == BINARY) {
       run_bin(client_socket, request_path); 
    } else if (type == TEXT) {
            get_text(client_socket, request_path, TEXT);
        } else if (type == MEDIA) {
        
        }
    for (int i = 0; request[i] != NULL; i++) {
        free_list(request[i]);
    }
    free(request);
    free(request_path);
    printf("close socket\n");
    write(client_socket, "", 1);
    close(client_socket);
    return OK;
}
int main(int argc, char** argv) {
    if (argc != 2) {
        puts("Incorrect args.");
        puts("./server <port>");
        puts("Example:");
        puts("./server 5000");
        return ERR_INCORRECT_ARGS;
    }
    int port = atoi(argv[1]);
    int server_socket = init_socket(port);
    int *client_socket = malloc(sizeof(int));
    int client_num = 0;
    while(1) {
        puts("Wait for connection");
        client_socket = realloc(client_socket, (client_num + 1) * sizeof(int));
        client_socket[client_num] = wait4connection(server_socket);
        if (client_socket[client_num] <= 0) {
            break;
        }
        pid_t pid = fork();
        if (pid == 0) {
            client_service(client_socket[client_num]);
            free(client_socket);
            return OK;
        }
        client_num++;
    }
    free(client_socket);
    return OK;
}
