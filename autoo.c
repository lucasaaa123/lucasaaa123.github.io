#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define SOKOBAN '@'
#define MUR '#'
#define CIBLE '.'
#define CAISSE '$'
#define VIDE ' '
#define CAISSECIBLE '*'
#define SOKOBANCIBLE '+'

#define HAUT 'h'
#define GAUCHE 'g'
#define BAS 'b'
#define DROITE 'd'
#define HAUT_Z 'z'
#define GAUCHE_Q 'q'
#define BAS_S 's'

#define HAUTCAISSE 'H'
#define GAUCHECAISSE 'G'
#define BASCAISSE 'B'
#define DROITECAISSE 'D'
#define HAUT_Z_MAJ 'Z'
#define GAUCHE_Q_MAJ 'Q'
#define BAS_S_MAJ 'S'

#define TAILLE 12
#define CHOIX 50
#define FICHIER 50
#define DEPLACEMENT 2000

typedef char t_plateau[TAILLE][TAILLE];
typedef char typeDeplacements[DEPLACEMENT];

// Historique des positions
typedef struct {
    int x;
    int y;
} Position;

void charger_partie(t_plateau plateau, char fichier[]);
void charger_deplacements(typeDeplacements t, char fichier[], int * nb);
void afficher_entete(char fichier[CHOIX], int compteur, int nb);
void afficher_plateau(t_plateau plateau);
int deplacement(t_plateau plateau, char dep);
bool gagne(t_plateau plateau);
void copier_plateau(t_plateau source, t_plateau dest);
void optimiser_sequence(t_plateau plateau_init, typeDeplacements dep_in, int nb_in, typeDeplacements dep_out, int *nb_out);
void sauvegarder_deplacements_auto(typeDeplacements t, int nb, char nom_source[]);

int main(){
    char fichier[FICHIER], suiteDep[CHOIX];
    int nb_init = 0, nb_opti = 0, compteur = 0;
    
    t_plateau plateau_initial, plateau_jeu;
    typeDeplacements dep_initial, dep_optimise;

    // Charger plateau + mouvement
    printf("1. Quel niveau choisissez-vous ? (ex: A.sok)\n");
    scanf("%s", fichier);
    charger_partie(plateau_initial, fichier);

    printf("2. Quelle suite de déplacements ? (ex: move_1.dep)\n");
    scanf("%s", suiteDep);
    charger_deplacements(dep_initial, suiteDep, &nb_init);

    // Faire jouer le sokoban
    copier_plateau(plateau_initial, plateau_jeu);
    
    system("clear");
    afficher_entete(fichier, compteur, nb_init);
    afficher_plateau(plateau_jeu);
    usleep(500000); 

    while (compteur < nb_init) {
        system("clear");
        
        deplacement(plateau_jeu, dep_initial[compteur]);
        compteur++;
        
        afficher_entete(fichier, compteur, nb_init);
        afficher_plateau(plateau_jeu);
        
        usleep(150000); 
    }

    // Bilan et optimisation
    bool est_solution = gagne(plateau_jeu);
    
    printf("\n--- BILAN ---\n");
    if (est_solution) {
        printf("Statut : La suite est une SOLUTION VALIDE pour %s.\n", fichier);
    } else {
        printf("Statut : La suite N'EST PAS une solution (ECHEC).\n");
    }

    // Calcul de l'optimisation
    optimiser_sequence(plateau_initial, dep_initial, nb_init, dep_optimise, &nb_opti);

    printf("Longueur initiale : %d coups\n", nb_init);
    printf("Longueur optimisée : %d coups (après suppression des inutiles)\n", nb_opti);

    // Sauvegarde auto 
    sauvegarder_deplacements_auto(dep_optimise, nb_opti, suiteDep);

    return 0;
}


void optimiser_sequence(t_plateau plateau_init, typeDeplacements dep_in, int nb_in, typeDeplacements dep_out, int *nb_out) {
    t_plateau simu;
    copier_plateau(plateau_init, simu);
    
    Position historique[DEPLACEMENT];
    int index_dep_out = 0;
    int taille_historique = 0;

    int start_x = 0, start_y = 0;
    for(int i=0; i<TAILLE; i++) {
        for(int j=0; j<TAILLE; j++) {
            if(simu[i][j] == SOKOBAN || simu[i][j] == SOKOBANCIBLE) {
                start_y = i; start_x = j;
            }
        }
    }
    
    historique[0].x = start_x;
    historique[0].y = start_y;
    taille_historique = 1;

    for (int i = 0; i < nb_in; i++) {
        char coup = dep_in[i];
        
        int type_mouv = deplacement(simu, coup);

        // Supprimer les déplacements inutile (mur)
        if (type_mouv == 0) {
            continue; 
        }

        int cur_x = 0, cur_y = 0;
        for(int r=0; r<TAILLE; r++) {
            for(int c=0; c<TAILLE; c++) {
                if(simu[r][c] == SOKOBAN || simu[r][c] == SOKOBANCIBLE) {
                    cur_y = r; cur_x = c;
                }
            }
        }

        if (type_mouv == 2) {
            // Une caisse bouge
            dep_out[index_dep_out] = coup;
            index_dep_out++;
            
            taille_historique = 0;
            historique[taille_historique].x = cur_x;
            historique[taille_historique].y = cur_y;
            taille_historique++;
        } 
        else if (type_mouv == 1) {
            // Marche simple : Vérifier les boucles
            int index_retrouve = -1;

            for (int k = 0; k < taille_historique; k++) {
                if (historique[k].x == cur_x && historique[k].y == cur_y) {
                    index_retrouve = k;
                    break;
                }
            }

            if (index_retrouve != -1) {
                // Boucle détecté
                int coups_a_annuler = (taille_historique - 1) - index_retrouve;
                index_dep_out -= coups_a_annuler;
                
                taille_historique = index_retrouve + 1;
            } else {
                // Nouvelle position
                dep_out[index_dep_out] = coup;
                index_dep_out++;
                
                historique[taille_historique].x = cur_x;
                historique[taille_historique].y = cur_y;
                taille_historique++;
            }
        }
    }
    *nb_out = index_dep_out;
    dep_out[index_dep_out] = '\0';
}

