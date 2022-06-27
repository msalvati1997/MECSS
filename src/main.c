#include "../include/config.h"
#include "../include/rngs.h"
#include "../include/rvgs.h"
#include "../include/rvms.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>          // Needed for printf()
#include <stdlib.h>  

/*@todo 
COSE DA FARE:
- funzione che calcola tempo totale teorico
- funzione che calcola dispendio energetico teorico 
- funzione che calcola tempo totale  
- aggiungere funzione calcolo dispendio energetico
- aggiungere cambiamento seed ad ogni simulazione  
*/
// Genera un tempo di arrivo secondo la distribuzione Esponenziale
struct clock_t clock;                          // Mantiene le informazioni sul clock di simulazione

FILE *open_csv(char *filename) {
    FILE *fpt;
    fpt = fopen(filename, "w+");
    return fpt;
}

char* stringFromEnum(int f) {

    char *strings[6]= {"CONTROL_UNIT", "VIDEO_UNIT", "WLAN_UNIT", "ENODE_UNIT", "EDGE_UNIT","CLOUD_UNIT"};
    return strings[f];
}


char* stringFromEnum2(int f) {
    if(f==6) {
        return "EXTERNAL";
    } else {
        return "INTERNAL";
    }
    return "";
}

double getArrival(double current) {
    double arrival = current;
    SelectStream(254);
    arrival += Exponential(ARRIVAL_RATE);
    printf("GENERATO NUOVO ARRIVO : %f s\n", arrival);
    return arrival;
}

// Genera un tempo di servizio esponenziale di media specificata e stream del servente individuato
double getService(int type_service, int stream) {
    SelectStream(stream);
    

    switch (type_service) {
        case 0:
            return Exponential(CONTROL_UNIT_SERVICE_TIME);
        case 1:
            return Exponential(VIDEO_SERVICE_TIME);
        case 2:
            return Exponential(WLAN_FRAME_UPLOAD_TIME);
        case 3:
            return Exponential(ENODE_FRAME_UPLOAD_TIME);
        case 4:
            return Exponential(EDGE_PROCESSING_TIME);
        case 5:
            return Exponential(CLOUD_PROCESSING_TIME);; 
        default:
            printf("Case not defined\n");
            return 0;
    }
}

// Inserisce un job nella coda del blocco specificata
void enqueue(block *block_t, double arrival, int type) {
    job *j = (job *)malloc(sizeof(job));
    if (j == NULL)
        handle_error("malloc");

    j->arrival = arrival;
    j->next = NULL;
    j->type = type;
    if (block_t->tail){  // Appendi alla coda se esiste, altrimenti è la testa
        printf("JOB PRESENTI IN CODA DI %s APPENDO ALLA CODA\n",stringFromEnum(block_t->type));
        block_t->tail->next = malloc(sizeof(job));
        block_t->tail->next = j; 
    }else{
        printf("JOB NON PRESENTI IN CODA DI %s- APPENDO IN TESTA\n", stringFromEnum(block_t->type));
        block_t->head_service=malloc(sizeof(job));
        block_t->head_service = j;
    }
    
    block_t->tail = j;

    if (block_t->head_queue == NULL) {
        block_t->head_queue = j;
    }
    printf("PRINT QUEUE AFTER ENQUEUE\n");
    printQUeue(block_t->head_queue);
}

void printQUeue(job *j) {
    job *tmp = (job *)malloc(sizeof(job));
    tmp=j;
    printJobInfo(tmp);
    while(tmp->next!=NULL) {
        tmp = tmp->next;
        printJobInfo(tmp);
    }
}

// Rimuove il job dalla coda del blocco specificata
int dequeue(block *block) {
    job *j = block->head_service;
    printf("PRINT QUEUE BEFORE DEQUEUE\n");
    printQUeue(block->head_queue);
    int type = j->type;

    if (!j->next)
        block->tail = NULL;

    block->head_service = j->next;

    if (block->head_queue != NULL && block->head_queue->next != NULL) {
        job *tmp = block->head_queue->next;
        block->head_queue = tmp;
        printf("PRINT QUEUE AFTER DEQUEUE\n");
        printQUeue(block->head_queue);
    } else {
        block->head_queue = NULL;
        printf("--------->EMPTY QUEUE\n");
    }

  
    return type;
}


