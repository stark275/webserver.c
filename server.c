#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

#include <sys/stat.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_WHITE   "\x1b[37m"

// Tableau des couleurs pour les threads
const char *colors[] = {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
};  

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_request_logs(int client_fd) {

    pthread_t tid = pthread_self();
    fprintf(stderr,"%sLOG-Thread %lu: DEBUT DU TRAITEMENT PARALLELE DES LOGS\n", colors[tid % 7], tid);
    for (size_t i = 0; i < 100; i++)
    {
        fprintf(stderr,"%sLOG-Thread %lu: Traitement parallele:\n", colors[tid % 7], tid);
    }
    fprintf(stderr,"%sLOG-Thread %lu: FIN DU TRAITEMENT TRAITEMENT PARALLELE DES LOGS\n", colors[tid % 7], tid);
    pthread_exit(NULL);
}

void handle_client(int client_fd) {

    pthread_t tid = pthread_self();
    fprintf(stderr,"%sThread %lu: DEBUT DU TRAITEMENT DE LA REQUETE\n", colors[tid % 7], tid);

    char buffer[256] = {0};
    recv(client_fd, buffer, 256, 0);
    
    // On simule un traitement supplementaire à afficher dans la console
    for (size_t i = 0; i < 100; i++)
    {
        fprintf(stderr,"%sThread %lu: En cours de traitement de la requête\n", colors[tid % 7], tid);
    }

    // GET /index.html .....
    
    char* f = buffer + 5;
    *strchr(f, ' ') = 0;

    pthread_mutex_lock(&mutex);

    int opened_fd = open(f, O_RDONLY);
    sendfile(client_fd, opened_fd, 0, 256);
    close(opened_fd);

    pthread_mutex_unlock(&mutex);


    fprintf(stderr,"%sThread %lu: FIN DU TRAITEMENT DE LA REQUETE\n", colors[tid % 7], tid);
    close(client_fd);

    pthread_exit(NULL);
}

int main() {

    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        AF_INET,
        0x461f, 
        INADDR_ANY
    };
    bind(s,(struct sockaddr *) &addr, sizeof(addr));
    listen(s, 10);
    
    while (1) {
        int client_fd = accept(s, 0, 0);
        if (client_fd < 0) {
            perror("accept error");
            continue;
        }

        pthread_t thread;
        pthread_t thread_log;

        pthread_create(&thread, NULL, (void *)handle_client, (void *)client_fd);
        pthread_create(&thread_log, NULL, (void *)handle_request_logs, (void *)client_fd);

        // pthread_join(thread, NULL);
    }
    close(s);
    return 1;
}




