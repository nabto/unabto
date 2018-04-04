#ifndef _UNABTO_CONNECTION_UTIL_H_
#define _UNABTO_CONNECTION_UTIL_H_


// Read client id from packet and insert it into the connection,
// return true if ok or false if client id not found or unsuccessful.
bool unabto_connection_util_read_client_id(const nabto_packet_header* header, nabto_connect* connection);

void unabto_psk_connection_util_verify_connect(const nabto_packet_header* header);

#endif
