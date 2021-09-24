/*!\file window.c
 * \brief Utilisation du raster "maison" pour finaliser le pipeline de
 * rendu 3D. Ici on peut voir les géométries disponibles.
 * \author Farès BELHADJ, amsi@up8.edu
 * \date December 4, 2020.
 * \todo pour les étudiant(e)s : changer la variation de l'angle de
 * rotation pour qu'il soit dépendant du temps et non du framerate
 */
#include <assert.h>
/* inclusion des entêtes de fonctions de gestion de primitives simples
 * de dessin. La lettre p signifie aussi bien primitive que
 * pédagogique. */
#include <GL4D/gl4dp.h>
/* inclure notre bibliothèque "maison" de rendu */
#include "moteur.h"
#include <ncurses.h>

/* inclusion des entêtes de fonctions de création et de gestion de
 * fenêtres système ouvrant un contexte favorable à GL4dummies. Cette
 * partie est dépendante de la bibliothèque SDL2 */
#include <GL4D/gl4duw_SDL2.h>
extern unsigned int * board(int w, int h);
extern void down();
extern void up();
extern void left();
extern void right();
/* protos de fonctions locales (static) */
static void init(void);
static void draw(void);
static void key(int keycode);
static void sortie(void);

/*!\brief un identifiant pour l'écran (de dessin) */
static GLuint _screenId = 0;
static float eyeX = -10.0,eyeY = 10.0,eyeZ = 50.0;
static float centerX = -10.0,centerY = 7.0,centerZ = 0.0;
static float angle = 0.0f;

vec4 blue = {0.6745f,0.7961f,0.8824f}, green = {0.0863f,0.8588f,0.5765f}, red = {0.8667f,0.0667f,0.3333f},purple = {0.5098f,0.1804f,0.5059f};
vec4 black = {0.0f,0.0f,0.0f}, rust = {0.7373f,0.2235f,0.0314f},platinium = {0.902f,0.902f,0.9137f},pink = {1.0f,0.2118f,0.6f};


/*!\brief une surface représentant un quadrilatère */
static surface_t * _quad = NULL;
/*!\brief une surface représentant un cube */
static surface_t * _cube = NULL;
static surface_t * _shape = NULL;
static unsigned int * _board = NULL;
static int bW = 10;
static int bH = 20;

/* des variable d'états pour activer/désactiver des options de rendu */
static int _use_tex = 1, _use_color = 1, _use_lighting = 1;

/*!\brief on peut bouger la caméra vers le haut et vers le bas avec cette variable */
static float _ycam = 3.0f;

/*!\brief paramètre l'application et lance la boucle infinie. */
int main(int argc, char ** argv) {
  /* tentative de création d'une fenêtre pour GL4Dummies */
  if(!gl4duwCreateWindow(argc, argv, /* args du programme */
			 "Tetris", /* titre */
			 10, 10, 640, 480, /* x, y, largeur, heuteur */
			 GL4DW_SHOWN) /* état visible */) {
    /* ici si échec de la création souvent lié à un problème d'absence
     * de contexte graphique ou d'impossibilité d'ouverture d'un
     * contexte OpenGL (au moins 3.2) */
    return 1;
  }
  /* Pour forcer la désactivation de la synchronisation verticale */
  SDL_GL_SetSwapInterval(0);
  init();
  /* création d'un screen GL4Dummies (texture dans laquelle nous
   * pouvons dessiner) aux dimensions de la fenêtre */
  _screenId = gl4dpInitScreen();
  /* mettre en place la fonction d'interception clavier */
  gl4duwKeyDownFunc(key);
  /* mettre en place la fonction de display */
  gl4duwDisplayFunc(draw);
  /* boucle infinie pour éviter que le programme ne s'arrête et ferme
   * la fenêtre immédiatement */
  gl4duwMainLoop();
  return 0;
}

/*!\brief init de nos données, spécialement les trois surfaces
 * utilisées dans ce code */