void printJobInfo(job * j) {  
    printf("---------> [JOB : ARRIVO %f, NEXT=NULL, TYPE : %s ]\n", j->arrival,stringFromEnum2(j->type));
}

// Ritorna il primo server libero nel blocco specificato
server *findFreeServer(int block_type) {
    block * b = &blocks[block_type];
    int num = b->num_servers;
    //printf("num server %d\n", num);
    //printf("type %d\n", block_type);
    for (int i = 0; i <  num; i++) {
        //printf("STATUS OF SERVER %d / %d \n", i, (*b->serv+i)->status);
        if((*b->serv+i)->status==IDLE){
            printf("SERVER IDLE FOUND IN %d - %s\n", b->type, stringFromEnum(b->type));
            return *b->serv+i;
        }
    }
    printf("SERVER IDLE NOT FOUND IN %s\n", stringFromEnum(b->type));
    return NULL;
}

// Processa un arrivo dall'esterno verso il sistema
void process_arrival() {
    print_line();
    printf("PROCESSO DI UN ARRIVO DALL'ESTERNO DEL SISTEMA\n");
    blocks[0].total_arrivals++;
    blocks[0].jobInBlock++;

    server *s = findFreeServer(0);
    // C'è un servente libero, quindi genero il completamento
    if (s != NULL) {
        double serviceTime = getService(CONTROL_UNIT, s->stream);
        printf("TEMPO DI PROCESSAMENTO IN %s GENERATO: %f\n", stringFromEnum(0), serviceTime);
        compl c = {s, INFINITY};
        s->block=&blocks[0];
        c.server=s;
        c.value = clock.current + serviceTime;
        s->status = BUSY;  // Setto stato busy
        s->sum.service += serviceTime;
        s->block->area.service += serviceTime;
        s->sum.served++;
        insertSorted(&global_sorted_completions, c);
        enqueue(&blocks[0], clock.arrival,EXTERNAL);  // lo appendo nella linked list di job del blocco 
    } else {
        enqueue(&blocks[0], clock.arrival,EXTERNAL);  // lo appendo nella linked list di job del blocco 
        blocks[0].jobInQueue++;              // Se non c'è un servente libero aumenta il numero di job in coda
    }
    clock.arrival = getArrival(clock.current);  // Genera prossimo arrivo
}

