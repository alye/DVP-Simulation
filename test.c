#include"DVP.h"

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



void print_values(){
    int i,j;
    
    for(i=0;i<server_no;i++)
    {
        printf("\n");
        for(j=0;j<server_no;j++)
            if(rout_table[i][j]==infinity){
                printf("inf\t");
            }else{
            printf("%d\t",rout_table[i][j]);
            }
    }
    printf("\n");
    
    for(i=0;i<server_no;i++)
        printf("\nList of server IDs, IPs and ports is : {%d , %s , %d }",serv_list[i].id,serv_list[i].ip,serv_list[i].port);
        
    
    
}

/** void get_my_IP()
{
    struct ifreq ifr;

    my_fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    /* ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    /* strncpy(ifr.ifr_name,"eth0", IFNAMSIZ-1);

    ioctl(my_fd, SIOCGIFADDR, &ifr);

    
    my_details.ip=inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    
    /* display result */
    /* printf("%s\n",my_details.ip);

} 

*/





/**
 * Prints hop-list
 */
 void print_hop(){
     int i;
     for(i=0;i<server_no;i++)
        printf("\nWhile going from ID%d to ID%d , next hop will be ID%d",my_details.id,(i+1),next_hop[i]);
    
    fflush(stdout);
 }
 
 
/**
 * Mock routing table
 * Used for testing only!!
 */
void routing_table()
{
    unsigned int i,j;
    for(i=0;i<server_no;i++)
        for(j=0;j<server_no;j++)
            if(i!=j)
            {
                recv_table[i][j]=1;
            }else
            {
                recv_table[i][j]=0;
            }
            
    //Update routing table with received value
    for(i=0;i<server_no;i++)
        for(j=0;j<server_no;j++)
            if(!(i==my_index || j==my_index))
            {
                rout_table[i][j]=recv_table[i][j];
            }
            
    
}
