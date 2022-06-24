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
    

    double arrival = current;
    SelectStream(254);
    arrival += Exponential(1 / ARRIVAL_RATE);
    printf("Arrival value: %f\n", arrival);
    return arrival;
}

// Genera un tempo di servizio esponenziale di media specificata e stream del servente individuato
double getService(int type_service, int stream) {
    SelectStream(stream);
    

    switch (type_service) {
        case 0:
            double e = Exponential(CONTROL_UNIT_SERVICE_TIME);
            printf("Service time case control unit: %f\n", e);
            return e;
        case 1:
            double e = Exponential(VIDEO_SERVICE_TIME);
            printf("Service time case video unit: %f\n", e);
            return e;
        case 2:
            double e = Exponential(WLAN_FRAME_UPLOAD_TIME);
            printf("Service time case wlan unit: %f\n", e);
            return e;
        case 3:
            double e = Exponential(ENODE_FRAME_UPLOAD_TIME);
            printf("Service time case enode unit: %f\n", e);
            return e;
        case 4:
            double e = Exponential(EDGE_PROCESSING_TIME);
            printf("Service time case edge unit: %f\n", e);
            return e;
        case 5:
            double e = Exponential(CLOUD_PROCESSING_TIME);
            printf("Service time case cloud unit: %f\n", e);
            return e; 
        default:
            printf("Case not defined\n");
            return 0;
    }
}

// Inserisce un job nella coda del blocco specificata
void enqueue(block *block, double arrival, int type) {
    printf("Enqueue\n");
    printf("Type: %d\n",type); //External 6
    job *j = (job *)malloc(sizeof(job));
    if (j == NULL)
        handle_error("malloc");

    j->arrival = arrival;
    j->next = NULL;
    j->type = type;

    if (block->tail){  // Appendi alla coda se esiste, altrimenti è la testa
        block->tail->next = j;
        printf("Append to tail\n");
    }else{
        block->head_service = j;
         printf("Append to head\n");
    }

    block->tail = j;

    if (block->head_queue == NULL) {
         printf("head queue null\n");
        block->head_queue = j;
    }
}


// Rimuove il job dalla coda del blocco specificata, ritorna tipo di job
int dequeue(block *block_t) {
    printf("Dequeue\n");
    job *j = malloc(sizeof(job));
    j = (block_t->head_service);
    printf("Arrival %f\n",j->arrival);
    int type = 0; 
    type = j->type;
    printf("Type: %d\n",j->type);
    if (!j->next)
        block_t->tail = NULL;
    block_t->head_service = j->next;

    if (block_t->head_queue != NULL && block_t->head_queue->next != NULL) {
        printf("Take next job of the queue\n");
        job *tmp = block_t->head_queue->next;
        block_t->head_queue = tmp;
    } else {
        printf("No job in the queue\n");
        block_t->head_queue = NULL;
    }
    free(j);
    return type;
}

// Ritorna il primo server libero nel blocco specificato
server *findFreeServer(block *b) {
    printf("Find free server called\n\n");
    int num = b->num_servers;
    server *serv = &(b->serv);
    for (int i = 0; i < b->num_servers; i++) {
        if((serv+i)->status==IDLE){
            printf("Found free server in position i: %d\n\n", i);
            return (serv+i);
        }
    }
    return NULL;
}

// Processa un arrivo dall'esterno verso il sistema
void process_arrival() {
    printf("Process arrival\n");
    blocks[0].total_arrivals++;
    blocks[0].jobInBlock++;

    server *s = findFreeServer(&blocks[0]);
    // C'è un servente libero, quindi genero il completamento
    if (s != NULL) {
        double serviceTime = getService(CONTROL_UNIT, s->stream);
        printf("Service time: %f\n", serviceTime);
        compl c = {s, INFINITY};
        s->block=&blocks[0];
        c.server=s;
        c.value = clock.current + serviceTime;
        printf("valore completamento: %d\n", c.value);
        s->status = BUSY;  // Setto stato busy
        s->sum.service += serviceTime;
        printf("Service time sum: %f", s->sum.service);
        //////////////////////////////////////////////
        //printf("num server %f\n",(s->block)->num_servers);
        s->block->area.service += serviceTime;
        printf("Service time area: %f", s->block->area.service);
        s->sum.served++;
        printf("served jobs: %ld\n", s->sum.served);
        insertSorted(&global_sorted_completions, c);
        enqueue(&blocks[0], clock.arrival,EXTERNAL);  // lo appendo nella linked list di job del blocco 
    } else {
        printf("Server free not found\n");
        enqueue(&blocks[0], clock.arrival,EXTERNAL);  // lo appendo nella linked list di job del blocco 
        blocks[0].jobInQueue++;              // Se non c'è un servente libero aumenta il numero di job in coda
    }
    clock.arrival = getArrival(clock.current);  // Genera prossimo arrivo
}