// Processa un next-event di completamento
void process_completion(compl c) {
    print_line();
    block *block_ = (c.server)->block;
    int block_type = (block_)->type;
    blocks[block_type].total_completions++;
    blocks[block_type].jobInBlock--;
    int type;
    int destination;
    server *freeServer;
    printf("BLOCCO DI PROCESSAMENTO DI UN NEXT EVENT IN : %s\n", stringFromEnum(block_type));
    printf("DIMINUISCO IL NUMERO DI JOB NEL BLOCCO IN QUANTO UN JOB VIENE SERVITO : jobInBlock= %d\n",blocks[block_type].jobInBlock);
    type = dequeue(&blocks[block_type]);  // Toglie il job servito dal blocco e fa "avanzare" la lista collegata di job
    deleteElement(&global_sorted_completions, c);
    // Se nel blocco ci sono job in coda, devo generare il prossimo completamento per il servente che si è liberato.
    if (blocks[block_type].jobInQueue > 0 && !c.server->need_resched) {
        printf("N. JOB IN CODA PRESENTI NEL BLOCCO %s: %d\n",stringFromEnum(block_type), blocks[block_type].jobInQueue);
        printf("QUINDI GENERO IL PROSSIMO COMPLETAMENTO\n");
        blocks[block_type].jobInQueue--;
        printf("TOLGO UN JOB DALLA CODA E LO PROCESSO NEL BLOCCO\n");
        double service_1 = getService(block_type, c.server->stream);
        printf("TEMPO DI PROCESSAMENTO IN %s GENERATO: %f\n", stringFromEnum(block_type), service_1);
        c.value = clock.current + service_1;
        c.server->sum.service += service_1;
        c.server->sum.served++;
        c.server->block->area.service += service_1;
        insertSorted(&global_sorted_completions, c);
    } else {
        if(block_type==CLOUD_UNIT && !c.server->need_resched) {
            printf("CLOUD NON HA CODA - E' UN M/M/INF\n");
            c.server->status=IDLE;
        }
        if(block_type==VIDEO_UNIT && !c.server->need_resched) {
            printf("VIDEO UNIT NON HA CODA\n");
            c.server->status=IDLE;
        }
        else {
          printf("JOB NON PRESENTI IN CODA - STATUS TO IDLE\n");
           c.server->status = IDLE;
        }
    }

    // Se un server è schedulato per la terminazione, non prende un job dalla coda e và OFFLINE
    if (c.server->need_resched) {
        printf("-------SERVER SCHEDULATO PER LA TERMINAZIONE ----------\n"); 
        c.server->online = OFFLINE;
        c.server->need_resched = false;
    }

    //uscita dalla rete se il job esce dal CLOUD
    if (block_type == CLOUD_UNIT) {
        printf("-----------JOB EXIT FROM CLOUD----------------\n");
        completed++;
        return;
    }

    // Gestione blocco destinazione job 
    destination = getDestination(c.server->block->type,type);  // Trova la destinazione adatta per il job appena servito 
    blocks[destination].total_arrivals++;
    printf("FROM %s TO %s\n", stringFromEnum(block_type), stringFromEnum(destination));
    if (destination == EXIT) {
        blocks[block_type].total_dropped++;
        dropped++;
        return;
    }
    if (destination == CLOUD_UNIT) { //M/M/INF non accoda mai, come se i server fossero sempre liberi
            server * cloud_server = *(blocks[CLOUD_UNIT].serv);
            blocks[destination].jobInBlock++;
            enqueue(&blocks[destination], c.value,INTERNAL);
            compl c2 = {cloud_server, INFINITY};
            double service_2 = getService(CLOUD_UNIT, cloud_server->stream);
            printf("TEMPO DI PROCESSAMENTO IN %s GENERATO: %f\n", stringFromEnum(destination) ,service_2);
            c2.value = clock.current + service_2;
            c2.server=cloud_server;
            cloud_server->sum.service += service_2;
            cloud_server->sum.served++;
            cloud_server->block->area.service += service_2; 
            insertSorted(&global_sorted_completions, c2);
            return;
    }
    if (destination != CLOUD_UNIT && destination != VIDEO_UNIT) {
        enqueue(&blocks[destination], c.value,INTERNAL);
        blocks[destination].total_arrivals++;
        blocks[destination].jobInBlock++;
        // Se il blocco destinatario ha un servente libero, generiamo un tempo di completamento, altrimenti aumentiamo il numero di job in coda
        freeServer = findFreeServer(destination);
        if (freeServer != NULL) {
            freeServer->block=&blocks[destination];
            compl c3 = {freeServer, INFINITY};
            double service_3 = getService(destination, freeServer->stream);
            printf("TEMPO DI PROCESSAMENTO IN %s GENERATO: %f\n", stringFromEnum(destination) ,service_3);
            c3.server=freeServer;
            c3.value = clock.current + service_3;
            freeServer->status = BUSY;
            freeServer->sum.service += service_3;
            freeServer->sum.served++;
            freeServer->block->area.service += service_3;
            insertSorted(&global_sorted_completions, c3);
            return;
        } else {
                  printf("NUMERO DI JOB IN CODA AUMENTA NEL BLOCCO %s\n", stringFromEnum(blocks[destination].type));
                  blocks[destination].jobInQueue++; 
                  return;
            }
        }
     if(destination == VIDEO_UNIT) {
        blocks[destination].total_arrivals++;
        // Se il blocco destinatario ha un servente libero, generiamo un tempo di completamento, altrimenti aumentiamo il numero di job in coda
        freeServer = findFreeServer(destination);
        if (freeServer != NULL) {
            blocks[destination].jobInBlock++;
            enqueue(&blocks[destination], c.value,INTERNAL);
            freeServer->block=&blocks[destination];
            compl c3 = {freeServer, INFINITY};
            double service_3 = getService(destination, freeServer->stream);
            printf("TEMPO DI PROCESSAMENTO IN %s GENERATO: %f\n", stringFromEnum(destination) ,service_3);
            c3.server=freeServer;
            c3.value = clock.current + service_3;
            freeServer->status = BUSY;
            freeServer->sum.service += service_3;
            freeServer->sum.served++;
            freeServer->block->area.service += service_3;
            insertSorted(&global_sorted_completions, c3);
            return;
        } else {
                 printf("JOB BYPASSED FROM VIDEO UNIT\n");
                 completed++;
                 bypassed++;
                 blocks[destination].total_bypassed++;
                 return;
            }
        }
}

