/*
 * application.cpp
 *
 * Example code
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

// specific constants
#define TOPIC_1 "topic_demo1"
#define TOPIC_2 "topic_demo2"
#define TOPIC_3 "topic_demo3"
#define TOPIC_4 "topic_demo4"

#define TOPIC_2_LENGTH 32

#define FOO_LIST "foo_list"

void setup() {
  printf("Executing setup...\n"); 
  
  new_name(TOPIC_1, INT_NAME);
  set_name(TOPIC_1, 0);
  
  new_name(TOPIC_2, STRING_NAME, TOPIC_2_LENGTH);
  set_name(TOPIC_2, "prueba");  
  
  new_name(TOPIC_3, DOUBLE_NAME);
  set_name(TOPIC_3, 0);
  
  new_name(TOPIC_4, INT_NAME);
  set_name(TOPIC_4, 777);
  
  new_name_list(FOO_LIST);  
  add_name(FOO_LIST, TOPIC_1);
  add_name(FOO_LIST, TOPIC_2);
  add_name(FOO_LIST, TOPIC_3);
}

void loop1(int period_ms = 1000) {
  static int valor = 0;

  printf(".\n");
  
  set_name(TOPIC_1, valor++);

//  publish(FOO_LIST);
  publish(TOPIC_1);
}

void on_mqtt_connected() {
  subscribe(TOPIC_1);
  subscribe(TOPIC_2);
  subscribe(TOPIC_3);
  subscribe(TOPIC_4);
}

void on_mqtt_disconnected() {

}

void on_mqtt_failure(const char *msg) {
  printf("%s\n", msg);
}

void on_mqtt_name_arrived(const char *name) {
  int value_1;
  char value_2[LENGTH_STRING];
  double value_3;
  char *type = get_type_name(name);

  if (type == NULL) {
    return;
  }
    
  if (strcmp(type, INT_NAME) == 0) {
    get_name(name, &value_1);
    printf("Arrived %s = %d\n", name, value_1);      
  
  } else if (strcmp(type, STRING_NAME) == 0) {
    get_name(name, value_2);
    printf("Arrived %s = %s\n", name, value_2);      
    
  } else if (strcmp(type, DOUBLE_NAME) == 0) {
    get_name(name, &value_3);
    printf("Arrived %s = %f\n", name, value_3);      
  }
}

void finalize() {
  printf("Executing finalice...\n");
    
  // ...  
}