// Processa un next-event di completamento
void process_completion(compl c) {
    printf("Process completion \n");
    block *block_ = (c.server)->block;
    int block_type = (block_)->type;
    blocks[block_type].total_completions++;
    blocks[block_type].jobInBlock--;
    int type;
    int destination;
    server *freeServer;

    type = dequeue(&blocks[block_type]);  // Toglie il job servito dal blocco e fa "avanzare" la lista collegata di job
    deleteElement(&global_sorted_completions, c);
    // Se nel blocco ci sono job in coda, devo generare il prossimo completamento per il servente che si è liberato.
    if (blocks[block_type].jobInQueue > 0 && !c.server->need_resched) {
        printf("Job presenti in coda: %d\n",blocks[block_type].jobInQueue);
        blocks[block_type].jobInQueue--;
        double service_1 = getService(block_type, c.server->stream);
        printf("Service time: %f", service_1);
        c.value = clock.current + service_1;
        c.server->sum.service += service_1;
        c.server->sum.served++;
        c.server->block->area.service += service_1;
        insertSorted(&global_sorted_completions, c);
    } else {
        printf("Job non presenti in coda\n");
        c.server->status = IDLE;
    }

    // Se un server è schedulato per la terminazione, non prende un job dalla coda e và OFFLINE
    if (c.server->need_resched) {
        printf("server schedulato per la terminazione\n"); 
        c.server->online = OFFLINE;
        c.server->need_resched = false;
    }

    //uscita dalla rete se il job esce dal CLOUD
    if (block_type == CLOUD_UNIT) {
        printf("Job esce dalla rete\n");
        completed++;
        return;
    }

    // Gestione blocco destinazione job interno
    destination = getDestination(c.server->block->type,type);  // Trova la destinazione adatta per il job appena servito 
    if (destination == EXIT) {
        printf("job destinato all'uscita\n");
        blocks[block_type].total_dropped++;
        dropped++;
        return;
    }
    if (destination == CLOUD_UNIT) { //M/M/INF non accoda mai, come se i server fossero sempre liberi
            printf("job destinato alla cloud unit\n");
            server * cloud_server = *(&blocks[CLOUD_UNIT].serv);
            blocks[destination].jobInBlock++;
            enqueue(&blocks[destination], c.value,INTERNAL);
            compl c2 = {cloud_server, INFINITY};
            double service_2 = getService(CLOUD_UNIT, cloud_server->stream);
            printf("Service time: %f\n", service_2);
            c2.value = clock.current + service_2;
            cloud_server->sum.service += service_2;
            cloud_server->sum.served++;
            cloud_server->block->area.service += service_2;
            completed++;
            return;
    }
    if (destination != CLOUD_UNIT && destination != VIDEO_UNIT) {
        printf("Destination: %d", destination);
        blocks[destination].total_arrivals++;
        blocks[destination].jobInBlock++;
        enqueue(&blocks[destination], c.value, INTERNAL);  // Posiziono il job nella coda del blocco destinazione e gli imposto come tempo di arrivo quello di completamento

        // Se il blocco destinatario ha un servente libero, generiamo un tempo di completamento, altrimenti aumentiamo il numero di job in coda
        freeServer = findFreeServer(&blocks[destination]);
        if (freeServer != NULL) {
            printf("Server libero trovato\n");
            compl c2 = {freeServer, INFINITY};
             enqueue(&blocks[destination], c.value,INTERNAL);
            double service_2 = getService(destination, freeServer->stream);
            printf("Service time: %f\n", service_2);
            c2.value = clock.current + service_2;
            insertSorted(&global_sorted_completions, c2);
            freeServer->status = BUSY;
            freeServer->sum.service += service_2;
            freeServer->sum.served++;
            freeServer->block->area.service += service_2;
            return;
        } else {
            printf("Server libero non disponibile\n");
            blocks[destination].jobInQueue++; 
            return;
        }
    }
    //video unit - a perdita 
    if (destination==VIDEO_UNIT)  {
        printf("Destinazione: Video Unit\n");
         blocks[destination].total_arrivals++;
         freeServer = findFreeServer(&blocks[destination]);
         if (freeServer != NULL) {
          printf("Server libero trovato per video unit \n");
          blocks[destination].jobInBlock++;
          enqueue(&blocks[destination], c.value,INTERNAL);
          compl c3 = {freeServer, INFINITY};
          double service_3 = getService(destination, freeServer->stream);
          printf("Service time: %f\n", service_3);
          c3.value = clock.current + service_3;
          insertSorted(&global_sorted_completions, c3);
          freeServer->status = BUSY;
          freeServer->sum.service += service_3;
          freeServer->sum.served++;
          freeServer->block->area.service += service_3;
          return;

    } else { //LOSS
        printf("Telecamera libera non disponibile\n");
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
                printf("Routing to Video Unit\n");
                return VIDEO_UNIT;
            }
            else { //internal 
                printf("Internal Routing\n"); 
                return routing_from_control_unit();
            }
        case VIDEO_UNIT:
            printf("Routing to Control Unit\n");
            return CONTROL_UNIT;
        case WLAN_UNIT:
            printf("Routing to Edge Unit\n");
            return EDGE_UNIT;
        case ENODE_UNIT:
            printf("Routing to Edge Unit\n");
            return EDGE_UNIT;
        case EDGE_UNIT:
            printf("Routing to Cloud Unit\n");
            return CLOUD_UNIT;
        case CLOUD_UNIT:
            printf("Routing to Exit\n");
            return EXIT;
            break;
    }
}

