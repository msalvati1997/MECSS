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

// Ritorna il nome del blocco passando il suo identificativo
char *stringFromEnum(int f) {
    char *strings[] = {"CONTROL_UNIT", "VIDEO_UNIT", "WLAN_UNIT", "ENODE_UNIT", "EDGE_UNIT","CLOUD_UNIT"};
    return strings[f];
}


