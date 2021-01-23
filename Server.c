#include "Server.h"
#define BUF_SIZE 516 // Maximum message size
#define TIME_OUT 3
#define MAX_ERRORS 7


const int WAIT_FOR_PACKET_TIMEOUT = 3;
const int NUMBER_OF_FAILURES = 7;

int main(int argc, char *argv[]) 
{
    if(argc < 2) {
        printf("Invalid usage..Please specift port number\n");
        exit(1);
    }

    int sockfd, portno;
    struct sockaddr_in server_addr, cli_addr;
    

    // set up udp socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("TTFTP_ERROR: Error creating UDP socket\n");
        exit(1);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    bzero((char *)&cli_addr, sizeof(cli_addr));
    portno = atoi(argv[1]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);
    char recv_buf[BUF_SIZE];

    // bind socket to the address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("TTFTP_ERROR: Error binding scoket to address\n");
        exit(1);
    }

    unsigned int client_len = sizeof(cli_addr);
    // At this point we are ready for communication.

    // Maximum timegap that no packet can be recieved
    struct timeval timeout;

    // File descriptor set. only Server FD interest's us.
    fd_set rfds;
    

    // Main server loop
    for (; ;)
    {

        printf("Waiting on port %d \n", htons(server_addr.sin_port));
        
        int is_active = 1;
        // The file we recieved
        FILE *in_file = NULL;
        // here we will store the initial write request.
        struct WRQ *wrq_packet;
        struct DATA *data_packet;
        char *ack;
        int msg_recvlen, writelen;
        
        uint32_t block_number = 0;
        int fault_counter = 0;
        int success = 0;

        while(is_active) 
        {
            timeout.tv_sec = 3;
            timeout.tv_usec = 0;
            FD_ZERO(&rfds);
            FD_SET(sockfd, &rfds);
            int ret = select(sockfd+1, &rfds, NULL, NULL, &timeout);
            if (ret == -1) {
                // error 
                printf("Error");    
                break;
                
            }
            else if(ret == 0) {
                // timeout
                
                if (in_file != NULL) {
                    ++fault_counter;
                    
                    ack = make_ACK_packet(block_number);
                    writelen = 0;
                    while (writelen <= 0)
                    {
                        writelen = sendto(sockfd, (const void *)ack, sizeof(struct ACK), 0, (struct sockaddr *)&cli_addr, client_len);
                        printf("OUT:ACK,%d\n", block_number);
                    }
                    free(ack);


                    printf("FLOWERROR: Didn't recieved any message in 3 seconds\n");
                    if (fault_counter >= NUMBER_OF_FAILURES) {
                        printf("FLOWERROR: Failured count exceeded the session limit (7)..Bailling out.\n");
                        fclose(in_file);
                        in_file = NULL;
                        break;
                    }
                }
                
                    
            }
            else {
                // recieve the opcode
                msg_recvlen = recvfrom(sockfd, (char *)recv_buf, BUF_SIZE, 0, (struct sockaddr *)&cli_addr, (unsigned int *)&client_len);
                if (msg_recvlen <= 0) {
                    // network error
                    printf("error, %s\n", strerror(errno));
                    printf("debug::net error\n");
                }

            }
            if(msg_recvlen > 0) 
            {
                int parsed_opcode = recv_buf[1];
                switch (parsed_opcode)
                {
                case OP_WRQ:
                    // TODO: validate first and only wrq of the session.

                    wrq_packet = parse_WRQ_packet(recv_buf);
                    printf("IN:WRQ,%s,%s\n", wrq_packet->file_name, wrq_packet->trans_mode);
                    in_file = fopen(wrq_packet->file_name, "w");
                    if (!in_file) {
                        perror("TTFTP_ERROR: Cannot open/create new file\n");
                    }
                    free(wrq_packet);

                    ack = make_ACK_packet(block_number);
                    writelen = 0;
                    while (writelen <= 0)
                    {
                        writelen = sendto(sockfd, (const void *)ack, sizeof(struct ACK), 0, (struct sockaddr *)&cli_addr, client_len);
                        printf("OUT:ACK,%d\n", block_number);
                    }
                    
                    free(ack);
                    block_number++;
                    break;
                case OP_ACK:

                    break;

                case OP_DATA:
                    if(!in_file) {
                        // error. cant get data before write request
                        printf("FLOWERROR: cant get data before write request\n");
                        exit(1);

                    }
                    // TODO: validate first and only wrq of the session.
                                        
                    //recvlen = recvfrom(sockfd, (char *)recv_buf, BUF_SIZE - 2, MSG_WAITALL,(struct sockaddr *)&cli_addr, &client_len);
                    if(msg_recvlen < BUF_SIZE) {
                        // last data packet recieved, send ack and end session.
                        success = 1;
                        is_active = 0;
                    }
                    data_packet = parse_DATA_packet(recv_buf, msg_recvlen - 4);
                    printf("IN:DATA, %d,%d\n", block_number, msg_recvlen);
                    int recv_block_num = ((int)data_packet->block_number[1] & 0xff) | (((int)data_packet->block_number[0] & 0xff) << 8);
                    if (recv_block_num != block_number) {
                        printf("FLOWERROR: The recieved block number is invalid..Bailling out.\n");
            
                        success = 0;
                        break;
                    }
                    printf("WRITING: %d\n", msg_recvlen - 4);
                    int written = fwrite(data_packet->data_buffer, sizeof(char), msg_recvlen - 4, in_file);
                    if (written != msg_recvlen - 4) {
                        perror("TTFTP_ERROR: Unable to write the requested amount of bytes to file\n");
                    }
                    
                    free(data_packet);

                    ack = make_ACK_packet(block_number);
                    writelen = 0;
                    while (writelen <= 0)
                    {
                        writelen = sendto(sockfd, (const void *)ack, sizeof(struct ACK), 0, (struct sockaddr *)&cli_addr, client_len);
                        printf("OUT:ACK,%d\n", block_number);
                    }
                    free(ack);

                    block_number++;

                    break;
                default:
                    writelen = 0;
                    break;
                }
                writelen = 0;
            }
            msg_recvlen = 0;
            writelen++;
        }    
        if (success) {
            printf("RECVOK\n");
        }
        else {
            printf("RECVFAIL\n");
        }
        if(in_file)
            fclose(in_file);
    }
    return 0;
}



