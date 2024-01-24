#ifndef CSQL_H
#define CSQL_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define COLUMN_USERNAME_BUF_LEN 32
#define COLUMN_EMAIL_BUF_LEN 255
#define TABLE_MAX_PAGES 100
#define DEFAULT_DB_PATH "mydb.cdb"
#define _(x) (void)(x)
#define attribute_size(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_COMMAND,
  PREPARE_SYNTAX_ERROR,
  PREPARE_STRING_TOO_LONG,
  PREPARE_NEGATIVE_ID
} PrepareResult;

typedef enum {
  EXECUTE_SUCCESS,
  EXECUTE_TABLE_FULL,
} ExecuteResult;

typedef enum { OPCODE_INSERT, OPCODE_SELECT } OpcodeType;

typedef struct {
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_BUF_LEN + 1];
  char email[COLUMN_EMAIL_BUF_LEN + 1];
} Row;

extern const uint32_t ID_SIZE;
extern const uint32_t USERNAME_SIZE;
extern const uint32_t EMAIL_SIZE;
extern const uint32_t ID_OFFSET;
extern const uint32_t USERNAME_OFFSET;
extern const uint32_t EMAIL_OFFSET;
extern const uint32_t ROW_SIZE;
extern const uint32_t PAGE_SIZE;
extern const uint32_t ROWS_PER_PAGE;
extern const uint32_t TABLE_MAX_ROWS;

typedef struct {
  int fd;
  uint32_t file_length;
  void *pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
  uint32_t num_rows;
  Pager *pager;
} Table;

typedef struct {
  OpcodeType type;
  Row insert_buffer; // to be use by select_opcode
} Opcode;

InputBuffer *new_input_buffer();
void close_input_buffer(InputBuffer *input_buffer);
void print_prompt();
void read_input(InputBuffer *input_buffer);
MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table);
PrepareResult prepare_opcode(InputBuffer *input_buffer, Opcode *opcode);
PrepareResult prepare_insert(InputBuffer *input_buffer, Opcode *opcode);
ExecuteResult execute_opcode(Opcode *opcode, Table *table);
ExecuteResult execute_insert(Opcode *opcode, Table *table);
ExecuteResult execute_select(Table *table);
void serialize_row(Row *source, void *destination);
void deserialize_row(void *source, Row *destination);
void *row_slot(Table *table, uint32_t nrows);
void print_row(Row *row);
Table *db_connect(const char *filename);
void db_close(Table *table);
Pager *pager_open(const char *filename);
void pager_flush(Pager *pager, uint32_t num_page, uint32_t size);
void *get_page(Pager *pager, uint32_t num_page);

#endif
