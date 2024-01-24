#include "csql.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const uint32_t ID_SIZE = attribute_size(Row, id);
const uint32_t USERNAME_SIZE = attribute_size(Row, username);
const uint32_t EMAIL_SIZE = attribute_size(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

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

void print_prompt() { printf("\e[1;31mdb \e[1;35m> \e[0m"); }

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

MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    // system("clear");
    printf("GoodBye... \n");
    db_close(table);
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

PrepareResult prepare_opcode(InputBuffer *input_buffer, Opcode *opcode) {
  if (strncmp(input_buffer->buffer, "insert", strlen("insert")) == 0) {
    return prepare_insert(input_buffer, opcode);
  }
  if (strcmp(input_buffer->buffer, "select") == 0) {
    opcode->type = OPCODE_SELECT;
    return PREPARE_SUCCESS;
  }
  return PREPARE_UNRECOGNIZED_COMMAND;
}

PrepareResult prepare_insert(InputBuffer *input_buffer, Opcode *opcode) {
  opcode->type = OPCODE_INSERT;

  char *keyword = strtok(input_buffer->buffer, " ");
  char *id_string = strtok(NULL, " ");
  char *username = strtok(NULL, " ");
  char *email = strtok(NULL, " ");
  _(keyword);

  if (id_string == NULL || username == NULL || email == NULL) {
    return PREPARE_SYNTAX_ERROR;
  }

  int id = atoi(id_string);
  if (id < 0) {
    return PREPARE_NEGATIVE_ID;
  }

  if (strlen(username) > COLUMN_USERNAME_BUF_LEN) {
    return PREPARE_STRING_TOO_LONG;
  }

  if (strlen(email) > COLUMN_EMAIL_BUF_LEN) {
    return PREPARE_STRING_TOO_LONG;
  }

  opcode->insert_buffer.id = id;
  strcpy(opcode->insert_buffer.username, username);
  strcpy(opcode->insert_buffer.email, email);

  return PREPARE_SUCCESS;
}

ExecuteResult execute_opcode(Opcode *opcode, Table *table) {
  switch (opcode->type) {
  case (OPCODE_INSERT):
    return execute_insert(opcode, table);
  case (OPCODE_SELECT):
    return execute_select(table);
  }
}

ExecuteResult execute_insert(Opcode *opcode, Table *table) {
  if (table->num_rows >= TABLE_MAX_ROWS) {
    return EXECUTE_TABLE_FULL;
  }

  Row *row_buf = &(opcode->insert_buffer);

  serialize_row(row_buf, row_slot(table, table->num_rows));
  table->num_rows += 1;

  return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Table *table) {
  Row row;
  for (uint32_t i = 0; i < table->num_rows; ++i) {
    deserialize_row(row_slot(table, i), &row);
    print_row(&row);
  }
  return EXECUTE_SUCCESS;
}

void serialize_row(Row *source, void *destination) {
  memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
  strncpy(destination + USERNAME_OFFSET, source->username, USERNAME_SIZE);
  strncpy(destination + EMAIL_OFFSET, source->email, EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination) {
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void *row_slot(Table *table, uint32_t nrows) {
  uint32_t page_num = nrows / ROWS_PER_PAGE;
  void *page = get_page(table->pager, page_num);
  uint32_t row_offset = nrows % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;
  return page + byte_offset;
}

void print_row(Row *row) {
  printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

Table *db_connect(const char *filename) {
  Pager *pager = pager_open(filename);
  uint32_t numrows = pager->file_length / ROW_SIZE;
  Table *table = (Table *)malloc(sizeof(Table));

  table->pager = pager;
  table->num_rows = numrows;
  return table;
}

void db_close(Table *table) {
  Pager *pager = table->pager;
  uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

  for (uint32_t i = 0; i < num_full_pages; ++i) {
    if (pager->pages[i] == NULL) {
      continue;
    }
    pager_flush(pager, i, PAGE_SIZE);
    free(pager->pages[i] = NULL);
  }

  uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
  if (num_additional_rows > 0) {
    uint32_t page_num = num_full_pages;
    if (pager->pages[page_num] != NULL) {
      pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
      free(pager->pages[page_num]);
      pager->pages[page_num] = NULL;
    }
  }

  int result = close(pager->fd);
  if (result == -1) {
    printf("ERROR: failed to close db. \n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < TABLE_MAX_PAGES; ++i) {
    void *page = pager->pages[i];
    if (page) {
      free(page);
      pager->pages[i] = NULL;
    }
  }
  free(pager);
  free(table);
}

Pager *pager_open(const char *filename) {
  int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

  if (fd == -1) {
    printf("ERROR: unable to open file. \n");
    exit(EXIT_FAILURE);
  }

  off_t file_length = lseek(fd, 0, SEEK_END);

  Pager *pager = malloc(sizeof(Pager));
  pager->fd = fd;
  pager->file_length = file_length;

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i) {
    pager->pages[i] = NULL;
  }
  return pager;
}

void pager_flush(Pager *pager, uint32_t num_page, uint32_t size) {
  if (pager->pages[num_page] == NULL) {
    printf("ERROR: tried to flush null page. \n");
    exit(EXIT_FAILURE);
  }

  off_t offset = lseek(pager->fd, num_page * PAGE_SIZE, SEEK_SET);

  if (offset == -1) {
    printf("ERROR: failed to seek: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  ssize_t bytes_written = write(pager->fd, pager->pages[num_page], size);

  if (bytes_written == -1) {
    printf("ERROR: failed to write to db: %d. \n", errno);
    exit(EXIT_FAILURE);
  }
}

void *get_page(Pager *pager, uint32_t num_page) {
  if (num_page > TABLE_MAX_PAGES) {
    printf("ERROR: tried to fetch page out of bounds. %d > %d\n. ", num_page,
           TABLE_MAX_PAGES);
    exit(EXIT_FAILURE);
  }

  if (pager->pages[num_page] == NULL) {
    void *page = malloc(PAGE_SIZE);
    uint32_t num_pages = pager->file_length / PAGE_SIZE;

    if (pager->file_length % PAGE_SIZE) {
      num_pages += 1;
    }

    if (num_page <= num_pages) {
      lseek(pager->fd, num_page * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = read(pager->fd, page, PAGE_SIZE);
      if (bytes_read == -1) {
        printf("ERROR: failed to read file: %d\n", errno);
        exit(EXIT_FAILURE);
      }
    }

    pager->pages[num_page] = page;
  }

  return pager->pages[num_page];
}
