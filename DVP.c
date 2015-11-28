/**
 * Implementing the Distance Vector Routing Protocol on a 
 * network. Each node of the network simulates a router. It
 * constructs the network topology from a local file. All 
 * nodes share updates with their neighbours using UDP. User provides
 * location of topology file and routing update interval. 
 * 
 * @author Alizishaan Khatri
 * 
 * Instructions for use: server -t <topology-file-name> -i <routing-update-interval>
 * 
 */

#include"DVP.h"

/* Function declarations : */

/* Define extern variables */
int server_no;                      //Store number of servers
int neighbours;                     //Store no of neighbours of given node
struct server_info serv_list[5];    //Store server details
struct server_info my_details;      //Store details of the host process
int neighbour_IDs[4];               //Store IDs of all the neighbouring nodes
int init_vector[4];                 //Store the initial weights
int my_fd;                          //Store FD for local socket
char msg_packet[100];               //Store packet that will be shared between hosts
int rec_packets=0;                  //Keep count of the number of packets received
int my_index=0;                     //Store index of host process
packet send_pack;                   //Stores the packet to be sent
packet update_pack;                 //Stoes the update packet to be sent
char node_status[5];                //Store neighbouring node status
int inactiveCount[5];               //Store no of cycles for which a particular neighbour has been inactive

/* Declare global (only to this source file) variables: */
float timeout;                      //Stores the value of update time interval
char *fil_path;  	                //Stores the path of the topology file
fd_set master,readfds;              //Stores list of active file descriptors

/**
 * Defining the routing table: 
 * The value of each element represents the distance between the nodes represented by the outer index
 * and inner index.
 */
int rout_table[5][5];
int recv_table[5][5];               //Stores copy of routing table received from neighbour
int next_hop[5];                    //Stores the hop matrix

/**
 * Function that puts everything together
 * 
 * Entry point into the program
 * @returns Void
 * 
 */
int main(int argc, char *argv[])
{
    
	int i,j; 			//Loop variables
    
    /* Check if the correct number of arguments have been input from command line */
    if(argc!=5)
    {
        exit_fn("\n Correct Usage: server -t <topology-file-name> -i <routing-update-interval> \n");
    }
    
    /* Extract file path and update interval values from the command line arguments */
    if(strcmp(argv[1],"-t")==0 && strcmp(argv[3],"-i")==0)
    {
        fil_path=argv[2];
        timeout=atof(argv[4]);
    }
    else if(strcmp(argv[3],"-t")==0 && strcmp(argv[1],"-i")==0)
    {
        fil_path=argv[4];
        timeout=atof(argv[2]);
    }
    else
    {
        exit_fn("\nCorrect Usage: server -t <topology-file-name> -i <routing-update-interval> \n");
    }
    
    /* Initialize the routing table */
    init_table();
    
    /*for(i=0;i<server_no;i++)
        printf("\t%d",next_hop[i]);
    */
    
    /* Initialize values */
    memset((char *)&node_status,no_direct_link,5);      //All neighbours are enabled
    memset((int *)&inactiveCount,0,5);                  //All counters are set to zero
    
    /* Populate the routing table from topology file */
    file_process();
    
    
    /* DEBUG: Print hop list */
    //print_hop();
    
    //printf("\n\nMy Public IP is : %s and FD is : %d\n\n",my_details.ip,my_fd);
    
    //build_packet();
    
    /* Read the topology file and construct routing table */
    calc_DV();
    //print_values();
    
    /*Print hop list */
    //print_hop();
    
    /* Set-up listening socket */
    my_fd=sock_init();
    my_fd=fd;
    
    /**
     * Set up select()
     */
    struct timeval time_out;
    time_out.tv_sec=(int) timeout;
    time_out.tv_usec=(int)((timeout-(int)timeout)*1000000);
    
    //Initialize values of master and reading fdlists
    memset((fd_set *)&master,0,sizeof(master));
    memset((fd_set *)&readfds,0,sizeof(readfds));
    
    //Add standard input to list of master FDs 
    FD_SET(0,&master);
    //Add listener socket to master FD list
    FD_SET(my_fd,&master);
    
    //Send values to all neighbours on startup
    void comm_step();
    
    //infinite loop
    for(;;)
    {
        //flush the stdout stream
        fflush(stdout);
        
         /* Refresh timeout value in time_val struct */
        time_out.tv_sec=(int)timeout;
        time_out.tv_usec=(int)((timeout-(int)timeout)*1000000);
        
        int rv;
        
        readfds=master;                                         //copy read 
        rv = select(my_fd+1, &readfds, NULL, NULL, &time_out);
        
        if (rv == -1)
        {
            exit_fn("select");                                  // error occurred in select()
        }else if(rv != 0)
        {// one or both of the descriptors have data
           
            if (FD_ISSET(0, &readfds))
            {   //Data read on standard input
                
                char input[999];
                
                //Read data from standard input
                fgets(input,999,stdin);
                input[strlen(input)+1]='\0';
                
                comm_process(input);
                printf("\n>>>");
                fflush(stdout);
                
            }
            if(FD_ISSET(my_fd, &readfds))
            {   //There is an incoming packet on listener
                
                //Increment packet Count
                rec_packets++;
                
                //Read packet 
                sock_read();
            }
        }else
        {  
            //Send routing updates to all neighbours
            void comm_step();
            
            //printf("\nTimeout!!");
            
            //Update inactivity counter and disable nodes that haven't sent a packet in three update cycles
            for(i=0;i<server_no;i++)
            {
                //Increment counter for active neighbours
                if(node_status[i]==active_neighbour)
                {
                    inactiveCount[i]++;
                }
                
                //Disable neighbours that haven't sent a packet in over 3 update intervals
                if(inactiveCount[i]==4){
                    node_status[i]=disabled;
                    rout_table[my_index][i]=infinity;
                }
            }
            //Compute routing table 
            calc_DV();
           
        }
    }
}