// Ritorna il blocco destinazione di un job dopo il suo completamento
int getDestination(enum block_types from, int type) {
    switch (from) {
        case CONTROL_UNIT:
            if(type==EXTERNAL) {  //external  
                printf("JOB SERVITO EXTERNAL -> DIRECTED TO VIDEO UNIT\n");
                return VIDEO_UNIT;
            }
            if(type==INTERNAL) { //internal 
                printf("JOB  SERVITO INTERNAL -> DIRECTED TO CLOUD\n");
                int ret = routing_from_control_unit();
                printf("ROUTING FROM CONTROL UNIT TO %s\n", stringFromEnum(ret));
                return ret;
            } else {
                return NULL;
            }
        case VIDEO_UNIT:
            return CONTROL_UNIT;
        case WLAN_UNIT:
            return EDGE_UNIT;
        case ENODE_UNIT:
            return EDGE_UNIT;
        case EDGE_UNIT:
            return CLOUD_UNIT;
        case CLOUD_UNIT:
            return EXIT;
            break;
    }
}

//Fornisce il codice del blocco di destinazione partendo dal blocco di controllo iniziale
//logica del dispatcher
int routing_from_control_unit() {
   intermittent_wlan();
   if(((*wlan_unit)->online==OFFLINE) && ((*wlan_unit+1)->online==OFFLINE)) { //i server della WLAN sono OFFLINE
           return ENODE_UNIT; 
        }      
   else {
       double random = Uniform(0, 1);
       if(random<=P_WLAN) {
          return WLAN_UNIT;
       } else {
          return ENODE_UNIT;
       }
   }
}

//Thread che disattiva la WLAN essendo un server intermittente 
int intermittent_wlan() {
    printf("INTERMITTENT WLAN\n");
    double random = Uniform(0,1);
    if(random<=P_OFF_WLAN) {
        printf("WLAN OFF\n");
        (*wlan_unit)->need_resched=true;
        (*wlan_unit+1)->need_resched=true;
    } else {
        printf("WLAN ON\n");
        (*wlan_unit)->need_resched=false;
        (*wlan_unit+1)->need_resched=false;
        (*wlan_unit)->online=ONLINE;
        (*wlan_unit+1)->online=ONLINE;
    }
}


// Inserisce un elemento nella lista ordinata
int insertSorted(sorted_completions *compls, compl completion) {
    int i;
    int n = compls->num_completions;

    for (i = n - 1; (i >= 0 && (compls->sorted_list[i].value > completion.value)); i--) {
        compls->sorted_list[i + 1] = compls->sorted_list[i];
    }
    compls->sorted_list[i + 1] = completion;
    compls->num_completions++;

    return (n + 1);
}

// Ricerca binaria di un elemento su una lista ordinata
int binarySearch(sorted_completions *compls, int low, int high, compl completion) {
    //printf("binary search\n");
    if (high < low) {
        return -1;
    }
    int mid = (low + high) / 2;
    if (completion.value == compls->sorted_list[mid].value) {
        return mid;
    }
    if (completion.value == compls->sorted_list[mid].value) {
        return binarySearch(compls, (mid + 1), high, completion);
    }
    return binarySearch(compls, low, (mid - 1), completion);
}

