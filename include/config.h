#define NUM_BLOCKS 6
#define START 0.0
#define STOP  20000.0 //100.0 * 24.0 * 60.0 * 60.0

//Input values
//#define LAMBDA_REMOTE 
//#define LAMBDA_VIDEO
#define WLAN_P 0.11

//services time 
#define CONTROL_UNIT_SERVICE_TIME 0,7831325201
#define VIDEO_SERVICE_TIME 1.0  //acquisizione di un frame da video service
#define WLAN_FRAME_UPLOAD_TIME 4.33333333333 //us
#define ENODE_FRAME_UPLOAD_TIME 13.0
#define EDGE_PROCESSING_TIME 0.86666666666
#define CLOUD_PROCESSING_TIME 0.4642857142

//delay
#define DELAY_FROM_EDGE_TO_CLOUD 1.0
#define DELAY_TO_EDGE 0.1

#define ONLINE 0
#define OFFLINE 1
#define LOSS_SYSTEM 0
#define NOT_LOSS_SYSTEM 1

int streamID; 

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
    int type; //local or remote
};

// Servente
typedef struct server_t {
    int id;
    int stream;
    int online; //{ONLINE=0,OFFLINE=1}
    int status; //{BUSY=0,IDLE=1}
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

enum block_types {
    control_unit,
    video_unit,
    wlan_unit,
    enode_unit,
    cloud_unit,
};

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
    enum block_types type;

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

struct block blocks[NUM_BLOCKS];

// Numero di ripetizioni e batch
#define NUM_REPETITIONS 128
#define BATCH_B 1024
#define BATCH_K 128



// --------------------------------------------------------------------------------------------------

//finite_horizon_simulation 
#define STOP_SIMULATION 3

