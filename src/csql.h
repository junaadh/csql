#ifndef CSQL_H
#define CSQL_H

#include <stddef.h>
#include <sys/types.h>

typedef struct {
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum { PREPARE_SUCCESS, PREPARE_UNRECOGNIZED_COMMAND } PrepareResult;

typedef enum { OPCODE_INSERT, OPCODE_SELECT } OpcodeType;

typedef struct {
  OpcodeType type;
} Opcode;

InputBuffer *new_input_buffer();
void close_input_buffer(InputBuffer *input_buffer);
void print_prompt();
void read_input(InputBuffer *input_buffer);
MetaCommandResult do_meta_command(InputBuffer *input_buffer);
PrepareResult prepare_opcode(InputBuffer *input_buffer, Opcode *opcode);
void execute_opcode(Opcode *opcode);

#endif
