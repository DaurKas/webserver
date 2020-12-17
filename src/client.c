#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

char ANSWER_OK[] = "HTTP/1.1 200\n";
char ANSWER_WRONG[] = "HTTP/1.1 404\n";
char CONTENT_IMG[] = "content-type: image\n";
#define OK_LEN sizeof(ANSWER_OK)
#define WRONG_LEN sizeof(ANSWER_WRONG)
int img_num = 1;
enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_CONNECT
};
enum requests {
    GET,
    POST
};

int init_socket(const char *ip, int port) {
    //open socket, result is socket descriptor
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Fail: open socket");
        _exit(ERR_SOCKET);
    }
    //connection
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(ip);
    if (connect(server_socket, (struct sockaddr*) &sin, (socklen_t) sizeof(sin)) < 0) {
        perror("Fail: connect");
        return -1;
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
char *recieve_line(int server) {
    char ch = 1, *line = NULL;
    int len = 1;
    int res = read(server, &ch, 1);
    if (res <= 0) 
        return NULL;
    line = malloc(sizeof(char));
    line[0] = ch;
    while (ch != '\n') {
        res = read(server, &ch, 1);
        if (res <= 0)
            break;
        line = realloc(line, (len + 1) * sizeof(char));
        line[len] = ch;
        len++;
    }
    line = realloc(line, (len + 1) * sizeof(char));
    line[len] = '\0';
    return line;
}
int recieve_header(int server) {
    char *line = NULL;
    char *content_type = NULL;
    char *content_len = NULL, *buf = NULL;
    int is_image = 0;
    line = recieve_line(server);
    puts(line);
    if (str_cmp(line, ANSWER_OK) == 0) {
        content_type = recieve_line(server);
        content_len = recieve_line(server);
        buf = recieve_line(server);
        printf("%s%s", content_type, content_len);
        if (str_cmp(content_type, CONTENT_IMG) == 0) {
            is_image = 1;
        }
        free(buf);
        free(content_type);
        free(content_len);
    }
    free(line);
    return is_image;
}
int recieve_img(int server, char *filename) {
    int fd, size = 1;
    char ch = 1;
    char name[] = "image%d%s";
    size_t name_len = (strlen(name) + 10) * sizeof(char);
    char *ext = get_ext(filename);
    char *new_file = malloc(name_len);
    snprintf(new_file,name_len, name, img_num, ext);
    fd = open(new_file, O_WRONLY | O_CREAT | O_TRUNC,
                                S_IRUSR | S_IWUSR);
    img_num++;
    free(ext);
    free(new_file);
    while(1) {
        size = read(server, &ch, 1);
        if (size <= 0) {
            return 0;
        }
        write(fd, &ch, 1);
    }
    return 0;
}
void reciever(int server, char *name) {
    char ch = 1;
    int size = 1, is_img = 0;
    is_img = recieve_header(server);
    if (is_img) {
        recieve_img(server, name);
    } else {
        while(ch > 0) {
            size = read(server, &ch, 1);
            if (size <= 0) {
                return;
            }
            putchar(ch);
        }
    }
}
char *get_text(char end_ch) {
    int len = 0;
    char ch = 0;
    char *text = malloc(sizeof(char));
    do {
        ch = getchar();
        text = realloc(text, (len + 1) * sizeof(char));
        text[len] = ch;
        len++;
    } while (ch != '\n' && ch != end_ch);
    text[len - 1] = '\0';
    return text;
}
void send_get_request(int server, char *filename, char *host) {
    char *message = malloc((15 + strlen(filename)) * sizeof(char));
    char *str_host = "host: %s", *host_send = NULL;
    host_send = malloc(strlen(str_host) + strlen(host));
    sprintf(host_send, str_host, host);
    sprintf(message, "GET %s HTTP/1.1\n", filename);
    write(server, message, strlen(message));
    write(server, host_send, strlen(host_send));
    write(server, "\n\n", 2);
    free(message);
    return;
}
void send_post_request(int server, char *filename, char *host) {
    int pos = 0;
    while (filename[pos] != '%') {
        pos++;
    }
    size_t msg_size = (17 + pos) * sizeof(char); 
    char *message = malloc(msg_size), *new_name = malloc((pos + 1) * sizeof(char));
    char *str_host = "host: %s\n", *host_send = NULL;
    host_send = malloc(strlen(str_host) + strlen(host));
    memcpy(new_name, filename, pos);
    new_name[pos] = '\0';
    sprintf(host_send, str_host, host);
    sprintf(message, "POST %s HTTP/1.1\n", new_name);
    write(server, message, strlen(message));
    write(server, host_send, strlen(host_send));
    write(server, filename + pos + 1, strlen(filename) - pos);
    write(server, "\n\n", 2);
    free(message);
    return;
}
char *add_slash(char *filename) {
    char *new = NULL;
    new = malloc((strlen(filename) + 2) * sizeof(char));
    new[0] = '/';
    memcpy(new + 1, filename, strlen(filename));
    new[strlen(filename) + 1] = '\0';
    return new;
}
int get_request_type(char *name) {
    for (int i = 0; name[i] != '\0'; i++) {
        if (name[i] == '%') {
            return POST;
        }
    }
    return GET;
}
int request(char **name) {
    char *str_port = NULL;
    int port;
    char *ip = NULL, *filename = NULL;
    int server, request_type = GET;
    ip = get_text(':');
    str_port = get_text('/');
    port = atoi(str_port);
    server = init_socket(ip, port);
    filename = get_text(' ');
    request_type = get_request_type(filename);
    *name = filename;
    filename = add_slash(filename);
    if (server < 0) {
        printf("Connection error\n");
        printf("%s %s\n", ip, str_port);
        return -1;
    }
    if (request_type == GET) {
        send_get_request(server, filename, ip);
    } else if (request_type == POST) {
            send_post_request(server, filename, ip);        
        }
    free(ip);
    free(str_port);
    free(filename);
    return server;
}
int main() {
    while (1) {
        int server;
        char *name = NULL;
        server = request(&name);
        if (server < 0) { 
            close(server);
            continue;
        }
        reciever(server, name);
        free(name);
        close(server);
    }
    return OK;
}
