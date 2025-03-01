/* Travail réalisé sur un <Dell Inspiron> avec <Windows 11> */

/* Auteur : Adam Nhaila
   Date : 08/12/2023

   compilation avec la commande : gcc -W -Wall -ansi -pedantic mandel.c colors.c -o mandel -lm
   exécution avec la commande :  ./mandel 600 -1.142817421949293,-0.21198254168631775,0.005544570446197573,0.004295131188703422 1000x1000 mandelbrot est un génie

    Ce programme permet de générer une image de l'ensemble de mandelbrot avec les paramètres d'affichage souhaités par l'utilisateur.
    On apprend que les paramètres du programme (la zone dans le plan complexe et la taille de l'image) permettent d'explorer différentes parties de l'ensemble, ce qui donne des motifs très très beaux.
    
*/

#include <stdio.h>
#include <math.h> /* pour les fonctions mathématiques*/ 
#include <stdlib.h> /* pour les fonctions de manipulation de chaînes*/
#include <string.h> /* pour la fonction strcat et strcpy*/
#include "colors.h" /* pour le fichier des couleurs*/

#define STRMAX 256
#define DEFAULT_WIDTH 1000
#define DEFAULT_HEIGHT 1000
#define BW 0
#define BW_ALTERN 1
#define GREY_ST 2
#define GREY_SM 3
#define RGB 4


/*structure struct pixdiv contenant trois champs : un entier iter et trois doubles x et y*/
struct pixdic{
    int iter;
    double x;
    double y;
};

/*Définition d'une structure struct camera dans laquelle figurent les coordonnées x,y
du point que l’on regarde et les dimensions du rectangle de visualisation*/
struct camera{

    double x;      
    double y;      
    double W;  
    double H; 
};

/*Définition de la structure struct render qui regroupe tous les champs relatifs au calcul et à l’affichage d’une image*/
struct render{

    /* les bornes min et max dans le plan complexe */
    double Oxmin;
    double Oxmax;
    double Oymin;
    double Oymax;

    /* la largeur et la hauteur de l’image */
    long width;
    long height;

    /* le nombre maximal d’itérations */
    int maxiter;

    /*le rayon d’échappement */    
    int radius;

    /*l’image*/
    struct pixdic *img;

    /*nom du fichier*/
    char basename[STRMAX];

    /*champ pov de type struct camera*/
    struct camera pov; 

    /* champ pour spécifier le type de rendu (RGB,BW,BW_ALTERN,GREY_SM,GREY_ST)*/
    int type;
};

/* Prototype des fonctions*/

double map(int v, int imin, int imax, double omin, double omax);
void render_init(struct render *set, int argc, char *argv[]);
void render_image(struct render *set);
void save_image(struct render *set);
void get_pixel_color(struct render *set, int Px, int Py, int *r, int *g, int *b);
int m2v(struct render *set, int x, int y);
void cam2rect(struct render *set, struct camera *pov);
void erreur_argument();
void concat_arguments(int argc, char *argv[], char *sortie);
void hsv2rgb(struct color *rgb, struct color *hsv);
void set_defaults(struct render *set);
int load_config(struct render *set, char *config_file);




int main(int argc, char *argv[]){

    struct render set;

    render_init(&set, argc, argv);

    cam2rect(&set, &set.pov);

    render_image(&set);

    save_image(&set);

    free(set.img);

    return 0;
}




/* la fonction map prend un int dans un intervalle et rend son proportionnel dans une autre intervalle*/
double map(int v, int imin, int imax, double omin, double omax){

    double s;

    s=(omax-omin)/(imax-imin)*(v-imin)+omin; 

    return s;
}


/*Fonction qui met à jour des champs xmin, xmax, ymin, ymax de la struct render en fonction de la struct camera*/
void cam2rect(struct render *set, struct camera *pov){

    set->Oxmin=pov->x-pov->W/2.0;
    set->Oxmax=pov->x+pov->W/2.0;
    set->Oymin=pov->y-pov->H/2.0;
    set->Oymax=pov->y+pov->H/2.0;
}


