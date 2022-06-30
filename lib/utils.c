#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//===========================================================================
//=  Recursive function to solve for Erlang-B blocking probability          =
//=   - Input is c = number of servers and a = total offered load           =
//=   - Output returned is Erlang-B blocking probability                    =
//===========================================================================

double E(int c, double a)
{
  double  e_last;        // Last E() value
  if (c == 0)
    return(1.0);
  else
    return((a * (e_last = E(c - 1, a))) / (c + a * e_last));
}



// Inserisce una nuova linea nel file csv specificato
void *append_on_csv(FILE *fpt, double *ts) {
    fprintf(fpt, "%2.6f, %2.6f, %2.6f\n", ts[0], ts[1],ts[2]);
    return fpt;
}

// Inserisce una nuova linea nel file csv specificato
void *append_on_csv_loss(FILE *fpt, double *ts) {
    for(int i=0; i<128;i++) {
          fprintf(fpt, "%2.6f \n", ts[i]);
    }
    return fpt;
}

// Stampa una barra di avanzamento relativa alla singola run di simulazione
void print_percentage(double part, double total, double oldPart) {
    double percentage = part / total * 100;
    double oldPercentage = oldPart / total * 100;

    if ((int)oldPercentage == (int)percentage) {
        return;
    }
    printf("\rSimulation Progress: |");
    for (int i = 0; i <= percentage / 2; i++) {
        printf("█");
        fflush(stdout);
    }
    for (int j = percentage / 2; j < 50 - 1; j++) {
        printf(" ");
    }
    printf("|");
    printf(" %02.0f%%", percentage + 1);
}

void *append_on_csv_delay(FILE *fpt, double ts) {
     for(int i=0; i<6;i++) {
          fprintf(fpt, "%2.6f \n", ts);
    }
    return fpt;
}



// Inserisce una nuova linea nel file csv specificato
void *append_on_csv3(FILE *fpt, double **ts, int rep) {
    for(int i=0;i<6;i++) {
      fprintf(fpt, "sim n. %d ;%2.6f; %2.6f; %2.6f; %2.6f;%2.6f; %2.6f;%2.6f; %2.6f;%2.6f; %2.6f;%2.6f\n",rep, ts[i][0],ts[i][1],ts[i][2],ts[i][3],ts[i][4],ts[i][5],ts[i][6],ts[i][7],ts[i][8],ts[i][9],ts[i][10]);
    }
    return fpt;
}

