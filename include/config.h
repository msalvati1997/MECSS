#include <stdbool.h>

#define NUM_BLOCKS 6
#define NUM_METRICS 10
#define START 0.0
#define STOP  20000.0 //100.0 * 24.0 * 60.0 * 60.0
#define INFINITY (100.0 * STOP)

#define WLAN_P 0.11
#define ARRIVAL_RATE 60.0

//services time 
#define CONTROL_UNIT_SERVICE_TIME 0.0113674698
#define VIDEO_SERVICE_TIME  0.014515384  //acquisizione di un frame da video service
#define WLAN_FRAME_UPLOAD_TIME  0.0629 
#define ENODE_FRAME_UPLOAD_TIME 0.1887
#define EDGE_PROCESSING_TIME 0.0125799999
#define CLOUD_PROCESSING_TIME 0.0673928571


//delay
#define DELAY_FROM_EDGE_TO_CLOUD 1.0
#define DELAY_TO_EDGE 0.1

#define ONLINE 0
#define OFFLINE 1
#define BUSY 2
#define IDLE 3
#define LOSS_SYSTEM 4
#define NOT_LOSS_SYSTEM 5
#define EXTERNAL 6  //arrivo esterno
#define INTERNAL 7 //arrivo interno 
#define EXIT 32
#define P_WLAN 0.6
#define P_OFF_WLAN 0.11
// Numero di ripetizioni e batch
#define NUM_REPETITIONS 128
#define BATCH_B 1024
#define BATCH_K 128

#define handle_error(msg)   \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

int streamID; 
typedef struct server_t server;
typedef struct block_t block;
typedef struct complement_t compl;
typedef struct job_t job;
static const sorted_completions empty_sorted;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//data structure
//Struttura che mantiene il clock
struct clock_t {
    double current;  // Tempo attuale di simulazione
    double next;     // Tempo attuale del prossimo evento, sia arrivo che completamento
    double arrival;  // Tempo attuale del prossimo arrivo
    double batch_current;
};

// Struttura che mantiene la somma accumulata
struct sum_t {
    double service;  // Tempi di servizio
    long served;     // Numero di job serviti
};

// Struttura che mantiene un job. Il puntatore *next implementa la Linked List
struct job_t {
    double arrival;
    job *next;
    int type; // {LOCAL;INTERNAL}
};

enum block_types {
    CONTROL_UNIT,
    VIDEO_UNIT,
    WLAN_UNIT,
    ENODE_UNIT,
    EDGE_UNIT,
    CLOUD_UNIT,
};

struct area_t {
    double node;    /* time integrated number in the node  */
    double queue;   /* time integrated number in the queue */
    double service; /* time integrated number in service */
};

// Blocco
struct block_t {
    job *head_service;
    job *tail;
    job *head_queue;
    double active_time;
    int jobInQueue;
    int jobInBlock;
    int num_servers;

    int batch_block;
    int batch_queue;
    enum block_types type;

    int batch_arrivals;
    int total_arrivals;
    int total_completions;
    int total_bypassed;
    int total_dropped;
    double service_rate;
    struct area_t area;

    server **serv;
};

// Servente
struct server_t {
    int id;
    int stream;
    int online; //{ONLINE=0,OFFLINE=1}
    int status; //{BUSY=0,IDLE=1}
    block *block;
    struct sum_t sum;
    bool need_resched;
    int loss;
};

//server list
server **control_unit;
server **video_unit;
server **wlan_unit;
server **enode_unit;
server **edge_unit;
server **cloud_unit; 

// Struttura che mantiene un completamento su un server
struct complement_t {
    server *server;
    double value; //tempo di completamento
};

// Struttura che mantiene la lista ordinata di tutti i completamenti
typedef struct {
    compl sorted_list[11];
    int num_completions;
} sorted_completions;


sorted_completions global_sorted_completions;  // Tiene in una lista ordinata tutti i completamenti nella rete così da ottenere il prossimo in O(log(N))
block blocks[NUM_BLOCKS];
struct clock_t clock;                          // Mantiene le informazioni sul clock di simulazione
int completed;
int bypassed;
int dropped;
// --------------------------------------------------------------------------------------------------


