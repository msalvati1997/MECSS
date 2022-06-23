#include "config.h"
#include <math.h>

//funzione t risposta M/M/1
double rtime_mm1(double lambda, double mu){
    double Etq;
    double rho = (lambda/mu);

    Etq = (rho*(1/mu)/(1-rho));

    return Etq + (1/mu);
}

//funzione t risposta M/M/2/2
double rtime_mm22(double mu){
    return 1/mu;
}

//t risposta M/M/k
double rtime_mmk(double lambda, double mu, int k){
    int i;
    double P0;
    double Pq;
    double rho = lambda/mu;
    for(i=0; i<k; i++){
        P0+=pow(k*rho,i)/(fattoriale(i));
    }
    P0+= pow(k*rho,k)/(fattoriale(k)*(1-rho));
}

//t risposta M/M/INF
double rtime_mminf(double mu){
    return 1/mu; //no tempo di attesa
}

int fattoriale(int n){
    int fatt=1;
    if(n > 0){
        int i=1;
        while(i < n){
            fatt=fatt*i;
            i = i + 1;
        }
    }
    return fatt;
}

