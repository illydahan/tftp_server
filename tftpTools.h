#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

#include <netinet/in.h>
#include <netdb.h>
 #include <arpa/inet.h>
 
 #include "MessageTypes.h"
/* 
 * Message parser functions
 * Each function will read from input buffer, as recived from UDP connection,
 * And will allocate the appropiate structers.
 * 
 * @INPUT:  input_msg - Input buffer from UDP.
 * @OUTPUT: pointer to pasred struct, as defined at "MessageTypes.h"
*/

struct WRQ *parse_WRQ_packet(char *input_msg);

struct ACK *parse_ACK_packet(char *input_msg);

struct DATA *parse_DATA_packet(char *input_msg, uint32_t data_size);

/*
 * Packet creation functions.
 * Will create acknowledge packet.
*/

char *make_ACK_packet(uint32_t block_number);


/* Description: Since strings need to be dynamiclly allocated, 
 *              here we will read a string and return the string size.
 * Input: string_legnth - the inferred string length, from the wrq packet.
 *        in_buffer - WRQ packet bytes
 * Ouput: the new name buffer.
*/ 
char * get_string_size(char *in_buffer, int *string_length);