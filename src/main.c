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
- decidere i tipi di job e implementare logica se diversi 
- implemetnare insertSorted  
- implementare funzione delete element dalla lista sortata 
- implementare get destination  (ritorna blocco) 
- funzione di routing from control unit to cloud  
- funzione di intermittenza WLAN (thread separato)
- funzione resetta variabili 
- aggiungere statistiche da fare 
- funzione che calcola tempo totale teorico
- funzione che calcola dispendio energetico teorico 
- funzione che calcola tempo totale teorico 
- aggiungere funzione calcolo dispendio energetico 
*/
// Genera un tempo di arrivo secondo la distribuzione Esponenziale

double getArrival(double current) {

    double arrival = current;
    SelectStream(254);
    arrival += Exponential(1 / ARRIVAL_RATE);
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
            return Hyperexponential(WLAN_FRAME_UPLOAD_TIME,WLAN_P);
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
void enqueue(block *block, double arrival) {
    job *j = (job *)malloc(sizeof(job));
    if (j == NULL)
        handle_error("malloc");

    j->arrival = arrival;
    j->next = NULL;

    if (block->tail)  // Appendi alla coda se esiste, altrimenti è la testa
        block->tail->next = j;
    else
        block->head_service = j;

    block->tail = j;

    if (block->head_queue == NULL) {
        block->head_queue = j;
    }
}


// Rimuove il job dalla coda del blocco specificata
void dequeue(block *block) {
    job *j = block->head_service;

    if (!j->next)
        block->tail = NULL;

    block->head_service = j->next;

    if (block->head_queue != NULL && block->head_queue->next != NULL) {
        job *tmp = block->head_queue->next;
        block->head_queue = tmp;
    } else {
        block->head_queue = NULL;
    }
    free(j);
}

// Ritorna il primo server libero nel blocco specificato
server *findFreeServer(block *b) {
    for (int i = 0; i < b->num_servers; i++) {
        if(((*(b->serv)+i)->status)==IDLE){
            return *(b->serv)+i;
        }
    }
    return NULL;
}

// Processa un arrivo dall'esterno verso il sistema
void process_arrival() {
    blocks[0].total_arrivals++;
    blocks[0].jobInBlock++;

    server *s = findFreeServer(&blocks[0]);

    // C'è un servente libero, quindi genero il completamento
    if (s != NULL) {
        double serviceTime = getService(CONTROL_UNIT, s->stream);
        compl c = {s, INFINITY};
        c.value = clock.current + serviceTime;
        s->status = BUSY;  // Setto stato busy
        s->sum.service += serviceTime;
        s->block->area.service += serviceTime;
        s->sum.served++;
        //insertSorted(, c); ??????
        enqueue(&blocks[0], clock.arrival);  // lo appendo nella linked list di job del blocco 
    } else {
        enqueue(&blocks[0], clock.arrival);  // lo appendo nella linked list di job del blocco 
        blocks[0].jobInQueue++;              // Se non c'è un servente libero aumenta il numero di job in coda
    }
    clock.arrival = getArrival(clock.current);  // Genera prossimo arrivo
}

// Processa un next-event di completamento
void process_completion(compl c) {
    int block_type = c.server->block->type;
    blocks[block_type].total_completions++;
    blocks[block_type].jobInBlock--;

    int destination;
    server *freeServer;

    dequeue(&blocks[block_type]);  // Toglie il job servito dal blocco e fa "avanzare" la lista collegata di job
    //deleteElement(&global_sorted_completions, c); ??????

    // Se nel blocco ci sono job in coda, devo generare il prossimo completamento per il servente che si è liberato.
    if (blocks[block_type].jobInQueue > 0 && !c.server->need_resched) {
        blocks[block_type].jobInQueue--;
        double service_1 = getService(block_type, c.server->stream);
        c.value = clock.current + service_1;
        c.server->sum.service += service_1;
        c.server->sum.served++;
        c.server->block->area.service += service_1;
        //insertSorted(); ?????????????????????

    } else {
        c.server->status = IDLE;
    }

    // Se un server è schedulato per la terminazione, non prende un job dalla coda e và OFFLINE
    if (c.server->need_resched) { //WLAN - INTERMITTENT
        c.server->online = OFFLINE;
        c.server->need_resched = false;
    }

    //uscita dalla rete se il blocco esce dal CLOUD
    if (block_type == CLOUD_UNIT) {
        return;
    }

    // Gestione blocco destinazione
    destination = getDestination(c.server->block->type);  // Trova la destinazione adatta per il job appena servito ??
    if (destination == EXIT) {
       // blocks[block_type].total_dropped++; ??
       // dropped++; ??
        return;
    }
    if (destination != CLOUD_UNIT) {
        blocks[destination].total_arrivals++;
        blocks[destination].jobInBlock++;
        enqueue(&blocks[destination], c.value);  // Posiziono il job nella coda del blocco destinazione e gli imposto come tempo di arrivo quello di completamento

        // Se il blocco destinatario ha un servente libero, generiamo un tempo di completamento, altrimenti aumentiamo il numero di job in coda
        freeServer = findFreeServer(&blocks[destination]);
        if (freeServer != NULL) {
            compl c2 = {freeServer, INFINITY};
            double service_2 = getService(destination, freeServer->stream);
            c2.value = clock.current + service_2;
            //insertSorted(&);  ??????????????????'
            freeServer->status = BUSY;
            freeServer->sum.service += service_2;
            freeServer->sum.served++;
            freeServer->block->area.service += service_2;

            return;
        } else {
            blocks[destination].jobInQueue++; // .. vedere in base alla perdita IF FRAME_VIDEO...
            return;
        }
    }
}

// Ritorna il blocco destinazione di un job dopo il suo completamento
int getDestination(enum block_types from) {
    switch (from) {
        case CONTROL_UNIT:
            return routing_from_control_unit();
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

// Fornisce il codice del blocco di destinazione partendo dal blocco di controllo iniziale
//dispatcher
int routing_from_control_unit() {

}

//Thread che disattiva la WLAN essendo un server intermittente 
int intermittent_wlan() {

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
    int i;
    int n = compls->num_completions;

    int pos = binarySearch(compls, 0, n - 1, completion);

    if (pos == -1) {
        printf("Element not found");
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

//Calcola l'energia consumata dal sistema (capire come aggiornare variabili per ogni esecuzione)
int calculate_energy_consumption() {

}



int initialize() {
   streamID=0;
   clock.current = START;
    
    for (int block_type = 0; block_type < NUM_BLOCKS; block_type++) {
        blocks[block_type].type = block_type;
        blocks[block_type].jobInBlock = 0;
        blocks[block_type].jobInQueue = 0;
        blocks[block_type].total_arrivals = 0;
        blocks[block_type].total_completions = 0;
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
       streamID=streamID++;
   }

   for(int i=0;i<blocks[2].num_servers;i++) {
         (*wlan_unit+i)->id=i;
         (*wlan_unit+i)->status=IDLE;
         (*wlan_unit+i)->online=ONLINE;
         (*wlan_unit+i)->loss=NOT_LOSS_SYSTEM;
         (*wlan_unit+i)->stream=streamID++;
         (*wlan_unit+i)->block=&blocks[2];
         streamID=streamID++;
   }

   (*enode_unit)->id=6;
   (*enode_unit)->status=IDLE;
   (*enode_unit)->online=ONLINE;
   (*enode_unit)->loss=NOT_LOSS_SYSTEM;
   (*enode_unit)->stream=streamID++;
   (*enode_unit)->block=&blocks[3];
   streamID=streamID++;
   
   for(int i=0;i<blocks[4].num_servers;i++) {
          (*edge_unit+i)->id=i;
          (*edge_unit+i)->status=IDLE;
          (*edge_unit+i)->online=ONLINE;
          (*edge_unit+i)->loss=NOT_LOSS_SYSTEM;
          (*edge_unit+i)->stream=streamID++;
          (*edge_unit+i)->block=&blocks[4];
          streamID=streamID++;
   }

   (*cloud_unit)->id=11;
   (*cloud_unit)->status=IDLE;
   (*cloud_unit)->online=ONLINE;
   (*cloud_unit)->loss=NOT_LOSS_SYSTEM;
   (*cloud_unit)->stream=streamID++;
   (*cloud_unit)->block=&blocks[5];

   printf("start memcpy\n");
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
   printf("Welcome\n");
   initialize();
   printf("%ld\n",blocks[0].num_servers);
   printf("%ld\n",blocks[1].num_servers);
   printf("%ld\n",blocks[2].num_servers);
   printf("%ld\n",blocks[3].num_servers);
   printf("%ld\n",blocks[4].num_servers);
   printf("%ld\n",blocks[5].num_servers);

   enqueue(&blocks[0],2305345.0);
   double arrival =blocks[0].head_queue->arrival;
   printf("job in queueu %f\n",arrival);
     // Compute Erlang-B blocking probability using recursive function E()

 
   double     prob_block;    // Erlang-B blocking probability
  
   prob_block = E(2, 0.67);
   printf("= Pr[block] (Erlang-B) = %f \n", prob_block);
  

}