// Function to delete an element
int deleteElement(sorted_completions *compls, compl completion) {
    //printf("delete element\n");
    int i;
    int n = compls->num_completions;
   // printf("num element before delete : %d\n",n);

    int pos = binarySearch(compls, 0, n - 1, completion);

    if (pos == -1) {
        printf("Element not found\n");
        return n;
    }

    // Deleting element
    for (i = pos; i < n; i++) {
        compls->sorted_list[i] = compls->sorted_list[i + 1];
    }
    compls->sorted_list[n - 1].value = INFINITY;
    compls->num_completions--;
    //printf("num element after delete : %d\n",compls->num_completions);

    return n - 1;
}

// Ritorna il minimo tra due valori
double my_min(double x, double y) {
    return (x < y) ? x : y;
}

void print_sorted_list() {
    printf("PRINT DELLA SORTED LIST DI COMPLETAMENTI\n");
    for(int i=0;i<global_sorted_completions.num_completions;i++) {
        compl *nextCompletion = &global_sorted_completions.sorted_list[i];
        server *nextCompletionServer = nextCompletion->server;
        printf("[BLOCCO : %s, TIME : %f], ",stringFromEnum(nextCompletionServer->block->type),nextCompletion->value );
    }
    printf("\n");
}
// Esegue una singola run di simulazione ad orizzonte finito
void finite_horizon_run(int stop_time, int repetition) {
    printf("Method : Finite horizon run\n");
    printf("Stop time %d\n",stop_time);
    int n = 1;
    while (clock.arrival <= stop_time) {
        print_line();
        print_sorted_list();
        compl *nextCompletion = &global_sorted_completions.sorted_list[0];
        server *nextCompletionServer = nextCompletion->server;
    
        clock.next = (double) my_min(clock.arrival,nextCompletion->value);  // Ottengo il prossimo evento
        printf("OTTENUTO PROSSIMO EVENTO DALLA SORTED LIST -> %f\n", clock.next);
        if(clock.next==clock.arrival) {
            printf("EVENTO : ARRIVO ESTERNO DAL SISTEMA\n");
        } else {
            printf("EVENTO : COMPLETAMENTO DI UN JOB IN %s\n", stringFromEnum(nextCompletionServer->block->type));
        }
       
        printf("NUMERO JOB PRESENTI NEL SISTEMA  : \n");
        for (int i = 0; i < NUM_BLOCKS; i++) {
            if (blocks[i].jobInBlock > 0) {
                printf(" ->       JOB PRESENTI NEL BLOCCO %s : %d\n", stringFromEnum(blocks[i].type) , blocks[i].jobInBlock);
                blocks[i].area.node += (clock.next - clock.current) * blocks[i].jobInBlock;
                blocks[i].area.queue += (clock.next - clock.current) * blocks[i].jobInQueue;
            }
        }
        clock.current = clock.next;  // Avanzamento del clock al valore del prossimo evento
       // printf("clock current : %f", clock.current);
        if (clock.current == clock.arrival) {
            process_arrival();
        } else {
            process_completion(*nextCompletion);
        }
        if (clock.current >= (n - 1) * 30 && clock.current < (n)*30 && completed > 16 && clock.arrival < stop_time) {
            printf("calculate statistic interno \n ");
            calculate_statistics_clock(blocks, clock.current);
            n++;
        }
    }
    //calculate statistic finali
   calculate_statistics_fin(blocks, clock.current, statistics, repetition);
    //calcolo bilanciamento energetico 
  print_line();
   printf("fine\n");
}

// Esegue le ripetizioni di singole run a orizzonte finito
void finite_horizon_simulation(int stop_time, int repetitions) {
    printf("finite horizon simulation\n");
    printf("\n\n==== Finite Horizon Simulation | sim_time %f | #repetitions #%d ====", STOP, NUM_REPETITIONS);
    printf("\n\n");
    for (int r = 0; r < repetitions; r++) {
        printf("simulazione ciclo numero %d\n", r);
        finite_horizon_run(stop_time, r);
        clear_environment();
    }
    printf("write to csv\n");
    write_rt_csv_finite();
}