/*fonction d'initialisation de tous les champs de la variable set de type struct render*/
void render_init(struct render *set, int argc, char *argv[]){

    /*Initialisation des paramètres*/
    set_defaults(set);

    /* établissement des valeurs entrée dans la commande comme paramètre sinon mettre les paramètres par défaut utiliser dans les versions précédentes*/
    if (argc>1){
        if (sscanf(argv[1], "%d", &set->maxiter)!=1){

            /* si le premier argument du programme n’est pas un entier alors il est considéré comme le nom du fichier de configuration*/
            if (load_config(set, argv[1])==0){
                /* En cas d'échec du chargement, on remet à défaut*/
                set_defaults(set);
            }
        }
    }

    if (argc>2){
        if (sscanf(argv[2], "%lf,%lf,%lf,%lf", &set->pov.x, &set->pov.y, &set->pov.W, &set->pov.H)!=4){
            erreur_argument();
            exit(1);
        }
    }

    if (argc>3){
        if (sscanf(argv[3], "%ldx%ld", &set->width, &set->height)!=2){
            erreur_argument();
            exit(1);
        }

        if (set->width<=0 || set->height<=0){
            erreur_argument();
            exit(1);
        }
    }

    /* Allocation dynamique de la mémoire pour le tableau img*/
    set->img=malloc((set->width*set->width)*sizeof(struct pixdic));
    if (set->img==NULL){
        printf("Erreur d'allocation mémoire !\n");
        exit(1);
    }
 
    if (argc>4){

        /* concaténation des arguments servant de nom pour le fichier, j'utilise concat_arguments et une variable intermédiaire nom_sortie*/
        char nom_sortie[STRMAX];
        concat_arguments(argc, argv, nom_sortie);
        strncpy(set->basename, nom_sortie, STRMAX - 5);
    } 
}


/*fonction de génération du tableau d'image*/
void render_image(struct render *set){
    int Py, Px, i, j;
    double x, x0;
    double y, y0;

    /*Parcours de chaque pixel de l'image*/
    for (Py=0; Py<set->height; Py++){

        /* Affichage du numéro de la ligne en cours de calcul*/
        printf("Ligne %d\r", Py);

        for (Px = 0; Px<set->width; Px++){

            /*Mapping des coordonnées du pixel dans la zone complexe*/
            x=x0=map(Px,0,set->width,set->Oxmin,set->Oxmax);
            y=y0=map(Py,0,set->height,set->Oymin,set->Oymax);
            i=0;

            /*Itération pour déterminer la couleur du pixel*/
            while (i<set->maxiter && sqrt(pow(x,2)+pow(y,2))<set->radius){
             
                double xt=x;
                x=pow(xt,2)-pow(y,2)+x0;
                y=2*xt*y+y0;
                i+=1;
            }

            /* 4 itérations supplémentaires pour x et y afin d'obtenir plus de précision*/
            for(j=0;j<4;j++){
                double xt=x;
                x=pow(xt,2)-pow(y,2)+x0;
                y=2*xt*y+y0;
            }

            /*Stockage de l'itération */
            set->img[m2v(set,Px,Py)].iter=i;
            set->img[m2v(set,Px,Py)].x=x;
            set->img[m2v(set,Px,Py)].y=y;

        }
    }

}


