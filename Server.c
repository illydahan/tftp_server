#include "Server.h"
#define BUF_SIZE 516 // Maximum message size



const int WAIT_FOR_PACKET_TIMEOUT = 3;
const int NUMBER_OF_FAILURES = 7;

int main(int argc, char *argv[]) 
{
    if(argc < 2) {
        fprintf(stderr, "Invalid usage..Please specift port number\n");
        exit(1);
    }

    int sockfd, portno;
    struct sockaddr_in server_addr, cli_addr;
    

    // set up udp socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR opening socket\n");
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
        fprintf(stderr, "Error binding socket to server address.(Port in use ?)\n");
        exit(1);
    }

    unsigned int client_len = sizeof(cli_addr);
    // At this point we are ready for communication.

    

    // Main server loop
    for (; ;)
    {

        printf("Waiting on port %d \n", htons(server_addr.sin_port));
        uint32_t block_number = 0;
        int is_active = 0;
        // The file we recieved
        FILE *in_file = NULL;

        // here we will store the initial write request.
        struct WRQ *wrq_packet;
        struct DATA *data_packet;
        char *ack;
        uint32_t recvlen, writelen;
        unsigned char op_code[2];
        recvlen = recvfrom(sockfd, &op_code, 2, MSG_WAITALL,(struct sockaddr *)&cli_addr, (unsigned int *)&client_len);
        
        is_active = recvlen > 0 ? 1 : 0;
        
        while(is_active) 
        {
            recvlen = recvfrom(sockfd, &op_code, 2, MSG_WAITALL,(struct sockaddr *)&cli_addr, (unsigned int *)&client_len);
            if(recvlen > 0) 
            {
                int parsed_opcode = op_code[1];
                switch (parsed_opcode)
                {
                case OP_WRQ:
                    printf("got WRQ..\n");
                    // TODO: validate first and only wrq of the session.
                    recvlen = recvfrom(sockfd, (char *)recv_buf, BUF_SIZE - 2, MSG_WAITALL,(struct sockaddr *)&cli_addr, (unsigned int *)&client_len);

                    wrq_packet = parse_WRQ_packet(recv_buf);
                    in_file = fopen(wrq_packet->file_name, "w");
                    printf("file name %s\n", wrq_packet->file_name);
                    free(wrq_packet);

                    ack = make_ACK_packet(block_number);

                    writelen = sendto(sockfd, (const void *)ack, sizeof(struct ACK), 0, (struct sockaddr *)&cli_addr, client_len);

                    free(ack);
                    block_number++;
                    break;
                case OP_ACK:

                    break;

                case OP_DATA:
                    if(in_file == NULL) {
                        // error. cant get data before write request
                        printf("error. cant get data before write request\n");
                        fclose(in_file);
                        exit(1);

                    }
                    printf("got data request\n");
                    recvlen = recvfrom(sockfd, (char *)recv_buf, BUF_SIZE - 2, MSG_WAITALL,(struct sockaddr *)&cli_addr, &client_len);
                    if(recvlen < BUF_SIZE - 2) {
                        // last data packet recieved, send ack and end session.
                        is_active = 0;
                    }
                    data_packet = parse_DATA_packet(recv_buf, recvlen - 4);
                    fwrite(data_packet->data_buffer, sizeof(char), recvlen - 4, in_file);
                    free(data_packet);

                    ack = make_ACK_packet(block_number);
                    writelen = sendto(sockfd, (const void *)ack, sizeof(struct ACK), 0, (struct sockaddr *)&cli_addr, client_len);
                    free(ack);

                    block_number++;

                    break;
                default:
                    writelen = 0;
                    break;
                }
                writelen = 0;
            }
            writelen++;
        }    
        fclose(in_file);
    }
    return 0;
}