/**
 * Function to handle unsuccessful exit
 * Parameters  : *msg Enter the error message here
 * Returns     : void
 */
void exit_fn(char *msg)
{
	perror(msg);
	exit(1);
}

/**
 * Function to construct routing table from topology file
 * Parameters  : none
 * Returns     : void
 */
void file_process()
{
    FILE *top_file;         //Pointer to the open file 
    char buffer[1024];      //To store value of string read from file 
	int i,j;                //loop variables
	
	
	/* Open topology file for reading */
    top_file=fopen(fil_path,"r");
    
    /* Generate error response if file or update value is invalid */
    if(top_file==NULL)
    {
        exit_fn("\nPlease enter the correct file path");
    }
    else if(timeout<=0)
    {
        exit_fn("\nUpdate time must be a positive quantity");
    }
	
	//Read total no of servers
	fscanf(top_file,"%d",&server_no);
	//Check if no of servers is valid
	if(server_no>5 || server_no==0)
	{
	    exit_fn("\nInvalid number of servers in topology file");
	}
	
	//Read no of neighbours
	fscanf(top_file,"%d",&neighbours);
	
	//Check if no of neighbours is valid
	if(neighbours>=server_no || neighbours<0)
	{
	    exit_fn("\nInvalid number of neighbours in topology file");
	}
	
	//Populate server details
	for(i=0;i<server_no;i++)
	{
	    char temp_ip[100];      //Field to store IP Address String read from file
	    int id,port;        //Temporary fields to store Server ID and port read from file
	    
	    //Read values from file
	    fscanf(top_file,"%d %s %d",&id,temp_ip,&port);
	    
	    //Add values to structure
	    serv_list[i].id=id;
	    
	    serv_list[i].ip= malloc(sizeof(char) * strlen(temp_ip));
	    strcpy(serv_list[i].ip,temp_ip);
	    
	    serv_list[i].port=port;
	    
	 }
	 
	 //Sort serv_list in ascending order of Server IDs
	 sort_servers();
	
	 //Get details of host server
	 fill_mydet();

	/** 
     * Initialize the hop values:
     * The inital next hop to each node is set to 
     * the host process  itself 
     */
    for(i=0;i<server_no;i++)
        next_hop[i]=my_details.id;
	 
	 //Update routing routing table as per values from topology file
	for(i=0;i<neighbours;i++)
	{
	   int id1,id2,int_cost;
	   char cost[10]; 
	   //char temp[10];
	   
	   //Read values from file
	    fscanf(top_file,"%d %d %s",&id1,&id2,cost);
	    
	    //Check if ID values in topology file are correct
	    if(id1<=0||id2<=0)
	    {
	        exit_fn("\nPlease enter valid ID values in topology file");
	    }
	    
	    //Check if topology file only contains its own link values
	    if(id1!=my_details.id && id2!=my_details.id)
	    {
	        exit_fn("\nTopology file must only contain the costs of links to neighbours");
	    }
	   
	    
	    //Get ineger value of link cost
	    int_cost=atoi(cost);
	    
	    //Check if link cost is positive
	    if(int_cost<0)
	    {
	        exit_fn("\nCost must be positive quantity");
	    }
	    
	    //Update get index positions for given IDs
	    int ind_id1,ind_id2;
	    ind_id1=get_serv_index(id1);
	    ind_id2=get_serv_index(id2);
	    
	    //Handle 'infinity' link costs
	    if(strcmp("inf",cost)!=0)
	    {
	        rout_table[ind_id1][ind_id2]=int_cost;
	        rout_table[ind_id2][ind_id1]=int_cost;
	    }
	    else
	    {
	        rout_table[ind_id1][ind_id2]=infinity;
	        rout_table[ind_id2][ind_id1]=infinity;   
	    }
	    
	    //printf("\nrout_table(%d , %d) is set to: %s",id1,id2,cost);
	    ///printf("\trout_table(%d , %d) is set to: %s",id2,id1,cost);
	    
	    //Store IDs of all the neighbours, mark them and update next hop values 
	    if(id2!=my_details.id){
	        neighbour_IDs[i]=id2;
	        node_status[ind_id2]=active_neighbour;
	        
	        if( rout_table[ind_id1][ind_id2]!=infinity)
	            next_hop[ind_id2]=id2;
	        
	    }else{
	        neighbour_IDs[i]=id1;
	        next_hop[ind_id1]=id1;
	        node_status[ind_id1]=active_neighbour;
	        
	        if( rout_table[ind_id1][ind_id2]!=infinity)
	            next_hop[ind_id1]=id1;
	    }
	        
	        
	    //Store initial costs of links to neighbours
	    init_vector[i]=int_cost;
	    
	    //Update list of hop values 
	    //next_hop[get_serv_index(neighbour_IDs[i])]=neighbour_IDs[i];
	    
	    //Update host process's index values 
	    my_index=get_serv_index(my_details.id);
	    
	    //printf("\nThe neighbour at position %d is %d",i,neighbour_IDs[i]);
	    
	}
	
	//close topology file
	fclose(top_file);
}

