#include "../include/config.h"
#include <stdio.h>
#include <math.h>

double analytics_statistics[NUM_BLOCKS][NUM_METRICS];

int fattoriale(int n){
    int fatt=1;
    if(n > 0){
        int i=1;
        while(i <= n){
            fatt=fatt*i;
            i = i + 1;
        }
    }
    return fatt;
}

//funzione t risposta M/M/1
double rtime_mm1(double lambda, double mu){
    double Etq;
    double rho = (lambda/mu);

    Etq = (rho*(1/mu)/(1-rho));

    return Etq + (1/mu);
}

//funzione t risposta M/M/2/2 e M/M/INF
double rtime_mmkk(double mu){
    return 1/mu;
}

//t risposta M/M/k
double rtime_mmk(double lambda, double mu, double k){
    int i;
    double P0 = 0.0;
    double Pq;
    double rho = lambda/(k*mu);
    printf("rho = %f\n", rho);
    for(i=0; i<k; i++){
        double p = pow(k*rho,i);
        double f = fattoriale(i);
        P0+=p/f;
    }
    double pk = pow(k*rho,k);
    double fk = fattoriale(k);
    P0+= pk/(fk*(1.0-rho));
    double invP0 = 1.0/P0;
    printf("il valore di P0 è %f\n", invP0);

    Pq = (invP0*pk)/(fk*(1.0-rho));
    printf("il valore di Pq è %f\n", Pq);

    double etq = (Pq*(1.0/(k*mu))/(1.0-rho));
    printf("il valore di E[tq] è %f\n", etq);

    return etq +(1.0/(mu));
}


int main(int argc, char** argv){
    printf("mmk = %f\n", rtime_mmk(4.0,1.5,4.0));
    return 0;
}


void calculate_statistic() {

    for(int i=0;i<NUM_BLOCKS;i++) {
        

    }
}