/*Fonction pour sauvegarder l'image dans un fichier*/
void save_image(struct render *set){

    /* Paramètre de boucle*/
    int Py, Px;

    /* Paramètre de couleur*/
    int r, g, b;


    /*Déclaration d'un pointeur vers une structure FILE, qui sera utilisé pour manipuler le fichier.ppm*/
    FILE* fout;

    /* variable intermédiaire qui servira comme nom.ppm*/
    char final_name[STRMAX]="";

    /* Création du nom du fichier image*/
    strcpy(final_name,set->basename);
    strcat(final_name,".ppm");

    /* Ouverture du fichier*/
    fout=fopen(final_name,"w");

    /* Vérification si le fichier a été ouvert correctement */
    if (fout==NULL){

        printf("erreur\n");
        exit(1);
    }

    /*Écriture de l'en-tête du fichier PPM*/
    fprintf(fout, "P3\n%ld %ld\n255\n", set->width, set->height);
    fprintf(fout,"#Nombre d'itérations max = %i\n", set->maxiter);
    fprintf(fout,"#la zone complexe visualisée est [%f;%f]x[%f;%f]\n", set->Oxmin, set->Oxmax, set->Oymin, set->Oymax);

    /* Écriture des données de pixels dans le fichier*/
    for (Py=0; Py<set->height;Py++){
        for (Px=0; Px<set->width; Px++){

            get_pixel_color(set, Px, Py, &r, &g, &b);

            fprintf(fout, "%d %d %d\n", r, g, b);
        }
    }

    /* Fermeture du fichier*/
    fclose(fout);

}


/*Fonction qui pour un couple (x,y) dans l’image [set->weight;set->height], renvoie l’indice 
correspondant dans le vecteur img de la structure struct render*/
int m2v(struct render *set, int x, int y){
    return y*set->width+x;
}


/* Fontion pour obtenir la couleur d'un pixel*/
void get_pixel_color(struct render *set, int Px, int Py, int *r, int *g, int *b){
    /*Initialisation de variables servant à avoir un code plus sobre et éviter les noms longs*/
    int indice;
    int iter;
    int iter_normal;

    double x;
    double y;
    double n_it;
    double grey;

    struct color hsv;
    struct color rgb;

    /*Calcul de l'indice dans le tableau img*/
    indice=m2v(set, Px, Py);
    iter=set->img[indice].iter;

    x=set->img[indice].x;
    y=set->img[indice].y;
    n_it=iter;

    switch(set->type){
        case BW:
            if (iter==set->maxiter) {
                /* Noir */
                *r=0;
                *g=0;
                *b=0;
            } 
            else {
                /* Blanc */
                *r=255;
                *g=255;
                *b=255;
            }
            break;

        case BW_ALTERN:

            /* Même parité: on voit si iter et maxiter sont congrues modulo 2 */
            if (iter%2==(set->maxiter)%2) {
                *r=0;
                *g=0;
                *b=0;
            } 
            else {
                *r=255;
                *g=255;
                *b=255;
            }
            break;

        case GREY_ST:

            if (iter==set->maxiter) {
                /* Noir */
                *r=0;
                *g=0;
                *b=0;
            }
            else{
                /* ré-échantillonnage des niveaux de gris*/
                iter_normal=(int)(((double)iter/set->maxiter)*255);
                /* N.B: Pour avoir un dégradé de noir vers blanc on pourra mettre 255-iter_normal et c'est plus beau néanmoins je garde ce qui est demandé dans l'énoncé*/
                *r=iter_normal;
                *g=iter_normal;
                *b=iter_normal;
            }
            break;

        case GREY_SM:

            if (iter==set->maxiter){
                *r=0;
                *g=0;
                *b=0;

            }   
            else {
                /* Gris amélioré*/
                grey=5+n_it-log(log(pow(x,2)+pow(y,2))/log(2))/log(2);
                grey=floor(512*grey/set->maxiter);

                if (grey>255){
                    grey=255;
                }
                *r=grey;
                *g=grey;
                *b=grey;
            }
            break;

        case RGB:

            /* Gris amélioré*/
            grey=5+n_it-log(log(pow(x,2)+pow(y,2))/log(2))/log(2);

            /*teinte de hsv dépend de grey*/
            hsv.c1=360*grey/set->maxiter;
            hsv.c2=1.0;
            hsv.c3=1.0;

            /*Conversion de HSV en RGB*/
            hsv2rgb(&rgb,&hsv);
            
            *r=rgb.c1;
            *g=rgb.c2;
            *b=rgb.c3;
            break;

    }
}