// Calcola le statistiche ogni 5 minuti per l'analisi nel continuo
void calculate_statistics_clock(block blocks[], double currentClock) {
        print_line();

    printf("calculate staticts clock\n");
    char filename[100];
    snprintf(filename, 100, "continuos_finite.csv");
    FILE *csv;
    csv = open_csv_appendMode(filename);

    double visit_rt = 0;
    double m = 0.0;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        int arr = blocks[i].total_arrivals;
        int jq = blocks[i].jobInQueue;
        double inter = currentClock / blocks[i].total_arrivals;
        int r_arr = arr - blocks[i].total_bypassed;

        double wait = blocks[i].area.node / arr;
        double delay = blocks[i].area.queue / r_arr;
        double service = blocks[i].area.service / r_arr;

        double external_arrival_rate = 1 / (currentClock / blocks[0].total_arrivals);
        double lambda_i = 1 / inter;
        double mu = 1 / service;
        double throughput = my_min(m * mu, lambda_i);
        if (i == CLOUD_UNIT) {
            throughput = lambda_i;
        }
        double visit = throughput / external_arrival_rate;
        visit_rt += wait * visit;
        print_statistics(&blocks[i], currentClock);
    }
    append_on_csv_v2(csv, visit_rt, currentClock);
    fclose(csv);
}

// Stampa a schermo le statistiche calcolate per ogni singolo blocco
void print_statistics(block blocks[], double currentClock) {
    double system_total_wait = 0;
    for (int i = 0; i < NUM_BLOCKS; i++) {

        int arr = blocks[i].total_arrivals;
        int r_arr = arr - blocks[i].total_bypassed;
        int jq = blocks[i].jobInQueue;
        double inter = currentClock / blocks[i].total_arrivals;

        double wait = blocks[i].area.node / arr;
        double delay = blocks[i].area.queue / r_arr;
        double service = blocks[i].area.service / r_arr;

        system_total_wait += wait;

        printf("\n\n======== Result for block %s ========\n", stringFromEnum(blocks[i].type));
        printf("Number of Servers ................... = %d\n",blocks[i].num_servers);
        printf("Arrivals ............................ = %d\n", arr);
        printf("Completions.......................... = %d\n", blocks[i].total_completions);
        printf("Job in Queue at the end ............. = %d\n", jq);
        printf("Average interarrivals................ = %6.6f\n", inter);

        printf("Average wait ........................ = %6.6f\n", wait);
        if (i == VIDEO_UNIT) {
            printf("Average wait (2)..................... = %6.6f\n", blocks[i].area.node / blocks[i].total_arrivals);
            printf("Number bypassed ..................... = %d\n", blocks[i].total_bypassed);
        }
        printf("Average delay ....................... = %6.6f\n", delay);
        printf("Average service time ................ = %6.6f\n", service);

        printf("Average # in the queue .............. = %6.6f\n", blocks[i].area.queue / currentClock);
        printf("Average # in the node ............... = %6.6f\n", blocks[i].area.node / currentClock);

        printf("\n    server     utilization     avg service\n");
        double p = 0;
        int n = 0;
        server **server_list;
        server_list=malloc(sizeof(server*));
        *(server_list) = malloc(sizeof(server)*blocks[i].num_servers);
        server_list = blocks[i].serv;
        for (int j = 0; j < blocks[i].num_servers; j++) {
            server *s = malloc(sizeof(server));
            s = *(server_list+j);
            if(s != NULL){
                //printf("%d\n", s->id);
                printf("   %d   %f    %f\n", s->id, (s->sum.service / currentClock), (s->sum.service / s->sum.served));
                p   += s->sum.service / currentClock;
                n++;
            }
            
        }
    }
}

