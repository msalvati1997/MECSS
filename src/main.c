#include "../include/config.h"
#include "../include/rngs.h"
#include "../include/rvgs.h"
#include "../include/rvms.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
server *findFreeServer(block b) {
    int block_type = b.type;
    for (int i = 0; i < b.num_servers; i++) {
    }
    return NULL;
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
   (*control_unit)->status=ONLINE;
   (*control_unit)->loss=NOT_LOSS_SYSTEM;
   (*control_unit)->stream=streamID++;
   streamID=streamID++;
   (*control_unit)->block=&blocks[0];
  
   printf("control_unit initialized\n");
   for(int i=0;i<blocks[1].num_servers;i++) {
       (*video_unit+i)->id=i;
       (*video_unit+i)->status=ONLINE;
       (*video_unit+i)->loss=LOSS_SYSTEM;
       (*video_unit+i)->stream=streamID++;
       (*video_unit+i)->block=&blocks[1];  
       streamID=streamID++;
   }

   for(int i=0;i<blocks[2].num_servers;i++) {
         (*wlan_unit+i)->id=i;
         (*wlan_unit+i)->status=ONLINE;
         (*wlan_unit+i)->loss=NOT_LOSS_SYSTEM;
         (*wlan_unit+i)->stream=streamID++;
         (*wlan_unit+i)->block=&blocks[2];
         streamID=streamID++;
   }

   (*enode_unit)->id=6;
   (*enode_unit)->status=ONLINE;
   (*enode_unit)->loss=NOT_LOSS_SYSTEM;
   (*enode_unit)->stream=streamID++;
   (*enode_unit)->block=&blocks[3];
   streamID=streamID++;
   
   for(int i=0;i<blocks[4].num_servers;i++) {
          (*edge_unit+i)->id=i;
          (*edge_unit+i)->status=ONLINE;
          (*edge_unit+i)->loss=NOT_LOSS_SYSTEM;
          (*edge_unit+i)->stream=streamID++;
          (*edge_unit+i)->block=&blocks[4];
          streamID=streamID++;
   }

   (*cloud_unit)->id=11;
   (*cloud_unit)->status=ONLINE;
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

}