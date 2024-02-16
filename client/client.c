#include <stdio.h>
#include <stdlib.h>

int main() {
    // Exécute la commande wget trois fois
    for (int i = 0; i < 2; ++i) {
        system("wget localhost:8006/Index.html");
        system("wget localhost:8006/login.html");

    }

    printf("Téléchargement terminé !\n");
    return 0;
}
