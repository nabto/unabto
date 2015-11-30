#include "er-coap-13/er-coap-13-engine.h"
#include "erbium/erbium.h"

void er_coap_nabto_init( void )
{
   // coap_receiver_init();
   // rest_init_engine();
}

void er_coap_nabto_tick( void )
{
    coap_receiver_tick();
    rest_manager_tick();
}