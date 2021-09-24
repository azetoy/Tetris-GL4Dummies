
!!!SI LE ./tetris ne se lance toujours pas après 3 ou 4 tentatives mettez en commentaire la fonction checker haute et son appel apparemment cela se produit en fonction des drivers de la carte/chipset graphique

Mon projet consiste à faire un Tetris en utilisant Gl4 dummies

objectif fait tourner un Tetris 
ce qui a été réaliser :
-création des tetrominos
-Déplacement des tetrominos
-test de collision entre les tetrominos
-test de collision contre les bords
-Création d'un thread pour le déplacement automatique vers le bas toutes les secondes
-fermeture du jeu si le bord supérieur et déplacé
-Chacun tetrominos à sa couleur 

ce qui reste à faire:
pas grand-chose éventuellement améliorer tout ça pour le rendre plus optimiser et corriger certains bugs éventuellement comment le récupérer :

comment le recuperer :

git clone 

il vous faudra les lib suivantes

pthread (pour crée un thread)

-sudo apt-get install libncurses5-dev libncursesw5-dev

comment le lancer : 

make clean all

./tetris

 Les déplacements sont gérés avec les flèches directionnelles et la camera avec z, q, s,d

Have a good game ;)
