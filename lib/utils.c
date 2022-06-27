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
<<<<<<< HEAD
=======
void print_line();
void *append_on_csv(FILE *fpt, double ts);
void *append_on_csv_v2(FILE *fpt, double ts, double p);
>>>>>>> 02b49a198f2d8c329abee79432da9f4837775fca

double E(int c, double a)
{
  double  e_last;        // Last E() value
  if (c == 0)
    return(1.0);
  else
    return((a * (e_last = E(c - 1, a))) / (c + a * e_last));
}



// Inserisce una nuova linea nel file csv specificato
void *append_on_csv(FILE *fpt, double ts) {
    fprintf(fpt, "%2.6f\n", ts);
    return fpt;
}

// Inserisce una nuova linea nel file csv specificato
void *append_on_csv_v2(FILE *fpt, double ts, double p) {
    fprintf(fpt, "%2.6f; %2.6f\n", ts, p);
    return fpt;
}
// Apre un file csv e ritorna il puntatore a quel file



