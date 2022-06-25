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
  double     e_last;        // Last E() value

  if (c == 0)
    return(1.0);
  else
    return((a * (e_last = E(c - 1, a))) / (c + a * e_last));
}

// Stampa a schermo una linea di separazione
void print_line() {
    printf("\n————————————————————————————————————————————————————————————————————————————————————————\n");
}
// Apre un csv in modalità append
FILE *open_csv_appendMode(char *filename) {
    FILE *fpt;
    fpt = fopen(filename, "a");
    return fpt;
}

// Inserisce una nuova linea nel file csv specificato
void *append_on_csv(FILE *fpt, double ts, double p) {
    fprintf(fpt, "%2.6f\n", ts);
    return fpt;
}

// Inserisce una nuova linea nel file csv specificato
void *append_on_csv_v2(FILE *fpt, double ts, double p) {
    fprintf(fpt, "%2.6f; %2.6f\n", ts, p);
    return fpt;
}
// Apre un file csv e ritorna il puntatore a quel file