void init(void) {
  GLuint id;  
  /* on créé nos trois type de surfaces */
  _quad   =   mkQuad();       /* ça fait 2 triangles        */
  _cube   =   mkCube();       /* ça fait 2x6 triangles      */
  _shape  =   mkCube();
  /* on change les couleurs de surfaces */

  _quad->dcolor = pink; _cube->dcolor = blue; _shape->dcolor = red;
  /* on leur rajoute la même texture */
  id = getTexFromBMP("images/tex.bmp");
  setTexId(  _quad, id);
  setTexId(  _cube, id);
  setTexId(  _shape, id);

  /* si _use_tex != 0, on active l'utilisation de la texture pour les
   * trois */
  if(_use_tex) {
    enableSurfaceOption(  _quad, SO_USE_TEXTURE);
    enableSurfaceOption(  _cube, SO_USE_TEXTURE);
    enableSurfaceOption(  _shape, SO_USE_TEXTURE);
 
  }
  /* si _use_lighting != 0, on active l'ombrage */
  if(_use_lighting) {
    enableSurfaceOption(  _quad, SO_USE_LIGHTING);
    enableSurfaceOption(  _cube, SO_USE_LIGHTING);
    enableSurfaceOption(  _shape, SO_USE_LIGHTING);

  }
  _board = board(bW,bH);
  /* mettre en place la fonction à appeler en cas de sortie */
  atexit(sortie);
}

/*!\brief la fonction appelée à chaque display. */
void draw(void) {
  int i, j;
  float mvMat[16], projMat[16], nmv[16];
  /* effacer l'écran et le buffer de profondeur */
  gl4dpClearScreen();
  clearDepth();
  /* des macros facilitant le travail avec des matrices et des
   * vecteurs se trouvent dans la bibliothèque GL4Dummies, dans le
   * fichier gl4dm.h */
  /* charger un frustum dans projMat */
  MFRUSTUM(projMat, -0.05f, 0.05f, -0.05f, 0.05f, 0.1f, 1000.0f);
  /* charger la matrice identité dans model-view */
  MIDENTITY(mvMat);
  /* on place la caméra en arrière-haut, elle regarde le centre de la scène */
  lookAt(mvMat, eyeX, eyeZ, eyeY, centerX, centerZ, centerY, 0, 1.0, 0);


  memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
  rotate(nmv, -90.0f, 1.0f, 0.0f, 0.0f);
  scale(nmv, bW - 1, bH - 1, 1.0f);
  transform_n_raster(_quad, nmv, projMat);
  /* le cube est mis à droite et tourne autour de son axe z */
  for(i = -bH / 2; i < bH / 2; ++i)
    for(j = -bW / 2; j < bW / 2; ++j) {
       /* vide, pas de bloc mur */
      if(_board[(i + bH / 2) * bW + j + bW / 2] == 0) continue;
        else if(_board[(i + bH / 2) * bW + j + bW / 2] == 1)
      {
        _shape->dcolor = red;
        memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
        translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
        translate(nmv, 0.0f, 1.0f, 0.0f);
        transform_n_raster(_shape, nmv, projMat);
      }
      else if(_board[(i + bH / 2) * bW + j + bW / 2] == 2)
      {
        _shape->dcolor = blue;
        memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
        translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
        translate(nmv, 0.0f, 1.0f, 0.0f);
        transform_n_raster(_shape, nmv, projMat);
      }
      else if(_board[(i + bH / 2) * bW + j + bW / 2] == 3)
      {
        _shape->dcolor = purple;
        memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
        translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
        translate(nmv, 0.0f, 1.0f, 0.0f);
        transform_n_raster(_shape, nmv, projMat);
      }
      else if(_board[(i + bH / 2) * bW + j + bW / 2] == 4)
      {
        _shape->dcolor = platinium;
        memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
        translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
        translate(nmv, 0.0f, 1.0f, 0.0f);
        transform_n_raster(_shape, nmv, projMat);
      }
      else if(_board[(i + bH / 2) * bW + j + bW / 2] == 5)
      {
        _shape->dcolor = green;
        memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
        translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
        translate(nmv, 0.0f, 1.0f, 0.0f);
        transform_n_raster(_shape, nmv, projMat);
      }
      else if(_board[(i + bH / 2) * bW + j + bW / 2] == 6)
      {
        _shape->dcolor = rust;
        memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
        translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
        translate(nmv, 0.0f, 1.0f, 0.0f);
        transform_n_raster(_shape, nmv, projMat);
      }
      else
      {
      _cube->dcolor = pink;
      //faire une fn pour savoir la case 
      memcpy(nmv, mvMat, sizeof nmv); /* copie mvMat dans nmv */
      translate(nmv, 2 * j, 0, 2 * i); /* pourquoi *2 ? */
      translate(nmv, 0.0f, 1.0f, 0.0f);
      transform_n_raster(_cube, nmv, projMat);
    }
    }
  /* déclarer qu'on a changé (en bas niveau) des pixels du screen  */
  gl4dpScreenHasChanged();
  /* fonction permettant de raffraîchir l'ensemble de la fenêtre*/
  gl4dpUpdateScreen(NULL);
}

