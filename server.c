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

// Taille par defaut pour l'allocation dynamique
#define MAX_LINE_LENGTH 1024

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

typedef struct {
  int client_fd;
  int tube_fd;
} client_params;

void handle_request_logs(void *params) {

    pthread_t tid = pthread_self();


    fprintf(stderr,"%sLOG-Thread %lu: DEBUT DU TRAITEMENT PARALLELE DES LOGS\n", colors[tid % 7], tid);
    for (size_t i = 0; i < 5; i++)
    {
        fprintf(stderr,"%sLOG-Thread %lu: Traitement parallele:\n", colors[tid % 7], tid);
    }
    fprintf(stderr,"%sLOG-Thread %lu: FIN DU TRAITEMENT TRAITEMENT PARALLELE DES LOGS\n", colors[tid % 7], tid);


    // Convertir le pointeur de void en ..
    client_params *args = (client_params *) params; 
    int client_fd = args->client_fd;
    int tube_fd = args->tube_fd;

    fprintf(stderr,"%sLOG-Thread : client: %d, Tube lecture: %d \n",colors[tid % 7], client_fd, tube_fd);


    // char message_in[12];
    // // read(tube_fd, message_in, sizeof(message_in));

    // fprintf(stderr,"%s[Log-Thread-Received] : %s\n", colors[tid % 7], message_in);
    
    pthread_exit(NULL);
}

void handle_client(void *params) {

    pthread_t tid = pthread_self();

    client_params *args = (client_params *) params; 
    int client_fd = args->client_fd;

    fprintf(stderr, "%sThread %lu: DEBUT DU TRAITEMENT DE LA REQUETE\n", colors[tid % 7], tid);

    // On alloue un espace memoire par defaut pour la requete et la reponse
    char *buffer = realloc(NULL, MAX_LINE_LENGTH);
    if (buffer == NULL) {
        perror("realloc error");
        pthread_exit(NULL);
    }

    // On reçoit la requete du cleint et on recupere la taille de la requete 
    ssize_t bytes_read = recv(client_fd, buffer, MAX_LINE_LENGTH - 1, 0);

    if (bytes_read == -1) {
        perror("recv error");
        free(buffer);
        pthread_exit(NULL);
    } else if (bytes_read == 0) {
        fprintf(stderr, "%sThread %lu: Client closed connection\n", colors[tid % 7], tid);
        free(buffer);
        pthread_exit(NULL);
    }

    // Limiter le buffer
    buffer[bytes_read] = '\0';

    // Ici on simule un long traitement pour qu'on visualise le speudo parallelisme 
    // dans la console
    for (size_t i = 0; i < 5; i++) {
        fprintf(stderr, "%sThread %lu: En cours de traitement de la requête\n", colors[tid % 7], tid);
    }

    //GET /index.html autre chose inutile
    char *f = buffer + 5; 
    *strchr(f, ' ') = 0;

    // Ouvrir le fichier  en mode lecture seule (O_RDONLY)
    int opened_fd = open(f, O_RDONLY);
    if (opened_fd == -1) {
        perror("open error");
        free(buffer);
        pthread_exit(NULL);
    }

    // Trouver la taille du fichier deùandé par le client
    struct stat statbuf;
    if (fstat(opened_fd, &statbuf) == -1) {
        perror("fstat error");
        close(opened_fd);
        free(buffer);
        pthread_exit(NULL);
    }

    // Reallouer de l'espace en fct. de la taille du fichier à transferer
    buffer = realloc(buffer, statbuf.st_size);
    if (buffer == NULL) {
        perror("realloc error");
        close(opened_fd);
        pthread_exit(NULL);
    }

    // Send the file content using sendfile (check return value for completeness)
    ssize_t sent_bytes = sendfile(client_fd, opened_fd, NULL, statbuf.st_size);
    if (sent_bytes == -1 || sent_bytes != statbuf.st_size) {
        perror("sendfile error");
    }

    // Fermer le fichier et liberer la memoir
    close(opened_fd);
    free(buffer);

    pthread_mutex_unlock(&mutex); 

    fprintf(stderr, "%sThread %lu: FIN DU TRAITEMENT DE LA REQUETE\n", colors[tid % 7], tid);
    close(client_fd);

    int tube_fd = args->tube_fd;
    fprintf(stderr,"%sLOG-Thread : client: %d, Tube ecriture: %d \n",colors[tid % 7], client_fd, tube_fd);
    write(args->tube_fd, "Log message from client thread \n", 33); 

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
        
        // Creation d'un tube
        int tube[2];
        if (pipe(tube) == -1) {
            perror("pipe error");
            exit(1);
        }

        

        pthread_t thread;
        pthread_t thread_log;

        client_params args_client = {client_fd, tube[1]};
        pthread_create(&thread, NULL, (void *)handle_client, (void *)&args_client);

        client_params args_logs = {client_fd, tube[0]};
        pthread_create(&thread_log, NULL, (void *)handle_request_logs, (void *)&args_logs);


        char message_in[33];
        read(tube[0], message_in, sizeof(message_in));
        fprintf(stderr,"%s[Log-Thread-Received] : %s\n", colors[6], message_in);

        close(tube[0]);
        close(tube[1]);

        // pthread_join(thread, NULL);
    }

    close(s);
    return 1;
}




