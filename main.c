#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>	

int * pointer;
int rng,acc;
pthread_t tid[2];

typedef struct Bloc Bloc;
struct Bloc 
{
    int **tableaux;
    int taille_du_carre,row,col;
};
Bloc current;

typedef struct Dimension Dimension;
struct Dimension 
{
    int w;
    int h;
};
Dimension Dim;

const Bloc BlocTableaux[24] = {
    
    {(int *[]){(int []){0,1,1},(int []){1,1,0}, (int []){0,0,0}}, 3},                                //S_shape     
    {(int *[]){(int []){2,2,0},(int []){0,2,2}, (int []){0,0,0}}, 3},                               //Z_shape     
    {(int *[]){(int []){0,3,0},(int []){0,3,3}, (int []){0,3,0}}, 3},                              //T_shape     
    {(int *[]){(int []){0,4,0},(int []){0,4,0}, (int []){0,4,4}}, 3},                             //L_shape        
    {(int *[]){(int []){5,5},(int []){5,5}}, 2},                                                 //SQ_shape
    {(int *[]){(int []){0,0,0,0}, (int []){6,6,6,6}, (int []){0,0,0,0}, (int []){0,0,0,0}}, 4}, //I_shape

    // first rotation

    {(int *[]){(int []){0,0,1},(int []){0,1,1}, (int []){0,1,0}}, 3},                                //S_shape     
    {(int *[]){(int []){0,0,2},(int []){0,2,2}, (int []){0,2,0}}, 3},                               //Z_shape     
    {(int *[]){(int []){0,3,0},(int []){3,3,3}, (int []){0,0,0}}, 3},                              //T_shape     
    {(int *[]){(int []){0,0,4},(int []){4,4,4}, (int []){0,0,0}}, 3},                             //L_shape        
    {(int *[]){(int []){5,5},(int []){5,5}}, 2},                                                 //SQ_shape
    {(int *[]){(int []){0,6,0,0}, (int []){0,6,0,0}, (int []){0,6,0,0}, (int []){0,6,0,0}}, 4}, //I_shape

    //second rotation

    {(int *[]){(int []){0,1,1},(int []){1,1,0}, (int []){0,0,0}}, 3},                                //S_shape     
    {(int *[]){(int []){2,2,0},(int []){0,2,2}, (int []){0,0,0}}, 3},                               //Z_shape     
    {(int *[]){(int []){3,3,3},(int []){0,3,0}, (int []){0,0,0}}, 3},                              //T_shape     
    {(int *[]){(int []){0,0,0},(int []){4,4,4}, (int []){4,0,0}}, 3},                             //L_shape        
    {(int *[]){(int []){5,5},(int []){5,5}}, 2},                                                 //SQ_shape
    {(int *[]){(int []){0,0,0,0}, (int []){6,6,6,6}, (int []){0,0,0,0}, (int []){0,0,0,0}}, 4}, //I_shape

    //third rotation

    {(int *[]){(int []){0,1,1},(int []){1,1,0}, (int []){0,0,0}}, 3},                                //S_shape     
    {(int *[]){(int []){2,2,0},(int []){0,2,2}, (int []){0,0,0}}, 3},                               //Z_shape     
    {(int *[]){(int []){0,3,0},(int []){3,3,0}, (int []){0,3,0}}, 3},                              //T_shape     
    {(int *[]){(int []){0,4,0},(int []){0,4,0}, (int []){4,4,0}}, 3},                             //L_shape        
    {(int *[]){(int []){5,5},(int []){5,5}}, 2},                                                 //SQ_shape
    {(int *[]){(int []){0,0,0,0}, (int []){6,6,6,6}, (int []){0,0,0,0}, (int []){0,0,0,0}}, 4}, // I

};

Bloc CopyBloc(Bloc bloc)
{
    Bloc new_bloc = bloc;
    int **copybloc = bloc.tableaux;
    new_bloc.tableaux = (int**)malloc(new_bloc.taille_du_carre*sizeof(int*));
    int i, j;
    for(i = 0; i < new_bloc.taille_du_carre; i++){
        new_bloc.tableaux[i] = (int*)malloc(new_bloc.taille_du_carre*sizeof(int));
        for(j=0; j < new_bloc.taille_du_carre; j++) {
            new_bloc.tableaux[i][j] = copybloc[i][j];
        }
    }
    return new_bloc;
};
void check_haut(int b[])
{
    if(b[current.row * Dim.w + current.col] != 0)
    {
        printf("merci d'avoir jouer ");
        //printf("a = %d ", a);
        exit(1);
    }
}

