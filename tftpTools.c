#include "tftpTools.h"

struct WRQ *parse_WRQ_packet(char *input_msg)
{
    struct WRQ *wrq_packet = (struct WRQ *)malloc(sizeof(struct WRQ));
    bzero(wrq_packet, sizeof(struct WRQ));
    // will keep truck of the current input_msg location.
    unsigned long index = 2;
    memset(wrq_packet->opcode, OP_WRQ, 2);
   
    int string_leangth = 0;
    wrq_packet->file_name = get_string_size(input_msg+index, &string_leangth);
    
    index += string_leangth;

    wrq_packet->trans_mode = get_string_size(input_msg+index, &string_leangth);
    index += string_leangth;

    return wrq_packet;
}

struct ACK *parse_ACK_packet(char *input_msg)
{
    struct ACK *ack_packet = (struct ACK *)malloc(sizeof(struct ACK));
    bzero(ack_packet, sizeof(struct ACK));
    // will keep truck of the current input_msg location.
    int index = 2;
    memset((char *)ack_packet->opcode, OP_ACK, 2);

    memcpy((char *)&ack_packet->block_number, (char *)input_msg+index, 2);
    index += 2;

    return ack_packet;
}

struct DATA *parse_DATA_packet(char *input_msg, uint32_t data_size)
{
    struct DATA *data_packet = (struct DATA *)malloc(sizeof(struct DATA));
    bzero(data_packet, sizeof(struct DATA));
    
    int index = 2;
    memset((char *)data_packet->opcode, OP_DATA, 2);


    memcpy((char *)&data_packet->block_number, (char *)input_msg+index, 2);
    index += 2;

    memcpy((char *)data_packet->data_buffer, (char *)input_msg+index, data_size);
    index += data_size;

    return data_packet;
}


char *make_ACK_packet(uint32_t block_number) 
{
    struct ACK *ack_packet = (struct ACK *)malloc(sizeof(struct ACK));
    bzero(ack_packet, sizeof(struct ACK));
    
    memset(&ack_packet->opcode[1], OP_ACK, 1);

    memset(&ack_packet->block_number[1], block_number & 0xff, sizeof(char));
    memset(&ack_packet->block_number[0], (block_number >> 8) & 0xff, sizeof(char));

    char *buffer = (char *)malloc(sizeof(struct ACK));
    memcpy((char *)buffer, (char *)ack_packet, sizeof(struct ACK));
    free(ack_packet);
    return buffer;
    
}

char * get_string_size(char *in_buffer, int *string_length)
{
    char current_char;
    *string_length = 0;
    do
    {
        current_char = *(in_buffer + *string_length);
        (*string_length)++;
    } while (current_char != '\0');
    // allocate memory for output buffer
    char *out_buffer = (char *)malloc(sizeof(char) * (*string_length));
    // re-wind the in_buffer
    
    memcpy(out_buffer, in_buffer , *string_length);
    printf("file name %s\n ", out_buffer);
    return out_buffer;
}