// Calcola le statistiche specificate
void calculate_statistics_fin(block blocks[], double currentClock, double rt_arr[NUM_REPETITIONS], int rep) {
    printf("calculate_statistics_fin\n");
    double visit_rt = 0;
    double m = 0.0;
    for (int i = 0; i < NUM_BLOCKS; i++) {
      
        int arr = blocks[i].total_arrivals;
        int r_arr = arr - blocks[i].total_bypassed;
        int jq = blocks[i].jobInQueue;
        double inter = currentClock / blocks[i].total_arrivals;

        double wait = blocks[i].area.node / arr;
        double delay = blocks[i].area.queue / r_arr;
        double service = blocks[i].area.service / r_arr;

        double external_arrival_rate = 1 / (currentClock / blocks[CONTROL_UNIT].total_arrivals);
        double lambda_i = 1 / inter;
        double mu = 1 / service;
        double throughput = my_min(m * mu, lambda_i);
        if (i == CLOUD_UNIT) {
            throughput = lambda_i;
        }
        double visit = throughput / external_arrival_rate;
        visit_rt += wait * visit;

        double utilization = lambda_i / (m * mu);
        print_statistics(&blocks[i],currentClock);
    }
    rt_arr[rep] = visit_rt;
    printf("print statistiche finali\n");
}

// Resetta l'ambiente di esecuzione tra due run ad orizzonte finito
void clear_environment() {
    printf("clear environment\n");
    global_sorted_completions = empty_sorted;
    for (int block_type = 0; block_type < NUM_BLOCKS; block_type++) {
        blocks[block_type].area.node = 0;
        blocks[block_type].area.service = 0;
        blocks[block_type].area.queue = 0;
    }
}


// Resetta le statistiche tra un batch ed il successivo
void reset_statistics() {
    printf("reset_statistics\n");
    clock.batch_current = clock.current;
    for (int block_type = 0; block_type < NUM_BLOCKS; block_type++) {
        blocks[block_type].total_arrivals = 0;
        blocks[block_type].total_completions = 0;
        blocks[block_type].total_bypassed = 0;
        blocks[block_type].area.node = 0;
        blocks[block_type].area.service = 0;
        blocks[block_type].area.queue = 0;
    }
}



//Calcola l'energia consumata dal sistema (capire come aggiornare vari'abili per ogni esecuzione)
int calculate_energy_consumption() {

}


