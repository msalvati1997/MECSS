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

double util_mm1(double lambda, double mu){
    return lambda/mu;
}

double util_mmk(double lambda, double mu, int k){
    return lambda/(k*mu);
}

double util_mmkk(double lambda, double mu, int k){
    double Ploss = 0.0;
    double den = 0.0;
    double num = pow(lambda/mu, k)/fattoriale(k);

    for(int i= 0; i<(k+1); i++){
        den += pow(lambda/mu, i)/fattoriale(i);
    }

    Ploss = num/den;

    printf("PLoss: %f\n", Ploss);
    
    return (lambda*(1.0-Ploss))/(k*mu);
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

void add_stats(){

//blocco control unit
analytics_statistics[0][0] = rtime_mm1(ARRIVAL_RATE,1.0/CONTROL_UNIT_SERVICE_TIME);
analytics_statistics[0][1] = util_mm1(ARRIVAL_RATE,1.0/CONTROL_UNIT_SERVICE_TIME);

//blocco video
analytics_statistics[1][0] = rtime_mmkk(1.0/VIDEO_SERVICE_TIME);
analytics_statistics[1][1] = util_mmkk(TH_VIDEO,1.0/CONTROL_UNIT_SERVICE_TIME,2);

//blocco wifi
analytics_statistics[2][0] = rtime_mmk(TH_WLAN,1.0/WLAN_FRAME_UPLOAD_TIME,2);
analytics_statistics[2][1] = util_mmk(TH_WLAN,1.0/WLAN_FRAME_UPLOAD_TIME,2);

//blocco rete cellulare
analytics_statistics[3][0] = rtime_mm1(TH_ENODE,1.0/ENODE_FRAME_UPLOAD_TIME);
analytics_statistics[3][1] = util_mm1(TH_ENODE,1.0/ENODE_FRAME_UPLOAD_TIME);

//blocco edge
analytics_statistics[4][0] = rtime_mmk(TH_EDGE,1.0/EDGE_PROCESSING_TIME,4);
analytics_statistics[4][1] = util_mmk(TH_EDGE,1.0/EDGE_PROCESSING_TIME,4);

//blocco cloud (utilizzazione tende a 0 al crescere di k serventi)
analytics_statistics[5][0] = rtime_mmkk(1.0/WLAN_FRAME_UPLOAD_TIME);
analytics_statistics[5][1] = 0;

}

void print_stats(){

    printf("Tempo risp control unit: %f\n", analytics_statistics[0][0]);
    printf("Tempo risp video unit: %f\n", analytics_statistics[1][0]);
    printf("Tempo risp wlan unit: %f\n", analytics_statistics[2][0]);
    printf("Tempo risp enode unit: %f\n", analytics_statistics[3][0]);
    printf("Tempo risp edge unit: %f\n", analytics_statistics[4][0]);
    printf("Tempo risp cloud unit: %f\n", analytics_statistics[5][0]);

    printf("Utilizzazione control unit: %f\n", analytics_statistics[0][1]);
    printf("Utilizzazione video unit: %f\n", analytics_statistics[1][1]);
    printf("Utilizzazione wlan unit: %f\n", analytics_statistics[2][1]);
    printf("Utilizzazione enode unit: %f\n", analytics_statistics[3][1]);
    printf("Utilizzazione edge unit: %f\n", analytics_statistics[4][1]);
    printf("Utilizzazione cloud unit: %f\n", analytics_statistics[5][1]);

}


int main(int argc, char** argv){
    add_stats();
    print_stats();
    return 0;
}


void calculate_statistic() {

    for(int i=0;i<NUM_BLOCKS;i++) {
        

    }
}