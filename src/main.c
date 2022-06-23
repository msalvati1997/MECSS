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
- decidere i tipi di job e implementare logica se diversi  (fatto)
- implemetnare insertSorted  (simo)
- implementare funzione delete element dalla lista sortata (simo)
- implementare get destination  (ritorna blocco)  (marti)
- funzione di routing from control unit to cloud   (marti)
- funzione di intermittenza WLAN (thread separato) -> {need_resched,offline/online}
/////////////////////////////////////////////////////////////////////
- funzione resetta variabili 
- aggiungere statistiche da fare 
- funzione che calcola tempo totale teorico
- funzione che calcola dispendio energetico teorico 
- funzione che calcola tempo totale  
- aggiungere funzione calcolo dispendio energetico 
*/
// Genera un tempo di arrivo secondo la distribuzione Esponenziale
struct clock_t clock;                          // Mantiene le informazioni sul clock di simulazione

FILE *open_csv(char *filename) {
    FILE *fpt;
    fpt = fopen(filename, "w+");
    return fpt;
}

double getArrival(double current) {
    printf("get arrival\n");

    double arrival = current;
    SelectStream(254);
    arrival += Exponential(1 / ARRIVAL_RATE);
    printf("print arrival %f\n",arrival);
    return arrival;
}

// Genera un tempo di servizio esponenziale di media specificata e stream del servente individuato
double getService(int type_service, int stream) {
    SelectStream(stream);
    printf("get service\n");

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
            return Exponential(CLOUD_PROCESSING_TIME); 
        default:
            return 0;
    }
}

// Inserisce un job nella coda del blocco specificata
void enqueue(block *block, double arrival, int type) {
    printf("enqueue\n");
    printf("type %d\n",type);
    job *j = (job *)malloc(sizeof(job));
    if (j == NULL)
        handle_error("malloc");

    j->arrival = arrival;
    j->next = NULL;
    j->type = type;

    if (block->tail)  // Appendi alla coda se esiste, altrimenti è la testa
        block->tail->next = j;
    else
        block->head_service = j;

    block->tail = j;

    if (block->head_queue == NULL) {
        block->head_queue = j;
    }
}


// Rimuove il job dalla coda del blocco specificata, ritorna tipo di job
int dequeue(block *block_t) {
    printf("dequeue\n");
    job *j = malloc(sizeof(job));
    j = &(block_t->head_service);
    printf("arrival %f\n",j->arrival);
    int type; 
    type=0;
    type = j->type;
    printf("type: %d\n",j->type);
    if (!j->next)
        block_t->tail = NULL;
    block_t->head_service = j->next;

    if (block_t->head_queue != NULL && block_t->head_queue->next != NULL) {
        job *tmp = block_t->head_queue->next;
        block_t->head_queue = tmp;
    } else {
        block_t->head_queue = NULL;
    }
    free(j);
    return type;
}

// Ritorna il primo server libero nel blocco specificato
server *findFreeServer(block *b) {
    printf("find free server\n\n");
    for (int i = 0; i < b->num_servers; i++) {
        if(((*(b->serv)+i)->status)==IDLE){
            return *(b->serv)+i;
        }
    }
    return NULL;
}

// Processa un arrivo dall'esterno verso il sistema
void process_arrival() {
    printf("process arrival\n");
    printf("\n\n\n\n\n\n");
    blocks[0].total_arrivals++;
    blocks[0].jobInBlock++;

    server *s = findFreeServer(&blocks[0]);
    // C'è un servente libero, quindi genero il completamento
    if (s != NULL) {
        printf("free server\n");
        double serviceTime = getService(CONTROL_UNIT, s->stream);
        compl c = {s, INFINITY};
        c.value = clock.current + serviceTime;
        s->status = BUSY;  // Setto stato busy
        s->sum.service += serviceTime;
        s->block->area.service += serviceTime;
        s->sum.served++;
        insertSorted(&global_sorted_completions, c);
        enqueue(&blocks[0], clock.arrival,EXTERNAL);  // lo appendo nella linked list di job del blocco 
    } else {
        printf("not free\n");
        enqueue(&blocks[0], clock.arrival,EXTERNAL);  // lo appendo nella linked list di job del blocco 
        blocks[0].jobInQueue++;              // Se non c'è un servente libero aumenta il numero di job in coda
    }
    clock.arrival = getArrival(clock.current);  // Genera prossimo arrivo
}

