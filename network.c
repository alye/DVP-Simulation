#define BUFSIZE 2048

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<strings.h>

#include"DVP.h"


struct sockaddr_in myaddr;              /* our address */
struct sockaddr_in remaddr;             /* remote address */
socklen_t addrlen = sizeof(remaddr);    /* length of addresses */
int recvlen;                            /* # bytes received */
int fd;                                 /* our socket */
unsigned char buf[BUFSIZE];              /* receive buffer */
struct sockaddr_in current;             /* Save current socket info */

int PORT;
struct server_info dest;

/**
 * Function Declarations
 */
 int sock_init();
 void sock_read();
 void send_sock(int index,packet p);
 void getPublicIP();
 int get_index_by_IP(char ip[]);
 
/**
 * Get Public IP and File Descriptor of the host process
 * @param   None
 * @returns void
 */
void getPublicIP()
{
	struct addrinfo hints, *res;
	
	
	socklen_t len = sizeof current;
	void *addr;
	
	char ipstr[INET_ADDRSTRLEN];
	char myHostname[1024];
	char service[50];
	
	 
	memset(&hints, 0, sizeof hints);    //Set all values of variable 'hints' to zero
	hints.ai_family = AF_INET;          //Set IP address type as IPv4
	hints.ai_socktype = SOCK_DGRAM;     //Set socket type as Datagram
	
	if(getaddrinfo("8.8.8.8", "53", &hints, &res) !=0)
	{
		exit_fn("\nFailed to get public IP");
	}
	
	my_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	if(my_fd == -1)
		exit_fn("\nsocket");
		
	if(connect(my_fd, res->ai_addr, res->ai_addrlen) == -1)
		exit_fn("\nError connecting the host process's socket");
	
	if(getsockname(my_fd, (struct sockaddr*)&current, &len) == -1)
		exit_fn("\nError getting the host process's socket name");
	
	addr = &(current.sin_addr);
	
	if(inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr)) == NULL)
		exit_fn("\nError on inet_ntop converstion");
	
	ipstr[strlen(ipstr)] = 0;
	
	my_details.ip=ipstr;
	
	close(my_fd);
	
	//printf("\nIP read is : %s %s",my_details.ip,ipstr);
	
	

}

/**
 * Initializes a listener socket on the host process
 * 
 * @param None
 * @return The file descriptor of the listener socket
 */
int sock_init()
{

    /* create a UDP socket */
    PORT=my_details.port;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        exit_fn("Cannot create socket\n");
    } 
    

    /* bind the socket to any valid IP address and a specific port */

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(PORT);
    
    //printf("\nDEBUG: Socket bound on host port, network port : %u %u",PORT, myaddr.sin_port);
    
    int yes=1;

    // lose the pesky "Address already in use" error message
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        exit_fn("Error on setsockopt");
    } 
    
    //Bind socket to local address
    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
    {
       exit_fn("Binding failed");
    }
        
    return(fd);
}

/**
 * Reads incoming data from listner fd initialized by previous method
 */
