#include"DVP.h"
#include<strings.h>

void build_update_packet(int destID, unsigned short newCost);

/**
 * Builds message packet from routing table in memory.
 * @param None
 * @return void
 */
 void build_packet()
 {
     //Clear all packet values
     memset((packet *)&send_pack,0,sizeof(packet));
     
     //Create packet header
     msgHead h;
     
     //Parse IP Address
     //char Ip[50];
     //inet_pton(AF_INET,(void *)&my_details.ip, (void *)&Ip);
     //printf("DEBUG: Parsed IP address is : %s",Ip);
     
     //Populate packet header
     h.no_of_fields= (unsigned short) server_no;
     h.server_port=(unsigned short) my_details.port;
     h.sender_ip=current.sin_addr.s_addr;
     
     //printf("\nIP (unsigned int) : %u",h.sender_ip);
     
     //Create packet body
     msgBody b[5];
     
     int i=0;                                 //Loop variable 
     
     //Populate packet body 
     for(i=0;i<server_no;i++){
         b[i].serv_port= (unsigned short) serv_list[i].port;
         b[i].serv_ID=(unsigned short) serv_list[i].id;
         b[i].cost=(unsigned short) rout_table[my_index][i];
     }
     
     //Add header and body to packet 
     memcpy((msgHead *)&send_pack.header,(msgHead *)&h,sizeof(msgHead));
     memcpy((void *)&send_pack.bodyentries,(void *)&b,sizeof(b));
     
     /*strcpy(msg_packet,"04");                   //Fix number of update fields
     
     char *myport;                            //Temp field to store char equivalent of port ID
     
     myport= malloc(sizeof(my_details.port));
     memcpy(myport, (char*)&my_details.port,sizeof(my_details.port));
     //printf("\nPort value is: %s",myport);
     unsigned int port=9999;
     
     strcat(msg_packet,myport);                //Add sender's port no to message packet
     memcpy((char*)&port,myport,sizeof(my_details.port));
     printf("\nExtracted Port is : %d",port);
     
     strcat(msg_packet, my_details.ip);         //Add sender's IP to msg packet
      */
 }
 
 /**
 * Builds message packet for update_command
 * @param Destination node ID
 * @return void
 */
 void build_update_packet(int destID, unsigned short newCost)
 {
    //Clear all packet values
    memset((packet *)&update_pack,0,sizeof(packet));
    
    //Get server index 
    int dest_idx=get_serv_index(destID);
    
    //Create packet header
     msgHead h;
     
     //Populate packet header
     h.no_of_fields= update;
     h.server_port=(unsigned short) my_details.port;
     h.sender_ip=current.sin_addr.s_addr;
     
     //Create packet body
     msgBody b[5];
     
     //Populate packet body 
     b[0].serv_port= (unsigned short) serv_list[dest_idx].port;
     b[0].serv_ID=(unsigned short) serv_list[dest_idx].id;
     b[0].cost=newCost;
     
     //Add header and body to packet 
     memcpy((msgHead *)&update_pack.header,(msgHead *)&h,sizeof(msgHead));
     memcpy((void *)&update_pack.bodyentries,(void *)&b,sizeof(b));
     
 }


/** 
 * Calculates the shortest path to each node by applying the 
 * Bellman-Ford equation. Also, updates the next hop values
 * 
 * @param none
 * @return none
 */
void calc_DV()
{
    //routing_table();
    
    //Update routing table with received values
    unsigned int i,j;                           //Loop variables
    
    //Get position of current process in cost and next hop matrices 
    int my_index=get_serv_index(my_details.id);
    //int weights[4]={0,0,0,0};
    
    int min,min_next_hop;
    
    //Update link costs as per the init vector values
    for(i=0;i<neighbours;i++){
        int neighbour_index=get_serv_index(neighbour_IDs[i]);
        
        //Update matrices if cost matrix entry is greater than initial cost and node is active
        if((rout_table[my_index][neighbour_index]>init_vector[i])&&(node_status[neighbour_index]==active_neighbour))
        {
            
            //Update cost matrix
            rout_table[my_index][neighbour_index]=init_vector[i];
            //rout_table[neighbour_index][my_index]=init_vector[i];
            
            //Update hop matrix
            if(rout_table[my_index][neighbour_index]!=infinity)
                next_hop[neighbour_index]=neighbour_IDs[i];
            else
                next_hop[neighbour_index]=my_details.id;
            //next_hop[neighbour_index][my_index]=neighbour_IDs[i];
        }
    }
    
    //Calculate distance between this servers and all other servers in the network
    for(i=0;i<server_no;i++)
    {
        //Set current link cost as minimum link cost
        min=rout_table[my_index][i];
        min_next_hop=next_hop[i];
        
        //Define variable to store the sum term of Bellman-Ford equation
        int sum=0;
        
        //Apply Bellman-Ford equation when the two nodes are different and node is active 
        if((i!=my_index)&&(node_status[i]!=disabled))
        {
            //Iterate through all neighbours of current process
            for(j=0;j<neighbours;j++)
            {
                //Get position of neighbour ID in routing table
                int neighbour_index=get_serv_index(neighbour_IDs[j]);
                
                //Check if current neighbour is active
                if(node_status[neighbour_index]==active_neighbour)
                {
                    //Calculate d(x,v)+d(v,y) for current neighbour 'v'
                    sum=rout_table[my_index][neighbour_index]+rout_table[neighbour_index][i];
                    
                    //Save minimum value found so far
                    if(sum<min)
                    {
                        min=sum;
                        min_next_hop=neighbour_IDs[j];
                    }
                }
            }
            
            //Update link costs in routing table
            rout_table[my_index][i]=min;
            //rout_table[i][my_index]=min;
            
            //Update next hop values in hop rout_table
            if(rout_table[my_index][i]==infinity)
                next_hop[i]=serv_list[my_index].id;
            else
                next_hop[i]=min_next_hop;
            //next_hop[i][my_index]=min_next_hop;
        }else if(node_status[i]==disabled)
        {
            rout_table[my_index][i]=infinity;
            next_hop[i]=my_details.id;
        }
        
    }
    
}

/**
 * Populates data structure containing the details of the host process
 * 
 * @param None
 * @return void
 */
void fill_mydet()
{
    char match='f';         //Flag to check if current server exists in list of servers read from file;
    unsigned int i;         //Loop variable
    
    getPublicIP();
    
    for(i=0;i<server_no;i++)
        if(strcmp(my_details.ip,serv_list[i].ip)==0)
        {
            my_details.id=serv_list[i].id;
            my_details.port=serv_list[i].port;
            match='t';
        }
    
    if(match=='f')
    {
        exit_fn("Host server doesn't exist in topology file");
    }
    
}