/*!\brief intercepte l'événement clavier pour modifier les options. */
void key(int keycode) {
  
  switch(keycode) {
  case GL4DK_UP:
    up();
    break;
  case GL4DK_DOWN:
    down();
    break;
  case GL4DK_LEFT:
    left();
    break;
  case GL4DK_RIGHT:
    right();
    for(int i = -bH / 2; i <= bH / 2; ++i){
    for(int j = -bW / 2; j <= bW / 2; ++j)
    {
      if(_board[(i + bH / 2) * bW + j + bW / 2] == -1){
        _board[(i + bH / 2) * bW + j + bW / 2] = 1;
      }
      printf("%d",_board[(i + bH / 2) * bW + j + bW / 2]);
    }
    printf("\n");
   }
    break;

  case GL4DK_d:

      eyeX ++;
      eyeY --;
      centerZ --;

      //centerY --;  
      //eyeZ ++;
      //centerX ++;
      //centerZ ++;
      //angle --;
      break;
    case GL4DK_q:
      eyeX--;
      eyeY++;
    break;
    case GL4DK_z:
      eyeX ++;
      centerX --;
    break;
    case GL4DK_s:
      eyeY--;
      centerY++;
      printf(" eye y = %f,centre y = %f",eyeY,centerY);
    break;

  case GL4DK_t: /* 't' la texture */
    _use_tex = !_use_tex;
    if(_use_tex) {
      enableSurfaceOption(  _quad, SO_USE_TEXTURE);
      enableSurfaceOption(  _cube, SO_USE_TEXTURE);
    } else {
      disableSurfaceOption(  _quad, SO_USE_TEXTURE);
      disableSurfaceOption(  _cube, SO_USE_TEXTURE);
    }
    break;
  case GL4DK_c: /* 'c' utiliser la couleur */
    _use_color = !_use_color;
    if(_use_color) {
      enableSurfaceOption(  _quad, SO_USE_COLOR);
      enableSurfaceOption(  _cube, SO_USE_COLOR);
    } else { 
      disableSurfaceOption(  _quad, SO_USE_COLOR);
      disableSurfaceOption(  _cube, SO_USE_COLOR);
    }
    break;
  case GL4DK_l: /* 'l' utiliser l'ombrage par la méthode Gouraud */
    _use_lighting = !_use_lighting;
    if(_use_lighting) {
      enableSurfaceOption(  _quad, SO_USE_LIGHTING);
      enableSurfaceOption(  _cube, SO_USE_LIGHTING);
    } else { 
      disableSurfaceOption(  _quad, SO_USE_LIGHTING);
      disableSurfaceOption(  _cube, SO_USE_LIGHTING);
    }
    break;
  default: break;
  }
}

/*!\brief à appeler à la sortie du programme. */
void sortie(void) {
  /* on libère nos trois surfaces */
  if(_quad) {
    freeSurface(_quad);
    _quad = NULL;
  }
  if(_cube) {
    freeSurface(_cube);
    _cube = NULL;
  }
  if(_shape) {
    freeSurface(_shape);
    _shape = NULL;
  }
  /* libère tous les objets produits par GL4Dummies, ici
   * principalement les screen */
  gl4duClean(GL4DU_ALL);
}