/*fonction du message d'erreur dans le cas où les arguments de la ligne de commande ne sont pas conformes*/
void erreur_argument(){
    printf("Erreur d'argument\n");
    printf("Commande d'éxecution doit être sous la forme: ./mandel arg1 arg2 arg3 arg4.... avec : \n");
    printf("type(arg1) : int : nombre d'itérations\n");
    printf("type(arg2) : float,float,float,float : borne inf réelle, borne max réelle, borne inf complexe, borne max complexe\n");
    printf("type(arg3) : intxint : largeurxhauteur\n");
    printf("type(arg4,arg5,..) : char[] : nom souhaité pour le fichier par exemple: mandel image test\n");
}


/* fonction qui concatène et met _ à la place de l'espace*/
void concat_arguments(int argc, char *argv[], char *sortie){
    int i;
    int j;

    /*Concaténation des arguments à partir du quatrième arguments*/
    j=0;
    for (i=4;i<argc; i++){
        if (j>0){
            sortie[j]='_';
            j++;
        }
        strcpy(&sortie[j],argv[i]);
        j+=strlen(argv[i]);
    }

    sortie[j]='\0';
}


/*Fonction qui initialise les valeurs de set par défaut*/
void set_defaults(struct render *set){

    /*Les valeurs par défaut du rendu*/
    set->maxiter=100;
    set->radius = 2;

    set->pov.x=-0.76;
    set->pov.y=0;
    set->pov.W=2.48;
    set->pov.H=2.48;

    set->width=DEFAULT_WIDTH;
    set->height=DEFAULT_HEIGHT;

    strncpy(set->basename, "mandel", STRMAX);

    set->type=RGB;
}


/*Fontion pour la lecture du fichier de configuration et mise à jour de set*/
int load_config(struct render *set, char *config_file){

    int taille=STRMAX-5;
    FILE * fichier;
    char ligne[STRMAX-5]; /* chaîne de caractère où je stocke chaque ligne*/
    int i; /*Compteur pour suivre le numéro de ligne*/

    i=0;

    /*Ouverture du fichier en mode lecture*/

    fichier=fopen(config_file,"r");

    /*On vérifit si le fichier est ouvert correctement*/
    if(fichier==NULL){
        printf("Erreur dans l'ouverture du fichier de configuration\n");
        return 0;
    }

    /* lecture de chaque ligne du fichier */
    while(fgets(ligne,taille,fichier)!=NULL){

        i+=1;

        if (i==1){
            if (sscanf(ligne,"%s",set->basename)==0){
            return 0;
            }
        }

        if (i==2){
            if (sscanf(ligne,"%ldx%ld", &set->width, &set->height)==0){
            return 0;
            }
        }

        if (i==3){

            if (strcmp(ligne,"rgb\n")==0){
                set->type=RGB;
            }

            else if (strcmp(ligne,"b&w\n")==0){
                set->type=BW; 
            }

            else if (strcmp(ligne,"b&w_alt\n")==0){
                set->type=BW_ALTERN;
            }

            else if (strcmp(ligne,"grey_stepped\n")==0){
                set->type=GREY_ST;
            }

            else if (strcmp(ligne,"grey_smoothed\n")==0){
                set->type=GREY_SM;
            }
            else{
                printf("Pour l'argument 3, le type de rendu attendu (b&w, b&w_alt, grey_stepped, grey_smoothed, rgb)\n");
                return 0;
            }
        }

        if (i==4){

            if (sscanf(ligne, "%lf,%lf,%lf,%lf", &set->pov.x, &set->pov.y, &set->pov.W, &set->pov.H)!=4){
                return 0;
            }

        }

        if (i==5){

            if (sscanf(ligne, "%d", &set->maxiter)==0){
                return 0;
            }
        }
        if (i==6){
            if (sscanf(ligne, "%d", &set->radius)==0){
                return 0;
            }
        }
    }

    fclose(fichier);
    return 1;
}


