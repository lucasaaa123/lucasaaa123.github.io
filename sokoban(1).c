#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define SOKOBAN '@'
#define MUR '#'
#define CIBLE '.'
#define CAISSE '$'
#define VIDE ' '
#define CAISSECIBLE '*'
#define SOKOBANCIBLE '+'
#define HAUT 'z'
#define GAUCHE 'q'
#define BAS 's'
#define DROITE 'd'
#define ABANDON 'x'
#define RECOMMENCER 'r'
#define ACCEPTER 'o'
#define ZOOMER '+'
#define DEZOOMER '-'
#define HAUT2 'h'
#define GAUCHE2 'g'
#define BAS2 'b'
#define DROITE2 'd'
#define HAUTCAISSE 'H'
#define GAUCHECAISSE 'G'
#define BASCAISSE 'B'
#define DROITECAISSE 'D'
#define UNDO 'u'

#define TAILLE 12
#define CHOIX 20
#define FICHIER 30
#define ENTETE 50
#define DEPLACEMENT_MAX 500

typedef char t_plateau[TAILLE][TAILLE];
typedef char t_tabDeplacement[DEPLACEMENT_MAX];

int kbhit();
void charger_partie(t_plateau plateau, char fichier[]);
void afficher_plateau(t_plateau plateau, int zoom);
void deplacer(t_plateau plateau, char fichier[CHOIX]);
void afficher_entete(char fichier[CHOIX], int deplacements);
bool gagne(t_plateau plateau);
void enregistrer_partie(t_plateau plateau, char fichier[]);
void zoom_dezoom(char touche, int *zoom);
void tableau_deplacement(char touche, int *tabdeplacement, t_tabDeplacement t_deplacement);
void tableau_deplacementCaisse(char touche, int *tabdeplacement, t_tabDeplacement t_deplacement);
void enregistrer_deplacements(t_tabDeplacement t_deplacemt, int nb, char fic[]);
void undo (t_plateau plateau, int *tabdeplacement, t_tabDeplacement t_deplacement, int *deplacements);

int main(){
    int deplacements = 0;
    int zoom = 1;
    char fichier[FICHIER], choixNiveau[CHOIX];
    t_plateau plateau;
    printf("Quels niveau choisissez vous ? (niveau + nb de 1 à 6 ou nom niveau sauvegarder sans le .sok)\n");
    scanf("%s", choixNiveau);
    sprintf(fichier, "%s.sok", choixNiveau);
    charger_partie(plateau, fichier);
    system("clear");
    afficher_entete(fichier, deplacements);
    afficher_plateau(plateau, zoom);
    deplacer(plateau, fichier);
}

