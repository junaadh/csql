#include "csql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputBuffer *new_input_buffer() {
  InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;
  return input_buffer;
}

void close_input_buffer(InputBuffer *input_buffer) {
  free(input_buffer->buffer);
  free(input_buffer);
}

void print_prompt() { printf("db > "); }

void read_input(InputBuffer *input_buffer) {
  ssize_t bytes_read =
      getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
  if (bytes_read <= 0) {
    fprintf(stderr, "ERROR: failed to read input\n");
    exit(EXIT_FAILURE);
  }

  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

PrepareResult prepare_opcode(InputBuffer *input_buffer, Opcode *opcode) {
  if (strncmp(input_buffer->buffer, "insert", strlen("insert")) == 0) {
    opcode->type = OPCODE_INSERT;
    return PREPARE_SUCCESS;
  }
  if (strcmp(input_buffer->buffer, "select") == 0) {
    opcode->type = OPCODE_SELECT;
    return PREPARE_SUCCESS;
  }
  return PREPARE_UNRECOGNIZED_COMMAND;
}

void execute_opcode(Opcode *opcode) {
  switch (opcode->type) {
  case (OPCODE_INSERT):
    printf("This is where we would do an insert. \n");
    break;
  case (OPCODE_SELECT):
    printf("This is where we would do a select. \n");
    break;
  }
}