int initialize() {
   printf("initialize\n");
   streamID=0;
   clock.current = START;
   completed = 0;
   bypassed = 0;
   dropped = 0;
   global_sorted_completions.num_completions = 0;
   printf("clock current initialize %f\n", clock.current);
  
    for (int block_type = 0; block_type < NUM_BLOCKS; block_type++) {
        blocks[block_type].type = block_type;
        blocks[block_type].jobInBlock = 0;
        blocks[block_type].jobInQueue = 0;
        blocks[block_type].total_arrivals = 0;
        blocks[block_type].total_completions = 0;
        blocks[block_type].total_bypassed = 0;
        blocks[block_type].area.node = 0;
        blocks[block_type].area.service = 0;
        blocks[block_type].area.queue = 0;
    }
   printf("blocks initialized  \n");
   control_unit=calloc(1,sizeof(server*));
   (*control_unit)=calloc(1,sizeof(server));
   blocks[0].num_servers=1; //control unit
   
   video_unit=calloc(2,sizeof(server*));
   (*video_unit)=calloc(2,sizeof(server));
   blocks[1].num_servers=2; //video 

   wlan_unit=calloc(2,sizeof(server*));
   (*wlan_unit)=calloc(2,sizeof(server));
   blocks[2].num_servers=2; //wlan 
 
   enode_unit=calloc(1,sizeof(server*));
   (*enode_unit)=calloc(1,sizeof(server));
   blocks[3].num_servers=1; //enode 

   edge_unit=calloc(4,sizeof(server*));
   (*edge_unit)=calloc(4,sizeof(server));
   blocks[4].num_servers=4; //edge

   cloud_unit=calloc(1,sizeof(server*));
   (*cloud_unit)=calloc(1,sizeof(server));
   blocks[5].num_servers=1; //cloud

   printf("unit starting initialized \n");

   (*control_unit)->id=0;
   (*control_unit)->status=IDLE;
   (*control_unit)->online=ONLINE;
   (*control_unit)->loss=NOT_LOSS_SYSTEM;
   (*control_unit)->stream=streamID++;
   streamID+=streamID;
   (*control_unit)->block=malloc(sizeof(block));
   (*control_unit)->block=&blocks[0];
  
   printf("control_unit initialized\n");
   for(int i=0;i<blocks[1].num_servers;i++) {
       (*video_unit+i)->id=i;
       (*video_unit+i)->status=IDLE;
       (*video_unit+i)->online=ONLINE;
       (*video_unit+i)->loss=LOSS_SYSTEM;
       (*video_unit+i)->stream=streamID++;
       (*video_unit+i)->block=malloc(sizeof(block));
       (*video_unit+i)->block=&blocks[1];  
       streamID+=streamID;
   }
   blocks[1].type=1;


   for(int i=0;i<blocks[2].num_servers;i++) {
         (*wlan_unit+i)->id=i;
         (*wlan_unit+i)->status=IDLE;
         (*wlan_unit+i)->online=ONLINE;
         (*wlan_unit+i)->loss=NOT_LOSS_SYSTEM;
         (*wlan_unit+i)->stream=streamID++;
         (*wlan_unit+i)->block=malloc(sizeof(block));
         (*wlan_unit+i)->block=&blocks[2];
         streamID+=streamID;
   }

   (*enode_unit)->id=6;
   (*enode_unit)->status=IDLE;
   (*enode_unit)->online=ONLINE;
   (*enode_unit)->loss=NOT_LOSS_SYSTEM;
   (*enode_unit)->stream=streamID++;
   (*enode_unit)->block=malloc(sizeof(block));
   (*enode_unit)->block=&blocks[3];
   streamID+=streamID;
   
   for(int i=0;i<blocks[4].num_servers;i++) {
          (*edge_unit+i)->id=i;
          (*edge_unit+i)->status=IDLE;
          (*edge_unit+i)->online=ONLINE;
          (*edge_unit+i)->loss=NOT_LOSS_SYSTEM;
          (*edge_unit+i)->stream=streamID++;
          (*edge_unit+i)->block=malloc(sizeof(block));
          (*edge_unit+i)->block=&blocks[4];
          streamID+=streamID;
   }

   (*cloud_unit)->id=11;
   (*cloud_unit)->status=IDLE;
   (*cloud_unit)->online=ONLINE;
   (*cloud_unit)->loss=NOT_LOSS_SYSTEM;
   (*cloud_unit)->stream=streamID++;
   (*cloud_unit)->block=malloc(sizeof(block));
   (*cloud_unit)->block=&blocks[5];

   blocks[0].serv = malloc(sizeof(server*));
   blocks[0].serv=control_unit;
   blocks[1].serv = malloc(sizeof(server*)*2);
   blocks[1].serv=video_unit;
   blocks[2].serv = malloc(sizeof(server*)*2);
   blocks[2].serv=wlan_unit;
   blocks[3].serv = malloc(sizeof(server*));
   blocks[3].serv=enode_unit;
   blocks[4].serv = malloc(sizeof(server*)*4);
   blocks[4].serv =edge_unit;
   blocks[5].serv = malloc(sizeof(server*));
   blocks[5].serv= cloud_unit;
   clock.arrival = getArrival(clock.current);
   blocks[1].type=1;


   insertSorted(&global_sorted_completions, (compl) {(*control_unit), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*video_unit), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*video_unit+1), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*wlan_unit), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*wlan_unit+1), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*enode_unit), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*edge_unit), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*edge_unit+1), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*edge_unit+2), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*edge_unit+3), INFINITY});
   insertSorted(&global_sorted_completions, (compl) {(*cloud_unit), INFINITY});
   global_sorted_completions.num_completions=0;

   printf("%f\n",clock.arrival);
   for(int i=0;i<6;i++) {
   printf("%d type\n", blocks[i].type);
   }
   printf("finish initialized\n");
}


int main(void) {
    PlantSeeds(231232132);
    initialize() ;
    finite_horizon_simulation(STOP, NUM_REPETITIONS);
}


// Scrive i tempi di risposta a tempo finito su un file csv
void write_rt_csv_finite() {
    FILE *csv;
    char* filename = "results.csv";
    csv = open_csv(filename);
    if(csv != NULL){
        printf("file exist!\n");
        for (int i = 0; i < NUM_REPETITIONS; i++) {
            append_on_csv(csv, i, statistics[i]);
        }
        fclose(csv);
    }
    
 }

