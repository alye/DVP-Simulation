#include"DVP.h"

/* Function Declarations */
void comm_update(unsigned short destID, unsigned short cost);
void disable(int ID);
void comm_step();


void comm_process(char *comm_line){
    
    char *comm_args[10];                    //Stores tokenized arguments of the input command
    char temp[999];                         //Create a copy of entered command line value 
    int i=0;                                //Counter
    unsigned int arguments=0;               //Stores no of command line arguments
    
    //Set all elements of temp array to '\0'
    memset(temp,'\0',sizeof(temp));
    
    //Copy input string
    strcpy(temp,comm_line);
    
    //Extract command from the user entered string
    comm_args[0]=strtok(comm_line," ");
    
    //printf("\nDEBUG: comm_args[0] is : %s",comm_args[0]);
    
    //Extract arguments
    while(comm_args[i]){
        
        comm_args[++i]=strtok(NULL," ");
        
        if(i==5 || comm_args[i]==NULL)
        {
            arguments=i-1;
            break;
        }
        
        //printf("\nDEBUG: comm_args[%d] is :%s",i,comm_args[i]);
    }
    
    /* if(comm_args[0]==NULL){
        comm_args[0]=temp;
    } */
    
    //Copy original value of command line back to it
    comm_line=temp;
    
    //printf("\nDEBUG: comm_args[0] is :%s",comm_args[0]);
    //printf("\nDEBUG: strncasecmp = %d",strncasecmp(comm_args[0],"display",sizeof("display")-1));
    
    //Command ladder
    if((strncasecmp(comm_args[0],"step",sizeof("step")-1)==0)&&(arguments==0))
    {
        //Step command has been detected
        comm_step();
        printf("\nstep SUCCESS");
    }else if((strncasecmp(comm_args[0],"packets",sizeof("packets")-1)==0)&&(arguments==0))
    {
        //Packets command has been detected
        printf("\nNo of packets received : %d",rec_packets);
        rec_packets=0;
        printf("\npackets SUCCESS");
    }else if((strncasecmp(comm_args[0],"crash",sizeof("crash")-1)==0)&&(arguments==0))
    {   //'Crash' command detected
    
        //Process crash command
        
        for(i=0;i<5;i++)
            node_status[i]=disabled;
        
        printf("\ncrash SUCCESS");
        
        exit(0);
        
    }else if((strncasecmp(comm_args[0],"display",sizeof("display")-1)==0)&&(arguments==0))
    {
        //'Display' command detected
        printf("\nDisplaying Routing Table:");
        printf("\nDestination(Server ID)\t\tNext Hop\t\tCost");
        
        //Display all the servers in list  
        for(i=0;i<server_no;i++)
            if((rout_table[my_index][i]!=infinity)&&(rout_table[my_index][i]!=disabled))
                printf("\n\t%d\t\t\t%d\t\t\t%d",serv_list[i].id,next_hop[i],rout_table[my_index][i]);
            else if (rout_table[my_index][i]==infinity)
                printf("\n\t%d\t\t\t%d\t\t\tinfinity",serv_list[i].id,next_hop[i]);
        
        printf("\ndisplay SUCCESS");
        return;
        
    }else if(strncasecmp(comm_args[0],"update",sizeof("update")-1)==0)
    {
        //Update command detected
        
        //Check if correct number of arguments are entered
        if(arguments!=3)
        {
            printf("\n update Correct usage:\nupdate <Server ID 1> <Server ID 2> <link cost>");
            return;
        }
        
        //Check if arguments are valid
        int ID1= atoi(comm_args[1]);
        int ID2= atoi(comm_args[2]);
        unsigned short cost;
        if(strcmp("inf",comm_args[3])==0){
            cost=infinity;
        }else
        {
            cost=(unsigned short) atoi(comm_args[3]);
        }
        
        unsigned short dest=0; 
        
        if(ID1==my_details.id){
            dest=(unsigned short)ID2;
        }else if(ID2==my_details.id){
            dest=(unsigned short)ID1;
        }else{
            printf("\n%s Command can only update values of host processes and neighbours",comm_line);
        }
        
        //Check if destination node is a neighbour 
        int isNeighbour=0;
        for(i=0;i<neighbours;i++)
            if(dest==neighbour_IDs[i])
            {
                isNeighbour=1;
            }
        
        if(isNeighbour!=1){
            printf("\n%s Command can only update values of host processes and neighbours",comm_line);
            return;
        }
        
        comm_update(dest,cost);
        
        
    }else if(strncasecmp(comm_args[0],"disable",sizeof("disable")-1)==0)
    {
        //Disable command detected
        
        //Check if correct number of arguments are entered
        if(arguments!=1){
            printf("\n %s Correct usage:\ndisplay <Server ID>",comm_line);
        }
        
        int dest_ID=atoi(comm_args[1]);
        
        //Check if argument is a neighbour
        int isNeighbour=0;
        for(i=0;i<neighbours;i++)
            if(dest_ID==neighbour_IDs[i])
            {
                isNeighbour=1;
            }
        
        if(isNeighbour!=1){
            printf("\n%s Command can only disable neighbours",comm_line);
            return;
        }
        
        disable(dest_ID);
        
        
    }else
    {
        printf("\n%s Invalid command",comm_line);
    }
}

/**
 * Send updates to all active neighbouring nodes
 * 
 * @param none
 * @return void
 * 
 */
 void comm_step(){
     
     int i;                                     //loop variable
     build_packet();                            //Build packet for sending to all neighbours
     
     for(i=0;i<neighbours;i++)
     {
        int neighbour_index=get_serv_index(neighbour_IDs[i]);
        
        if(node_status[neighbour_index]==active_neighbour){
            
            //Send Routing updates to all neighbours
            send_sock(neighbour_index,send_pack);
        }
     }
 }
 
 /**
  * Updates link cost 
  * 
  * @param ID of destination mode 
  * @param new value of cost 
  * 
  * @return void 
  * 
  */
 void comm_update(unsigned short destID, unsigned short cost){
     
    //Get destination server index 
    int dest_index=get_serv_index((int)destID);
     
    //Update cost matrix & next hop entries in routing Table
    rout_table[my_index][dest_index]=(int)cost;
    next_hop[dest_index]=(int) destID;
     
     
    int i=0;
            
    //Find position of sender in neighbours list 
    while(get_serv_index(neighbour_IDs[i])!=dest_index)
    {
        i++;
    }
    
    /* Update init vector */        
    init_vector[i]=(int)cost;
    
    /* Send update to neighbour */
    build_update_packet((int) destID, cost);
    //send_pack=update_pack;
    send_sock(dest_index,update_pack);
     
 }
 
 /**
  * Disables a given node 
  * 
  * @param ID of node to be disabled
  * @reutrn void
  */
  void disable(int ID){
      
     //Get destination server index 
    int dest_index=get_serv_index(ID);
    node_status[dest_index]=disabled;
    rout_table[my_index][dest_index]=infinity;
    
  }
