/*
 * Application template code
 * 
 * CCC 1/2021
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "runtime.h"
#include "gateway_mqtt.h"
#include "name_server.h"

#include "application.h"

// specific libraries


// Function executed by runtime just before loops
void setup() {
  // Declaration of names
  // type = INT_NAME | DOUBLE_NAME | STRING_NAME
  new_name(<name>, <type>, [<length>]);  
  
  // Assigning value to names
  set_name(<name>, <value>);
  
  // Create name list
  new_name_list(<name_list>);
  add_name(<name_list>, <name>);

  // Sequential accest to name list
  new_iterator_names(<name_list>);
  while (has_next_name(<name_list>)) {
    ... next_name(<name_list>);  // return next name list
  }  
}

// Loop functions are executed following rate monotonic (i.e. shorter period higher execution priority)

// Function executed periodically by runtime every period_1 
void loop1(int period_ms = <period_1>) {
  <var_type> <var>;

  // Setting value to a name
  set_name(<name>, <value>);
  
  // Getting value from a name
  get_name(<name>, &<var>);

  // Publish
  publish(<name> | <name_list>);
}

...

// Function executed periodically by runtime every period_N 
void loopN(int period_ms = <period_N>) {
  ...
}

// Function executed by runtime when device is connected to mqtt broker
void on_mqtt_connected() {
  subscribe(<name>);
  
  ...
}

// Function executed by runtime when device is disconnected to mqtt broker
void on_mqtt_disconnected() {
  ...
}

// Function executed by runtime when a mqtt failure has occurred
void on_mqtt_failure(const char *msg) {
  ...
}

// Function executed by runtime when one name has been arrived from mqtt
void on_mqtt_name_arrived(const char *name) {
  ...
}


// Function executed just before shutdown
void finalize() {
  ...
}