void sock_read(){
    
    int rcv_pkt_size,remaddrsize=sizeof(remaddr);
    
    packet pkt;
    
    //int ip;
    unsigned short port1,id,cost;
    char ip[50];
    
    memset((packet *)&pkt,0,sizeof(packet));
    memset((void *)&ip,'\0',sizeof(ip));
    
    //Receive packet
    rcv_pkt_size=recvfrom(fd,&pkt,sizeof(pkt),0,(struct sockaddr *)&remaddr,&remaddrsize);
    
    //Get sender IP
    inet_ntop(AF_INET,(unsigned int *)&pkt.header.sender_ip, (char *)&ip, sizeof(ip));
    
    printf("Sender's address is : %s",ip);
    
    //Identify sender
    int sender_index;                   //Stores index of process that sent the packet 
    int i;                              //loop variable
        
    sender_index=get_index_by_IP(ip);
    
    //Reset inactivity counter for sender
    inactiveCount[sender_index]=0;
    
    //Extract no of no_of_fields in packet 
    int fields=pkt.header.no_of_fields;
    
    printf("\nRECEIVED A MESSAGE FROM SERVER : %d",serv_list[sender_index].id);
    if(fields==update)
    {    //Update Packet has been detected
        //if(node_status[sender_index]!=active_neighbour)
        //{
            //printf("\nUpdate packet can only come from an active neighbour");
            //return;
        //}else
        //{
            //Update cost matrix of routing table
            rout_table[my_index][sender_index]=pkt.bodyentries[0].cost;
            
            /* Update init vector */
            i=0;
            
            //Find position of sender in neighbours list 
            while(get_serv_index(neighbour_IDs[i])!=sender_index){
                i++;
            }
            
            //Update init vector 
            init_vector[i]=pkt.bodyentries[0].cost;
        //}
        
    }else
    {
        //Normal packet has been detected
        
        /*
        //Check sender's validity
        if(sender_index==infinity)
        {
            printf("\nPacket contains invalid IP address.");
            return;
        }else if(node_status[sender_index]==disabled)
        {
            printf("\nDiscarding packet received from disabled server ID: %d",serv_list[sender_index].id);
            return;
        }else if(node_status[sender_index]==no_direct_link)
        {
            printf("\nDiscarding packet received from non-neighbour: %d",serv_list[sender_index].id);
            return;
        }
        */
        
        //Check packet validity
        if(fields!=server_no)
        {
            printf("\nReceived packet with invalid number of fields");
            return;
        }else
        {
            //Update cost matrix values using received Packet
            for(i=0;i<server_no;i++)
            {
                if(node_status[i]!=disabled)
                {
                    rout_table[sender_index][i]=pkt.bodyentries[i].cost;   
                }else
                {
                    rout_table[sender_index][i]=infinity;
                }
            }
        }
        
        //Compute distance vector using updated cost matrix
        calc_DV();
    }
    //sprintf(ip,"%d",pkt.header.sender_ip);
    //printf("\nReceived packet is: \nHeader:\nNo. of fields: %u\nServer port: %u\nSender IP:%s,%u",pkt.header.no_of_fields,port1,ip,pkt.header.sender_ip);
    
    //Print body
    //int i;
    /*for(i=0;i<5;i++){
        port1=pkt.bodyentries[i].serv_port;
        id=pkt.bodyentries[i].serv_ID;
        cost=pkt.bodyentries[i].cost;
        printf("\nBody entry %d is:",(i+1));
        printf("\nPort :%u\nID :%u\nCost :%u",port1,id,cost);
    }
    
    printf("\nReceived values: %u",pkt.header.no_of_fields); */
}

/** 
 * Sends a string to the specified server_info
 * 
 * @param dest The destination server (server_info type)
 * @param data The string to be transmitted
 * 
 * @return void
 * 
 */
 void send_sock(int index,packet p)
 {
    //build_packet();
    
    Serv dest=serv_list[index]; 
    int out_port=dest.port;
    int file_desc;
    
    packet data;
    
    //=malloc(sizeof(packet) * strlen("Sending works!/0"));
    
    data=p;
    
    //char data[]="abc";
    //data=100f;
    
    //strcpy(data,"Sending works!");
    //unsigned char *data='a';
    
    
    //Field to store destination socket address
    //struct sockaddr destination;
    
    /*
    //Create sender socket 
    if ((file_desc=socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {
        exit_fn("\nUnable to create socket for writing\n");
    } */
    
    //Build destination addresses
    memset((struct sockaddr *)&remaddr, 0, sizeof(remaddr));
    remaddr.sin_family=AF_INET;
    remaddr.sin_port=htons(dest.port);
    int pton = inet_pton(AF_INET,dest.ip,(void *)&remaddr.sin_addr);
    
    //printf("\nDEBUG: Destination IP is : %s",dest.ip);
    //printf("\nDEBUG: Destination port is : host %u , network %u",dest.port,remaddr.sin_port);
    //printf("\nDEBUG: pton return value is : %d",pton);
    //printf("\nDEBUG: IP Address value in remaddr is : %u ",(unsigned int) remaddr.sin_addr.s_addr);
    
    //remaddr.sin_addr.s_addr
    //inet_ntop(AF_INET,(unsigned int *)&pkt.header.sender_ip, (char *)&ip, sizeof(ip));
    
    int send_to=0;
    //Send the data to the above address
    if ((send_to=sendto(fd,&data,sizeof(data),0,(struct sockaddr *) &remaddr,sizeof(remaddr)))==-1)
    {
        exit_fn("\nError sending data");
    }
    
    //printf("\nDEBUG: send_to value: %d :",send_to);
    
    //free(data);
    //close(file_desc);
 }


/**
 * Get index by IP address
 * 
 * @param char[] IP Address 
 * 
 * @return index of node
 *
 */
 int get_index_by_IP(char ip[]){
     int i=0;
     
     while(i<server_no)
     {
         if((strcmp(ip,serv_list[i].ip)==0)){
             return(i);
         }
         i++;
     }
     return(infinity);
 }
