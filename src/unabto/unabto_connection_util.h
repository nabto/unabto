#ifndef _UNABTO_CONNECTION_UTIL_H_
#define _UNABTO_CONNECTION_UTIL_H_

#include <unabto/unabto_packet_util.h>
#include <unabto/unabto_connection.h>

// Read client id from packet and insert it into the connection,
// return true if ok or false if client id not found or unsuccessful.
bool unabto_connection_util_read_client_id(const nabto_packet_header* header, nabto_connect* connection);

// read fingerprint payload from connect packet.
bool unabto_connection_util_read_fingerprint(const nabto_packet_header* header, nabto_connect* connection);

bool unabto_psk_connection_util_verify_connect(const nabto_packet_header* header, nabto_connect* connection);

#endif