int deplacement(t_plateau plateau, char dep){
    int y = 0, x = 0, dx = 0, dy = 0, nx = 0, ny = 0, bx = 0, by = 0;

    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if (plateau[i][j] == SOKOBAN || plateau[i][j] == SOKOBANCIBLE){
                y = i; x = j;
            }
        }
    }

    switch(dep) {
        case HAUT: case HAUTCAISSE: case HAUT_Z: case HAUT_Z_MAJ:
            dx = 0; dy = -1; break;
        case GAUCHE: case GAUCHECAISSE: case GAUCHE_Q: case GAUCHE_Q_MAJ:
            dx = -1; dy = 0; break;
        case DROITE: case DROITECAISSE: 
            dx = 1; dy = 0; break;
        case BAS: case BASCAISSE: case BAS_S: case BAS_S_MAJ:
            dx = 0; dy = 1; break;
    } 

    if (dx != 0 || dy != 0){
        nx = x + dx; ny = y + dy;
        bx = nx + dx; by = ny + dy;

        if (plateau[ny][nx] == VIDE || plateau[ny][nx] == CIBLE){
            if (plateau[y][x] == SOKOBANCIBLE) plateau[y][x] = CIBLE;
            else plateau[y][x] = VIDE;
            
            if (plateau[ny][nx] == CIBLE) plateau[ny][nx] = SOKOBANCIBLE;
            else plateau[ny][nx] = SOKOBAN;
            return 1;
        }
        else if (plateau[ny][nx] == CAISSE || plateau[ny][nx] == CAISSECIBLE){
            if (plateau[by][bx] == VIDE || plateau[by][bx] == CIBLE){
                if (plateau[by][bx] == CIBLE) plateau[by][bx] = CAISSECIBLE;
                else plateau[by][bx] = CAISSE;

                if (plateau[y][x] == SOKOBANCIBLE) plateau[y][x] = CIBLE;
                else plateau[y][x] = VIDE;

                if (plateau[ny][nx] == CAISSECIBLE) plateau[ny][nx] = SOKOBANCIBLE;
                else plateau[ny][nx] = SOKOBAN;
                return 2;
            }
        }
    }
    return 0; 
}

void charger_partie(t_plateau plateau, char fichier[]){
    FILE *f = fopen(fichier, "r");
    if (f == NULL) { printf("ERREUR FICHIER SOK\n"); exit(EXIT_FAILURE); }
    for (int ligne = 0; ligne < TAILLE; ligne++) {
        for (int colonne = 0; colonne < TAILLE; colonne++) {
            char c;
            do { c = fgetc(f); } while(c == '\n' || c == '\r');
            plateau[ligne][colonne] = c;
        }
    }
    fclose(f);
}

void charger_deplacements(typeDeplacements t, char fichier[], int * nb){
    FILE * f = fopen(fichier, "r");
    char dep;
    *nb = 0;
    if (f==NULL) { printf("FICHIER DEP NON TROUVE\n"); return; }
    while (fread(&dep, sizeof(char), 1, f) == 1){
        if (dep != '\n' && dep != '\r' && dep != ' ' && *nb < DEPLACEMENT) {
            t[*nb] = dep;
            (*nb)++;
        }
    }
    fclose(f);
}

void afficher_plateau(t_plateau plateau){
    for (int i = 0; i < TAILLE; i++){ 
        for (int j = 0; j < TAILLE; j++){
            if (plateau[i][j] == CAISSECIBLE) printf("*");
            else if (plateau[i][j] == SOKOBANCIBLE) printf("+");
            else printf("%c", plateau[i][j]);
        }
        printf("\n");
    }
}

void afficher_entete(char fichier[CHOIX], int compteur, int nb){
    printf("SOKOBAN - Niveau : %s \n", fichier);
    printf("Déplacements : %d / %d\n", compteur, nb);
}

bool gagne(t_plateau plateau){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if (plateau[i][j] == CAISSE) return false; 
        }
    }
    return true;
}

void copier_plateau(t_plateau source, t_plateau dest) {
    for (int i=0; i<TAILLE; i++) {
        for (int j=0; j<TAILLE; j++) {
            dest[i][j] = source[i][j];
        }
    }
}

void sauvegarder_deplacements_auto(typeDeplacements t, int nb, char nom_source[]) {
    char nom_final[CHOIX + 10]; 
    
    // écriture automatique de opti_ + nom_source
    sprintf(nom_final, "opti_%s", nom_source);
    
    FILE *f = fopen(nom_final, "w");
    if (f) {
        for(int i=0; i<nb; i++) fprintf(f, "%c", t[i]);
        fclose(f);
        printf("\nSauvegarde AUTOMATIQUE effectuée sous : %s <<<\n", nom_final);
    } else {
        printf("\nERREUR : Impossible de créer le fichier %s\n", nom_final);
    }
}