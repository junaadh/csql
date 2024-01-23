#include "csql.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  InputBuffer *buffer = new_input_buffer();
  Table *table = new_table();
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
    case (PREPARE_SYNTAX_ERROR):
      printf("ERROR: syntax error: couldnt parse statement. \n");
      continue;
    case (PREPARE_UNRECOGNIZED_COMMAND):
      printf("ERROR: unrecognized keyword at start of '%s'.\n", buffer->buffer);
      continue;
    }

    switch (execute_opcode(&op, table)) {
    case (EXECUTE_SUCCESS):
      printf("INFO: executed successfully. \n");
      break;
    case (EXECUTE_TABLE_FULL):
      printf("ERROR: table full. \n");
      break;
    }
  }
}
