#include <stdlib.h>

#define MAX_DATA_SIZE 512
// packet opcodes.
#define OP_RRQ          1 // Unsupported in out implementation
#define OP_WRQ          2
#define OP_DATA         3
#define OP_ACK          4
#define OP_ERROR        5


/* In this file we define the varius message types.
 * The message types are written in "PURE C",
 * Because we want absoule control over the structs size.
 * (No objects)
*/

struct WRQ {
    // WRQ opcode.
    char opcode[2];
    // Pointer to file name. we'll allocate it dynamiclly.
    char *file_name;
    // Pointer to transmission mode. we'll allocate it dynamiclly.
    char *trans_mode;
} __attribute__((packed));

struct ACK {
    // ACK opcode.
    char opcode[2];
    // Block number indicator.
    char block_number[2];
} __attribute__((packed));

struct DATA {
    // Data packet header. identical to ACK packet.
    char opcode[2];
    // Block number indicator.
    char block_number[2];
    // DATA Buffer.
    // Maximum size is defined at 512 bytes.
    char data_buffer[MAX_DATA_SIZE];
} __attribute__((packed));