/**
 * Sort servers in list in ascending order of their neighbour_IDs
 * @params none
 * @return none
 */
void sort_servers()
{
    unsigned int i,j;                   //loop variables
    
    //Implement Bubble Sort Algorithm to set servers in sequence
    for(i=0;i<(server_no-1);i++)
        for(j=0;j<(server_no-1);j++)
            if(serv_list[j].id>serv_list[j+1].id){
                
                //Define variables to hold values for swapping
                unsigned int temp_port, temp_id;
                char *temp_ip;
                temp_ip=malloc(sizeof(char) * strlen(serv_list[j].ip));
                
                //Make copies of data for swapping
                temp_port=serv_list[j].port;
                temp_id=serv_list[j].id;
                strcpy(temp_ip,serv_list[j].ip);
                
                //Assign values of (j+1)th element to jth element
                serv_list[j].id=serv_list[j+1].id;
                serv_list[j].port=serv_list[j+1].port;
                strcpy(serv_list[j].ip,serv_list[j+1].ip);
                
                //Assign original value of jth element to (j+1)th element
                serv_list[j+1].id=temp_id;
                serv_list[j+1].port=temp_port;
                strcpy(serv_list[j+1].ip,temp_ip);
            }
}

/**
 * Get index (position) of current server in serv_list
 * @param ID: The ID of server whose position has to be looked up
 * @return The position of given server in serv_list
 */
int get_serv_index(int id)
{
    int ii=0;                       //Stores count
   
    //Count till server ID is found or end of array is reached
    while(serv_list[ii].id!=id )
    {
        ii++;
        if(ii==server_no)
        {
             exit_fn("Please check ID values");
        }
    }
    return(ii);
}

/**
 * Initialize all values in routing table: * 
 * a) The link cost of each node to itself is zero
 * b) All other link costs are set to infinity
 * 
 * @param none
 * @return void
 */
void init_table()
{
    int i,j;                //Loop variables
    for(i=0;i<5;i++)
		for(j=0;j<5;j++)
		    if(i!=j)
		    {
			    rout_table[i][j]=infinity;
			    //next_hop[i][j]=infinity;
		    }else
		    {
		        rout_table[i][j]=0;
		        //next_hop[i][j]=0;
		    }
}