void charger_partie(t_plateau plateau, char fichier[]){
    FILE *f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f == NULL)
    {
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    }
    else
    {
        for (int ligne = 0; ligne < TAILLE; ligne++)
        {
            for (int colonne = 0; colonne < TAILLE; colonne++)
            {
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

void afficher_entete(char fichier[CHOIX], int deplacements){
    printf(" SOKOBAN\tNiveau : %s \n déplacement : zqsd  Zoomer : +  Dezoomer : -\n", fichier);
    printf(" Revenir en arrière : u \n");
    printf(" x = abandonner et r = recommencer \n nombres de déplacements = %d\n", deplacements);
}

void afficher_plateau(t_plateau plateau, int zoom){
    for (int i = 0; i < TAILLE; i++){ 
        for (int zo = 0; zo < zoom ; zo++ ){
            for (int j = 0; j < TAILLE; j++){
                for (int zo= 0; zo < zoom; zo++){
                    if (plateau[i][j] == CAISSECIBLE){
                        printf("$");
                    }
                    else if (plateau[i][j] == SOKOBANCIBLE){
                        printf("@");
                    }
                    else{
                        printf("%c", plateau[i][j]);
                    }
                }
            }
            printf("\n");
        }
    }
}

int kbhit()
{
    // la fonction retourne :
    // 1 si un caractere est present
    // 0 si pas de caractere présent
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    // mettre le terminal en mode non bloquant
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    // restaurer le mode du terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        unCaractere = 1;
    }
    return unCaractere;
}

void deplacer(t_plateau plateau, char fichier[CHOIX]){
    int x = 0;
    int y = 0;
    int dx = 0;
    int dy = 0;
    int nx = 0;
    int ny = 0;
    int bx = 0;
    int by = 0;
    int deplacements = 0;
    int tabdeplacement = 0;
    int zoom = 1;
    t_tabDeplacement t_deplacement;

    while (1){
        if (kbhit()){
            char touche = getchar();
            if (touche == ABANDON){
                char reponse;
                char reponse2;
                printf("Vous avez abandonné la partie.\n");
                printf("Sauvegarder ? o pour accepter ou n'importe quel touche pour annuler\n");
                scanf(" %c", &reponse); 
            if (reponse == 'o'){
                char nomFichier[CHOIX];
                printf("Entrez le nom du fichier de sauvegarde + .sok");
                scanf("%s", nomFichier);
                enregistrer_partie(plateau, nomFichier);
                printf("Partie sauvegardée dans %s\n", nomFichier);
            }
            printf("voulez vous sauvegarder la suite de déplacements effectuer ? O pour accepterou n'importe quel touche pour annuler\n");
            scanf(" %c", &reponse2);
            if ( reponse2 == 'O' ){
                char nomFichier2[CHOIX];
                printf("Entrez le nom du fichier de sauvegarde + .dep");
                scanf("%s", nomFichier2);
                enregistrer_deplacements(t_deplacement, tabdeplacement, nomFichier2);
            }
            break;
        }
            if (touche == RECOMMENCER ){
                char reponse2;
                printf("Etes vous sur de recommencer ? o pour oui ou n'importe quel touche pour annuler\n");
                scanf("%c", &reponse2);
            if (reponse2 == ACCEPTER){
                charger_partie(plateau, fichier);
                deplacements = 0;
                system("clear");
                afficher_entete(fichier, deplacements);
                afficher_plateau(plateau, zoom);
                continue;
            }

            }


            for (int i = 0; i < TAILLE; i++){
                for (int j = 0; j < TAILLE; j++){
                    if (plateau[i][j] == SOKOBAN || plateau[i][j] == SOKOBANCIBLE){
                        y = i;
                        x = j;
                    }
                }
            }
            if (touche == HAUT){
                dx = 0;
                dy = -1;
            }
            else if (touche == BAS){
                dx = 0;
                dy = 1;
            }
            else if (touche == GAUCHE){
                dx = -1;
                dy = 0;
            }
            else if (touche == DROITE){
                dx = 1;
                dy = 0;
            }
            else {
                dx = 0;
                dy = 0;
            }
            nx = x + dx;
            ny = y + dy;
            bx = nx + dx;
            by = ny + dy;
            if (plateau[ny][nx] == VIDE || plateau[ny][nx] == CIBLE){


                if (plateau[y][x] == SOKOBANCIBLE){
                    plateau[y][x] = CIBLE;
                }
                else{
                    plateau[y][x] = VIDE;
                }
                if (plateau[ny][nx] == CIBLE){
                    plateau[ny][nx] = SOKOBANCIBLE;
                }
                else{
                    plateau[ny][nx] = SOKOBAN;
                }
                deplacements++;
                tableau_deplacement(touche, &tabdeplacement, t_deplacement);
            }
            else if (plateau[ny][nx] == CAISSE || plateau[ny][nx] == CAISSECIBLE){
                if (plateau[by][bx] == VIDE || plateau[by][bx] == CIBLE){
                    if (plateau[by][bx] == CIBLE){
                        plateau[by][bx] = CAISSECIBLE;
                    }
                    else{
                        plateau[by][bx] = CAISSE;
                    }
                    if (plateau[y][x] == SOKOBANCIBLE){
                        plateau[y][x] = CIBLE;
                    }
                    else{
                        plateau[y][x] = VIDE;
                    }
                    if (plateau[ny][nx] == CAISSECIBLE){
                        plateau[ny][nx] = SOKOBANCIBLE;
                    }
                    else{
                        plateau[ny][nx] = SOKOBAN;
                    }
                    deplacements++;
                    tableau_deplacementCaisse(touche, &tabdeplacement, t_deplacement);
                }
            }
            
            if ( touche == UNDO ){
                undo(plateau, &tabdeplacement, t_deplacement, &deplacements);
            }

            zoom_dezoom(touche, &zoom);
            system("clear");
            afficher_entete(fichier, deplacements);
            afficher_plateau(plateau, zoom);
            if (gagne(plateau)){
                char reponse3;
                printf("PARTIE GAGNE !!!!!\n");
                printf("voulez vous sauvegarder la suite de déplacements effectuer ? O pour accepter ou n'importe quel touche pour annuler\n");
                scanf(" %c", &reponse3);
                if ( reponse3 == 'O' ){
                char nomFichier2[CHOIX];
                printf("Entrez le nom du fichier de sauvegarde + .dep");
                scanf("%s", nomFichier2);
                enregistrer_deplacements(t_deplacement, tabdeplacement, nomFichier2);
            }
                break;
            }
        }
    }
}

void zoom_dezoom(char touche, int *zoom){
    if ( touche == ZOOMER && *zoom < 3){
                (*zoom)++;
    }        
    else if ( touche == DEZOOMER && *zoom > 1) {
        (*zoom)--;
    }
}

bool gagne(t_plateau plateau){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if (plateau[i][j] == CAISSE) {
                return false; 
            }
        }
    }
    return true;
}