// Processa un next-event di completamento
void process_completion(compl c) {
    printf("process completion \n");
    block *block_ = (c.server)->block;
    int block_type = (block_)->type;
    blocks[block_type].total_completions++;
    blocks[block_type].jobInBlock--;
    int type;
    int destination;
    server *freeServer;

    type = dequeue(&blocks[block_type]);  // Toglie il job servito dal blocco e fa "avanzare" la lista collegata di job
    printf("before delete\n");
    deleteElement(&global_sorted_completions, c);
    printf("after delete\n");
    // Se nel blocco ci sono job in coda, devo generare il prossimo completamento per il servente che si è liberato.
    if (blocks[block_type].jobInQueue > 0 && !c.server->need_resched) {
        blocks[block_type].jobInQueue--;
        double service_1 = getService(block_type, c.server->stream);
        c.value = clock.current + service_1;
        c.server->sum.service += service_1;
        c.server->sum.served++;
        c.server->block->area.service += service_1;
        insertSorted(&global_sorted_completions, c);
    } else {
        c.server->status = IDLE;
    }

    // Se un server è schedulato per la terminazione, non prende un job dalla coda e và OFFLINE
    if (c.server->need_resched) { 
        c.server->online = OFFLINE;
        c.server->need_resched = false;
    }

    //uscita dalla rete se il job esce dal CLOUD
    if (block_type == CLOUD_UNIT) {
        completed++;
        return;
    }

    // Gestione blocco destinazione job interno
    destination = getDestination(c.server->block->type,type);  // Trova la destinazione adatta per il job appena servito 
    if (destination == EXIT) {
        blocks[block_type].total_dropped++;
        dropped++;
        return;
    }
    if (destination == CLOUD_UNIT) { //M/M/INF non accoda mai, come se i server fossero sempre liberi
            server * cloud_server = *(&blocks[CLOUD_UNIT].serv);
            blocks[destination].jobInBlock++;
            enqueue(&blocks[destination], c.value,INTERNAL);
            compl c2 = {cloud_server, INFINITY};
            double service_2 = getService(CLOUD_UNIT, cloud_server->stream);
            c2.value = clock.current + service_2;
            cloud_server->sum.service += service_2;
            cloud_server->sum.served++;
            cloud_server->block->area.service += service_2;
            completed++;
            return;
    }
    if (destination != CLOUD_UNIT & destination != VIDEO_UNIT) {
        blocks[destination].total_arrivals++;
        blocks[destination].jobInBlock++;
        enqueue(&blocks[destination], c.value, INTERNAL);  // Posiziono il job nella coda del blocco destinazione e gli imposto come tempo di arrivo quello di completamento

        // Se il blocco destinatario ha un servente libero, generiamo un tempo di completamento, altrimenti aumentiamo il numero di job in coda
        freeServer = findFreeServer(&blocks[destination]);
        if (freeServer != NULL) {
            compl c2 = {freeServer, INFINITY};
             enqueue(&blocks[destination], c.value,INTERNAL);
            double service_2 = getService(destination, freeServer->stream);
            c2.value = clock.current + service_2;
            insertSorted(&global_sorted_completions, c2);
            freeServer->status = BUSY;
            freeServer->sum.service += service_2;
            freeServer->sum.served++;
            freeServer->block->area.service += service_2;
            return;
        } else {
            blocks[destination].jobInQueue++; 
            return;
        }
    }
    //video unit - a perdita 
    if (destination==VIDEO_UNIT)  {
         blocks[destination].total_arrivals++;
         freeServer = findFreeServer(&blocks[destination]);
         if (freeServer != NULL) {
          blocks[destination].jobInBlock++;
          enqueue(&blocks[destination], c.value,INTERNAL);
          compl c3 = {freeServer, INFINITY};
          double service_3 = getService(destination, freeServer->stream);
          c3.value = clock.current + service_3;
          insertSorted(&global_sorted_completions, c3);
          freeServer->status = BUSY;
          freeServer->sum.service += service_3;
          freeServer->sum.served++;
          freeServer->block->area.service += service_3;
          return;

    } else { //LOSS
        completed++;
        bypassed++;
        blocks[destination].total_bypassed++;
        return;
    }
    }
}

