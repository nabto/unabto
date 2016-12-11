#ifndef UNABTO_APPLICATION_H
#define UNABTO_APPLICATION_H

#include <unabto/unabto_app.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_env_base.h>

#include "LY096BG30.h"

// Missing function prototypes from LM75A.h
void print_Line(uint8_t Line, char Text[]);
void print_C(uint8_t Col, uint8_t Line, char ascii);

application_event_result application_event(application_request* request, unabto_query_request* read_buffer, unabto_query_response* write_buffer);
uint8_t setLight(uint8_t id, uint8_t onOff);
uint8_t readLight(uint8_t id);

#endif
