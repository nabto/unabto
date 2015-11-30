#ifndef _UNABTO_TIME_AUTO_UPDATE_H_
#define _UNABTO_TIME_AUTO_UPDATE_H_

/**
 * Switch auto update of the timestamp on/off
 * 
 * If switched off you need to call unabto_unix_time_update_stamp by
 * yourself from your application.
 */
void unabto_time_auto_update(bool enabled);
void unabto_time_update_stamp();


#endif
