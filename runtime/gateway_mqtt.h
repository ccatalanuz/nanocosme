/*  gateway_mqtt.h
 *
 *  v1.0 CCC 12/2020
 *  
 */

void init_gateway_mqtt();
void finalize_gateway_mqtt();
int publish(const char *name);
int subscribe(const char *name);
