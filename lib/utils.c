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
    fprintf(fpt, "%2.6f , %2.6f, %2.6f\n", ts[0], ts[1],ts[2]);
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

void *append_on_csv_delay(FILE *fpt, double ts, int batch, int block) {
    fprintf(fpt, "%d , %d , %2.6f \n", batch, block, ts);
    
    return fpt;
}
void *append_on_csv_batch(FILE *fpt, double *ts, int batch) {
     fprintf(fpt, "%d , %2.6f ,%2.6f , %2.6f \n",  batch, ts[2] ,ts[0], ts[1]);
    return fpt;
}
void *append_on_csv_batch_find(FILE *fpt, double *ts, int batch) {
     fprintf(fpt, "%2.6f \n", ts[0]);
    return fpt;
}
