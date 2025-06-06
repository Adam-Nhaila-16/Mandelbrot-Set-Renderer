# Mandelbrot-Set-Renderer

Ce projet en C génère des images de l’ensemble de Mandelbrot au format PPM.

Contenu
	•	mandel.c : calcul des itérations et génération des pixels.
	•	colors.c et colors.h : gestion des couleurs et conversion HSV→RGB.
	•	Fichiers de test (full_bw_alt.mdb, full_rgb.mdb).

Prérequis
	•	macOS (ou Linux) avec un compilateur C (Clang ou GCC).
	•	Pour macOS, installer les Command Line Tools si nécessaire :

	•	xcode-select --install


	•	(Optionnel) Homebrew : brew install gcc

Compilation

Dans le dossier du projet, exécutez :

clang -W -Wall -ansi -pedantic mandel.c colors.c -o mandel -lm

Ou, si vous utilisez GNU GCC installé via Homebrew (par exemple gcc-14) :

gcc-14 -W -Wall -ansi -pedantic mandel.c colors.c -o mandel -lm

Utilisation

./mandel <maxiter> <x,y,W,H> <width>x<height> <nom_de_base>

	•	<maxiter> : nombre maximum d’itérations (ex. 600).
	•	<x,y,W,H> : coordonnées du centre (x,y) et taille du rectangle (W = largeur, H = hauteur) séparés par des virgules.
	•	<width>x<height> : résolution en pixels (ex. 1000x1000).
	•	<nom_de_base> : nom du fichier de sortie sans extension.

Exemple

./mandel 600 -1.1428,-0.2119,0.00554,0.004295 1000x1000 mandelbrot

Génère mandelbrot.ppm en couleur.

Mode noir & blanc avec fichier de configuration

Créez un fichier config.txt :

output_bw
1000x1000
b&w
-1.1428,-0.2119,0.00554,0.004295
600
2

Puis lancez :

./mandel config.txt

Visualisation

Le format PPM (ASCII) peut être ouvert avec GIMP, IrfanView ou converti en PNG :

convert mon_image_bw.ppm mon_image_bw.png