// Ritorna il blocco destinazione di un job dopo il suo completamento
int getDestination(enum block_types from, int type) {
    printf("getdestination\n");
    switch (from) {
        case CONTROL_UNIT:
            if(type==EXTERNAL) {  //external  
                return VIDEO_UNIT;
            }
            else { //internal  
                return routing_from_control_unit();
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
    printf("routing control unit\n");

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
    printf("intermittent wlan\n");
    double random = Uniform(0,1);
    if(random<=P_OFF_WLAN) {
        (*wlan_unit)->need_resched=true;
        (*wlan_unit+1)->need_resched=true;
    } else {
        (*wlan_unit)->need_resched=false;
        (*wlan_unit+1)->need_resched=false;
        (*wlan_unit)->online=ONLINE;
        (*wlan_unit+1)->online=ONLINE;
    }
}


// Inserisce un elemento nella lista ordinata
int insertSorted(sorted_completions *compls, compl completion) {
    printf("insert sorted\n");
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
    printf("binary search\n");
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
    printf("delete element\n");
    int i;
    int n = compls->num_completions;

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

    return n - 1;
}

// Ritorna il minimo tra due valori
double my_min(double x, double y) {
    return (x < y) ? x : y;
}
// Esegue una singola run di simulazione ad orizzonte finito
void finite_horizon_run(int stop_time, int repetition) {
    printf("finite horizon run\n");
    int n = 1;
    while (clock.arrival <= stop_time) {
        compl *nextCompletion = &global_sorted_completions.sorted_list[0];
        server *nextCompletionServer = nextCompletion->server;
        printf("nextCompetion value %f\n",nextCompletion->value);
        printf("clock arrival value %f\n",clock.arrival);
        clock.next = (double) my_min(clock.arrival,nextCompletion->value);  // Ottengo il prossimo evento
        printf("clock next %f\n", clock.next);
        printf("get next event \n");
        for (int i = 0; i < NUM_BLOCKS; i++) {
            if (blocks[i].jobInBlock > 0) {
                blocks[i].area.node += (clock.next - clock.current) * blocks[i].jobInBlock;
                blocks[i].area.queue += (clock.next - clock.current) * blocks[i].jobInQueue;
            }
        }
        clock.current = clock.next;  // Avanzamento del clock al valore del prossimo evento
        printf("clock current %f\n", clock.current);
        printf("clock arrival %f\n", clock.arrival);

        if (clock.current == clock.arrival) {
            printf("process arrival finite horizon\n");
            process_arrival();
        } else {
            printf("process completion finite horizon \n");
            process_completion(*nextCompletion);
        }
        if (clock.current >= (n - 1) * 300 && clock.current < (n)*300 && completed > 16 && clock.arrival < stop_time) {
            calculate_statistics_clock(blocks, clock.current);
            n++;
        }
    }
    //calculate statistic finali
   calculate_statistics_fin(blocks, clock.current, statistics, repetition);
    //calcolo bilanciamento energetico 
}

// Esegue le ripetizioni di singole run a orizzonte finito
void finite_horizon_simulation(int stop_time, int repetitions) {
    printf("finite horizon simulation\n");
    printf("\n\n==== Finite Horizon Simulation | sim_time %d | #repetitions #%d ====", STOP, NUM_REPETITIONS);
    printf("\n\n");
    for (int r = 0; r < repetitions; r++) {
        finite_horizon_run(stop_time, r);
        clear_environment();
    }
    printf("write to csv\n");
    write_rt_csv_finite();
}

// Calcola le statistiche ogni 5 minuti per l'analisi nel continuo
void calculate_statistics_clock(block blocks[], double currentClock) {
    printf("calculate staticts clock\n");
    char filename[100];
    snprintf(filename, 100, "results/finite/continuos_finite.csv");
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
    }
    append_on_csv_v2(csv, visit_rt, currentClock);
    fclose(csv);
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
    }
    rt_arr[rep] = visit_rt;
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



//Calcola l'energia consumata dal sistema (capire come aggiornare variabili per ogni esecuzione)
int calculate_energy_consumption() {

}


int initialize() {
   printf("initialize\n");
   streamID=0;
   clock.current = START;
   printf("clock current initialize %f\n", clock.current);
   completed = 0;
   bypassed=0;

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
   insertSorted(&global_sorted_completions, (compl) {(*control_unit), INFINITY});
   blocks[0].num_servers=1; //control unit

   video_unit=calloc(2,sizeof(server*));
   (*video_unit)=calloc(1,sizeof(server));
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
   printf("1\n");
   (*control_unit)->status=IDLE;
   (*control_unit)->online=ONLINE;
   (*control_unit)->loss=NOT_LOSS_SYSTEM;
   (*control_unit)->stream=streamID++;
   streamID=streamID++;
   (*control_unit)->block=&blocks[0];
  
   printf("control_unit initialized\n");
   for(int i=0;i<blocks[1].num_servers;i++) {
       (*video_unit+i)->id=i;
       (*video_unit+i)->status=IDLE;
       (*video_unit+i)->online=ONLINE;
       (*video_unit+i)->loss=LOSS_SYSTEM;
       (*video_unit+i)->stream=streamID++;
       (*video_unit+i)->block=&blocks[1];  
       insertSorted(&global_sorted_completions, (compl){(*video_unit+i), INFINITY});
       streamID=streamID++;
   }

   for(int i=0;i<blocks[2].num_servers;i++) {
         (*wlan_unit+i)->id=i;
         (*wlan_unit+i)->status=IDLE;
         (*wlan_unit+i)->online=ONLINE;
         (*wlan_unit+i)->loss=NOT_LOSS_SYSTEM;
         (*wlan_unit+i)->stream=streamID++;
         (*wlan_unit+i)->block=&blocks[2];
         insertSorted(&global_sorted_completions, (compl){(*wlan_unit+i), INFINITY});
         streamID=streamID++;
   }

   (*enode_unit)->id=6;
   (*enode_unit)->status=IDLE;
   (*enode_unit)->online=ONLINE;
   (*enode_unit)->loss=NOT_LOSS_SYSTEM;
   (*enode_unit)->stream=streamID++;
   (*enode_unit)->block=&blocks[3];
    insertSorted(&global_sorted_completions, (compl){(*enode_unit), INFINITY});
   streamID=streamID++;
   
   for(int i=0;i<blocks[4].num_servers;i++) {
          (*edge_unit+i)->id=i;
          (*edge_unit+i)->status=IDLE;
          (*edge_unit+i)->online=ONLINE;
          (*edge_unit+i)->loss=NOT_LOSS_SYSTEM;
          (*edge_unit+i)->stream=streamID++;
          (*edge_unit+i)->block=&blocks[4];
          insertSorted(&global_sorted_completions, (compl){(*edge_unit), INFINITY});
          streamID=streamID++;
   }

   (*cloud_unit)->id=11;
   (*cloud_unit)->status=IDLE;
   (*cloud_unit)->online=ONLINE;
   (*cloud_unit)->loss=NOT_LOSS_SYSTEM;
   (*cloud_unit)->stream=streamID++;
   insertSorted(&global_sorted_completions, (compl){(*cloud_unit), INFINITY});
   (*cloud_unit)->block=&blocks[5];

   blocks[0].serv = calloc(1,sizeof(control_unit));
   memcpy(blocks[0].serv, &control_unit, sizeof(control_unit));
   blocks[1].serv = calloc(1,sizeof(video_unit));
   memcpy(blocks[1].serv, &video_unit, sizeof(video_unit));
   blocks[2].serv = calloc(1,sizeof(wlan_unit));
   memcpy(blocks[2].serv, &wlan_unit, sizeof(wlan_unit));
   blocks[3].serv = calloc(1,sizeof(enode_unit));
   memcpy(blocks[3].serv, &enode_unit, sizeof(enode_unit));
   blocks[4].serv = calloc(1,sizeof(edge_unit));
   memcpy(blocks[4].serv, &edge_unit, sizeof(edge_unit));
   blocks[5].serv = calloc(1,sizeof(cloud_unit));
   memcpy(blocks[5].serv, &cloud_unit, sizeof(cloud_unit));

   clock.arrival = getArrival(clock.current);
   printf("%f\n",clock.arrival);
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

