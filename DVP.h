/**
 * Header file used to define extern(global) parameters
 * and include header files common to all source files.
 * 
 * @author Alizishaan Khatri
 */
 
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include<stdint.h>

/* Macro definitions */
#define infinity 65535                  //'infinity' is defined as the maximum value of an unsigned 2-byte datatype
#define disabled 'd'
#define update 10
#define active_neighbour 'a'
#define no_direct_link 'u'

/* Custom datatype to store server info */
struct server_info{
    int id;
    char *ip;
    short port;
};

typedef struct server_info Serv;

/* Custom datatype to transmit/receive message Header */
typedef struct msg_header{
    unsigned short no_of_fields;
    unsigned short server_port;
    unsigned int sender_ip;
}msgHead;

/* Custom datatype to send/receive individual server information */
typedef struct msg_entry{
    unsigned short serv_port;
    unsigned short blank;
    unsigned short serv_ID;
    unsigned short cost;
}msgBody; 

/* Custom datatype to store packets */
typedef struct pack{
    msgHead header;
    msgBody bodyentries[5];
}packet;

/* Declaring extern variables */
extern int rout_table[5][5];                //Routing Table
extern int next_hop[5];                     //Store a list of next hops
extern int server_no;                       //Store number of servers
extern int neighbours;                      //Store no of neighbours of given node
extern struct server_info serv_list[5];     //Store server details
extern int neighbour_IDs[4];                //Store IDs of all the neighbouring nodes
extern struct server_info my_details;       //Store details of the host process
extern int my_fd,fd;                        //Store File Descriptor of Host Socket
extern char msg_packet[100];                //Store packet that will be shared between hosts
extern int recv_table[5][5];                //Stores copy of routing table received from neighbour
extern int init_vector[4];                  //Store the initial weights
extern int rec_packets;                     //Stores the current no of packets received
extern int my_index;                        //Stores the index of the host process 
extern packet send_pack;                    //Stores the packet to be sent
extern packet update_pack;                  //Stores the update packet to be sent 
extern struct sockaddr_in current;          //Save current socket info
extern char node_status[5];                 //Store status of each node in system
extern int inactiveCount[5];                //Store no of cycles for which a particular neighbour has been inactive


/* Function declarations */
void exit_fn(char *msg);
void fill_mydet();
int get_serv_index(int id);
void exit_fn(char *msg);
void file_process();
void sort_servers();
int get_serv_index(int id);
void init_table();
void calc_DV();
void comm_step();


void build_packet();
void build_update_packet(int destID, unsigned short newCost);

void print_hop();
void print_values();
void routing_table();


int sock_init();
void sock_read();
void send_sock(int index,packet p);

void getPublicIP();
void comm_process(char *comm_line);
