#include "csql.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  InputBuffer *buffer = new_input_buffer();
  while (true) {
    print_prompt();
    read_input(buffer);

    if (buffer->buffer[0] == '.') {
      switch (do_meta_command(buffer)) {
      case (META_COMMAND_SUCCESS):
        continue;
      case (META_COMMAND_UNRECOGNIZED_COMMAND):
        printf("ERROR: unrecognized command '%s'.\n", buffer->buffer);
        continue;
      }
    }

    Opcode op;
    switch (prepare_opcode(buffer, &op)) {
    case (PREPARE_SUCCESS):
      break;
    case (PREPARE_UNRECOGNIZED_COMMAND):
      printf("ERROR: unrecognized keyword at start of '%s'.\n", buffer->buffer);
      continue;
    }

    execute_opcode(&op);
    printf("INFO: executed successfully. \n");
  }
}
