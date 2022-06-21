#define NUM_BLOCKS 6

//Input values
#define LAMBDA_REMOTE
#define LAMBDA_VIDEO
#define WLAN_P 0.3

//services time 
#define CONTROL_UNIT_SERVICE_TIME 0.1
#define VIDEO_SERVICE_TIME 0.34  //acquisizione di un frame da video service
#define WLAN_FRAME_UPLOAD_TIME 0.34 //us
#define ENODE_FRAME_UPLOAD_TIME 0.34
#define EDGE_PROCESSING_TIME 0.34
#define CLOUD_PROCESSING_TIME 0.34

//delay
#define DELAY_FROM_EDGE_TO_CLOUD 1
#define DELAY_TO_EDGE 0.1


//data structure
// Struttura che mantiene il clock
struct clock_t {
    double current;  // Tempo attuale di simulazione
    double next;     // Tempo attuale del prossimo evento, sia arrivo che completamento
    double arrival;  // Tempo attuale del prossimo arrivo
    double batch_current;
};

// Struttura che mantiene la somma accumulata
struct sum {
    double service;  // Tempi di servizio
    long served;     // Numero di job serviti
};

// Struttura che mantiene un job. Il puntatore *next implementa la Linked List
struct job {
    double arrival;
    struct job *next;
};

// Servente
typedef struct server_t {
    int id;
    int stream;
    int status; //{ONLINE=0,OFFLINE=1}
    struct block *block;
    struct sum sum;
    bool need_resched;
    int loss;
} server;

server control_unit[1];
server video_unit[2];
server wlan_unit[2];
server enode_unit[1];
server edge_unit[4];
server cloud_unit[1]; 

struct area {
    double node;    /* time integrated number in the node  */
    double queue;   /* time integrated number in the queue */
    double service; /* time integrated number in service */
};

// Blocco
struct block {
    struct job *head_service;
    struct job *tail;
    struct job *head_queue;
    double active_time;
    int jobInQueue;
    int jobInBlock;

    int batch_block;
    int batch_queue;

    int batch_arrivals;
    int total_arrivals;
    int total_completions;
    double service_rate;
    struct area area;
};

// Struttura che mantiene un completamento su un server
typedef struct {
    server *server;
    double value;
} complement;

// --------------------------------------------------------------------------------------------------