void check(int b[])
{
    int check_acc;
    int i,j,k,l;
    //2 premier for pour parcourir le tableaux 2d
    for (i = 0; i < Dim.h; i++)
    {

        check_acc = 0;
        for (j = 0; j < Dim.w; j++)
        {   
            //si la case est occuper 1 oui 0 vide
            if (b[i * Dim.w + j] >= 1)
            {
                check_acc ++;
            }

            //si toute la ligne est occuper
            if (check_acc == Dim.w)
            {   
                //parours de la ligne en question est mise a zero
                for(k = 0;k < Dim.w; k++)
                {
                    b[i * Dim.w +  k] = 0;
                }
                //a partir de la ligne de zero on remonte e tableaux et on vaisse toute les valeur de 1 case
                for(l = i;l >= 1;l--)
                {
                    for(k = 0;k < Dim.w;k++)
                    {
                        b[l * Dim.w +  k] = b[(l - 1) * Dim.w +  k];

                    }
                }
            }
        }
    }

}
void DeleteBloc(Bloc bloc)
{
    int i;
    for(i = 0; i < bloc.taille_du_carre; i++){
        free(bloc.tableaux[i]);
    }
    free(bloc.tableaux);
};

void GetNewBloc_rotate(int a)
{
    Bloc new_bloc = CopyBloc(BlocTableaux[a]);
    new_bloc.col = current.col;
    new_bloc.row = current.row;
    DeleteBloc(current);
    current = new_bloc;

};

void rotation()
{
    acc++;

    if (acc == 1)
    {
        GetNewBloc_rotate(rng + 6);
    }
    if (acc == 2)
    {
     GetNewBloc_rotate(rng  + (6 * (acc)));   
    }
    if (acc == 3)
    {
    GetNewBloc_rotate(rng + (6 * (acc)));
    acc = 0;
    }

}

void clear_tab_position(int b[], Bloc temps)
{

    int i,j;
    for(i = 0; i < temps.taille_du_carre ;i++){
        for(j = 0; j < temps.taille_du_carre ; j++){
            if(temps.tableaux[i][j])
                b[(temps.row+i) * Dim.w + (temps.col+j)] = 0;
        }
    }

};

int test_move(Bloc temp)
{
    int i,j;

    for(i = 0; i < temp.taille_du_carre;i++)
    {  
        for(j = 0; j < temp.taille_du_carre;j++)
        {
            if (temp.row + i >= Dim.h || temp.col + j >= Dim.w || temp.col + j < 0)
            {
                if (temp.tableaux[i][j])
                {
                    return 0; 
                }
            }
            else if(pointer[(temp.row + i) * Dim.w + (temp.col + j)] && temp.tableaux[i][j])
                return 0;  
        }
    }
    return 1;

};

void send_to_tab(int b[])
{
    int i,j;
    for(i = 0; i < current.taille_du_carre ;i++){
        for(j = 0; j < current.taille_du_carre ; j++){
            if(current.tableaux[i][j])
                b[(current.row+i) * Dim.w + (current.col+j)] = current.tableaux[i][j];
        }
    }
};


void GetNewBloc()
{
    srand(time(NULL));
    rng = rand() % 7;
    Bloc new_bloc = CopyBloc(BlocTableaux[rng]);
    new_bloc.col = 1;
    new_bloc.row = 0;
    DeleteBloc(current);
    current = new_bloc;
    check_haut(pointer);
    acc = 0;
};

void down()
{
    clear_tab_position(pointer,current);
    current.row ++;  //move down
    if(!test_move(current))
    {
        current.row --;
        send_to_tab(pointer);
        check(pointer);
        GetNewBloc();
    }
    send_to_tab(pointer);

};
void* timer_thread()
{
    pthread_t id = pthread_self();
    while (true) {
        down();
        sleep(1);

    }


    return NULL;
}
void up()
{
    clear_tab_position(pointer,current);
    rotation();
    send_to_tab(pointer);
};

void left()
{
    clear_tab_position(pointer,current);
    current.col --;
    if(!test_move(current))
    {
        current.col ++;
    }
    send_to_tab(pointer);
    
};

void right()
{
    clear_tab_position(pointer,current);
    current.col ++;
    if(!test_move(current))
    {
        current.col --;
    }
    send_to_tab(pointer);
};

unsigned int * board(int w, int h)
{
    int i,j;
    Dim.w = w;
    Dim.h = h; 
    int * board = w * h - 1;
    board = malloc(w * h * sizeof *board);
    pointer = board;
    GetNewBloc();
    for (i = 0; i < h; ++i) 
    {
        for (j = 0; j < w; ++j)
        {
            board[i * Dim.w + j] = 0;
        }
    }
    send_to_tab(board);
    int id = 0;
    pthread_create(&(tid[id]), NULL, &timer_thread, NULL);
    return (unsigned int*)board;
};
