#include "urrdtool.h"

urrdStatus urrd_create_table(uint8_t id, urrd_table* table);

bool urrd_open_table(urrd_table* table);

bool urrd_write_header(urrd_table* table, uint8_t pointerNo, urrd_pointer* data);
bool urrd_read_header(urrd_table* table, uint8_t pointerNo, urrd_pointer* data);

bool urrd_write_data(urrd_table* table, urrd_size offset, const void* data, urrd_size dataLength);
bool urrd_read_data(urrd_table* table, urrd_size offset, void* data, urrd_size dataLength);

uint32_t urrd_time();

