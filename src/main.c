#include <stdio.h>
#include <config.h>


// Genera un tempo di arrivo secondo la distribuzione Esponenziale
double getArrival(double current) {
    double arrival = current;
    SelectStream(254);
    arrival += Exponential(1 / arrival_rate);
    return arrival;
}

// Genera un tempo di servizio esponenziale di media specificata e stream del servente individuato
double getService(int type_service, int stream) {
    SelectStream(stream);

    switch (type) {
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
void enqueue(struct block *block, double arrival) {
    struct job *j = (struct job *)malloc(sizeof(struct job));
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

   control_unit->id=0;
   control_unit->status=ONLINE;
   control_unit->loss=NOT_LOSS_SYSTEM;
   control_unit->stream=streamID++;
   control_unit->block=&blocks[0];

   video_unit->id=2;
   video_unit->status=ONLINE;
   video_unit->loss=LOSS_SYSTEM;
   video_unit->stream=streamID++;
   video_unit->block=&blocks[1];

   video_unit->id=3;
   video_unit->status=ONLINE;
   video_unit->loss=LOSS_SYSTEM;
   video_unit->stream=streamID++;
   video_unit->block=&blocks[1];

   wlan_unit->id=4;
   wlan_unit->status=ONLINE;
   wlan_unit->loss=NOT_LOSS_SYSTEM;
   wlan_unit->stream=streamID++;
   wlan_unit->block=&blocks[2];

   wlan_unit->id=5;
   wlan_unit->status=ONLINE;
   wlan_unit->loss=NOT_LOSS_SYSTEM;
   wlan_unit->stream=streamID++;
   wlan_unit->block=&blocks[2];

   enode_unit->id=6;
   enode_unit->status=ONLINE;
   enode_unit->loss=NOT_LOSS_SYSTEM;
   enode_unit->stream=streamID++;
   enode_unit->block=&blocks[3];

   edge_unit->id=7;
   edge_unit->status=ONLINE;
   edge_unit->loss=NOT_LOSS_SYSTEM;
   edge_unit->stream=streamID++;
   edge_unit->block=&blocks[4];

   edge_unit->id=8;
   edge_unit->status=ONLINE;
   edge_unit->loss=NOT_LOSS_SYSTEM;
   edge_unit->stream=streamID++;
   edge_unit->block=&blocks[4];

   edge_unit->id=9;
   edge_unit->status=ONLINE;
   edge_unit->loss=NOT_LOSS_SYSTEM;
   edge_unit->stream=streamID++;
   edge_unit->block=&blocks[4];

   edge_unit->id=10;
   edge_unit->status=ONLINE;
   edge_unit->loss=NOT_LOSS_SYSTEM;
   edge_unit->stream=streamID++;
   edge_unit->block=&blocks[4];

   cloud_unit->id=11;
   cloud_unit->status=ONLINE;
   cloud_unit->loss=NOT_LOSS_SYSTEM;
   cloud_unit->stream=streamID++;
   cloud_unit->block=&blocks[5];


   clock.arrival = getArrival(clock.current);
}
 

int main(void) {
   printf("Hello, World!");
}