//Fornisce il codice del blocco di destinazione partendo dal blocco di controllo iniziale
//logica del dispatcher
int routing_from_control_unit() {

   if(((*wlan_unit)->online==OFFLINE) && ((*wlan_unit+1)->online==OFFLINE)) { //i server della WLAN sono OFFLINE
           printf("Routing to Enode from Control Unit\n");
           return ENODE_UNIT; 
        }      
   else {
       double random = Uniform(0, 1);
       if(random<=P_WLAN) {
          printf("Routing to Wlan from Control Unit\n"); 
          return WLAN_UNIT;
       } else {
          printf("Routing to Enode from Control Unit because Wlan is Off\n"); 
          return ENODE_UNIT;
       }
   }
}

//Thread che disattiva la WLAN essendo un server intermittente 
int intermittent_wlan() {
    printf("intermittent wlan\n");
    double random = Uniform(0,1);
    if(random<=P_OFF_WLAN) {
        printf("wlan off\n");
        (*wlan_unit)->need_resched=true;
        (*wlan_unit+1)->need_resched=true;
    } else {
        printf("wlan on\n");
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
    printf("binary done");
    return binarySearch(compls, low, (mid - 1), completion);
}

// Function to delete an element
int deleteElement(sorted_completions *compls, compl completion) {
    printf("delete element\n");
    int i;
    int n = compls->num_completions;
    printf("num element before delete : %d\n",n);

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
    printf("num element after delete : %d\n",compls->num_completions);

    return n - 1;
}

// Ritorna il minimo tra due valori
double my_min(double x, double y) {
    return (x < y) ? x : y;
}
// Esegue una singola run di simulazione ad orizzonte finito
void finite_horizon_run(int stop_time, int repetition) {
    printf("Method : Finite horizon run\n");
    printf("Stop time %d\n",stop_time);
    int n = 1;
    while (clock.arrival <= stop_time) {
        compl *nextCompletion = &global_sorted_completions.sorted_list[0];
        server *nextCompletionServer = nextCompletion->server;
     
        clock.next = (double) my_min(clock.arrival,nextCompletion->value);  // Ottengo il prossimo evento
        printf("NextCompletion value : %f\n", nextCompletion->value);
        printf("Arrival %f\n",clock.arrival);
        for (int i = 0; i < NUM_BLOCKS; i++) {
            if (blocks[i].jobInBlock > 0) {
                printf("num job in block:%d\n",blocks[i].jobInBlock);
                blocks[i].area.node += (clock.next - clock.current) * blocks[i].jobInBlock;
                blocks[i].area.queue += (clock.next - clock.current) * blocks[i].jobInQueue;
            }
        }
        clock.current = clock.next;  // Avanzamento del clock al valore del prossimo evento
        printf("clock current : %f", clock.current);
        if (clock.current == clock.arrival) {
            printf("process arrival finite horizon - clock.current %f = clock.arrival %f \n", clock.current, clock.arrival);
            process_arrival();
        } else {
            printf("process completion finite horizon \n");
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
   printf("fine\n");
}

// Esegue le ripetizioni di singole run a orizzonte finito
void finite_horizon_simulation(int stop_time, int repetitions) {
    printf("finite horizon simulation\n");
    printf("\n\n==== Finite Horizon Simulation | sim_time %d | #repetitions #%d ====", STOP, NUM_REPETITIONS);
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
    printf("calculate staticts clock\n");
    printf("---------------------------------------------------------------\n");

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
    char type[20];
    double system_total_wait = 0;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        strcpy(type, stringFromEnum(blocks[i].type));

        int arr = blocks[i].total_arrivals;
        int r_arr = arr - blocks[i].total_bypassed;
        int jq = blocks[i].jobInQueue;
        double inter = currentClock / blocks[i].total_arrivals;

        double wait = blocks[i].area.node / arr;
        double delay = blocks[i].area.queue / r_arr;
        double service = blocks[i].area.service / r_arr;

        system_total_wait += wait;

        printf("\n\n======== Result for block %s ========\n", type);
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
        for (int j = 0; j < blocks[i].num_servers; j++) {
            server *s = *(blocks[i].serv+j);
            printf("%8d %15.5f %15.2f\n", s->id, (s->sum.service / currentClock), (s->sum.service / s->sum.served));
            p += s->sum.service / currentClock;
            n++;
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
   (*control_unit)->status=IDLE;
   (*control_unit)->online=ONLINE;
   (*control_unit)->loss=NOT_LOSS_SYSTEM;
   (*control_unit)->stream=streamID++;
   streamID=streamID++;
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
       streamID=streamID++;
   }

   for(int i=0;i<blocks[2].num_servers;i++) {
         (*wlan_unit+i)->id=i;
         (*wlan_unit+i)->status=IDLE;
         (*wlan_unit+i)->online=ONLINE;
         (*wlan_unit+i)->loss=NOT_LOSS_SYSTEM;
         (*wlan_unit+i)->stream=streamID++;
         (*wlan_unit+i)->block=malloc(sizeof(block));
         (*wlan_unit+i)->block=&blocks[2];
         streamID=streamID++;
   }

   (*enode_unit)->id=6;
   (*enode_unit)->status=IDLE;
   (*enode_unit)->online=ONLINE;
   (*enode_unit)->loss=NOT_LOSS_SYSTEM;
   (*enode_unit)->stream=streamID++;
   (*enode_unit)->block=malloc(sizeof(block));
   (*enode_unit)->block=&blocks[3];
   streamID=streamID++;
   
   for(int i=0;i<blocks[4].num_servers;i++) {
          (*edge_unit+i)->id=i;
          (*edge_unit+i)->status=IDLE;
          (*edge_unit+i)->online=ONLINE;
          (*edge_unit+i)->loss=NOT_LOSS_SYSTEM;
          (*edge_unit+i)->stream=streamID++;
          (*edge_unit+i)->block=malloc(sizeof(block));
          (*edge_unit+i)->block=&blocks[4];
          streamID=streamID++;
   }

   (*cloud_unit)->id=11;
   (*cloud_unit)->status=IDLE;
   (*cloud_unit)->online=ONLINE;
   (*cloud_unit)->loss=NOT_LOSS_SYSTEM;
   (*cloud_unit)->stream=streamID++;
   (*cloud_unit)->block=malloc(sizeof(block));
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

