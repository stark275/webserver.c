#include <stdio.h>
#include <stdlib.h>

int main() {
    // Exécute la commande wget trois fois
    for (int i = 0; i < 3; ++i) {
        system("wget localhost:8006/Index.html");
    }

    printf("Téléchargement terminé !\n");
    return 0;
}