void enregistrer_partie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne='\n';

    f = fopen(fichier, "w");
    for (int ligne=0 ; ligne<TAILLE ; ligne++){
        for (int colonne=0 ; colonne<TAILLE ; colonne++){
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

void enregistrer_deplacements(t_tabDeplacement t, int nb, char fic[]){
    FILE * f;

    f = fopen(fic, "w");
    fwrite(t,sizeof(char), nb, f);
    fclose(f);
}

void tableau_deplacement(char touche, int *tabdeplacement, t_tabDeplacement t_deplacement) {
    if (*tabdeplacement < DEPLACEMENT_MAX) {
        if (touche == GAUCHE) {
            t_deplacement[*tabdeplacement] = GAUCHE2;
        }
        else if (touche == DROITE) {
            t_deplacement[*tabdeplacement] = DROITE2;
        }
        else if (touche == HAUT) {
            t_deplacement[*tabdeplacement] = HAUT2;
        }
        else if (touche == BAS) {
            t_deplacement[*tabdeplacement] = BAS2;
        }
        (*tabdeplacement)++;
    }

}

void tableau_deplacementCaisse(char touche, int *tabdeplacement, t_tabDeplacement t_deplacement) {
    if (*tabdeplacement < DEPLACEMENT_MAX) {
        if (touche == GAUCHE) {
            t_deplacement[*tabdeplacement] = GAUCHECAISSE;
        }
        else if (touche == DROITE) {
            t_deplacement[*tabdeplacement] = DROITECAISSE;
        }
        else if (touche == HAUT) {
            t_deplacement[*tabdeplacement] = HAUTCAISSE;
        }
        else if (touche == BAS) {
            t_deplacement[*tabdeplacement] = BASCAISSE;
        }
        (*tabdeplacement)++;
    }

}

void undo(t_plateau plateau, int *tabdeplacement, t_tabDeplacement t_deplacement, int *deplacements) {
    if ( (*tabdeplacement) > 0 ) {
        (*tabdeplacement)--;
        (*deplacements)--;
        int dx = 0, dy = 0;
        char dernier_dep = t_deplacement[*tabdeplacement];
        if (dernier_dep == GAUCHE2 || dernier_dep == GAUCHECAISSE) {
            dx = 1; dy = 0;
        }
        else if (dernier_dep == DROITE2 || dernier_dep == DROITECAISSE) {
            dx = -1; dy = 0;
        }
        else if (dernier_dep == HAUT2 || dernier_dep == HAUTCAISSE) {
            dx = 0; dy = 1;
        }
        else if (dernier_dep == BAS2 || dernier_dep == BASCAISSE) {
            dx = 0; dy = -1;
        }
        int x = -1;
        int y = -1;
        for (int i = 0; i < TAILLE; i++) {
            for (int j = 0; j < TAILLE; j++) {
                if (plateau[i][j] == SOKOBAN || plateau[i][j] == SOKOBANCIBLE) {
                    y = i;
                    x = j;
                }
            }
        }

            int nx = x + dx; 
            int ny = y + dy;
            if (plateau[y][x] == SOKOBAN) {
                plateau[y][x] = VIDE;
            }
            else if (plateau[y][x] == SOKOBANCIBLE) {
                plateau[y][x] = CIBLE;
            }
            if (plateau[ny][nx] == VIDE) {
                plateau[ny][nx] = SOKOBAN;
            }
            else if (plateau[ny][nx] == CIBLE) {
                plateau[ny][nx] = SOKOBANCIBLE;
            }
            if (dernier_dep == GAUCHECAISSE || dernier_dep == DROITECAISSE || dernier_dep == HAUTCAISSE || dernier_dep == BASCAISSE) {
                int caisse_x = x - dx; 
                int caisse_y = y - dy;
                if (plateau[caisse_y][caisse_x] == CAISSE) {
                    plateau[caisse_y][caisse_x] = VIDE;
                }
                else if (plateau[caisse_y][caisse_x] == CAISSECIBLE) {
                    plateau[caisse_y][caisse_x] = CIBLE;
                }
                if (plateau[y][x] == VIDE) {
                    plateau[y][x] = CAISSE;
                }
                else if (plateau[y][x] == CIBLE) {
                    plateau[y][x] = CAISSECIBLE;
                }  
        }
    }
}



