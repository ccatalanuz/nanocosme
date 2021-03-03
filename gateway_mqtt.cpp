/*  gateway_mqtt.cpp
 *
 *  v1.0 CCC 12/2020
 * 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTAsync.h>

#include "runtime.h"
#include "name_server.h"
#include "application.h"
#include "gateway_mqtt.h"

#define QOS 1

MQTTAsync client;


/***********************************
* char *get_config(char *key)
***********************************/
char *get_config(char *key) {
  char line[LENGTH_STRING];
  char prefix[LENGTH_STRING];
  
  FILE *fp = fopen("config.xml", "r");
  
  if (fp == NULL) {
    return NULL;
  }
  
  while (fgets(line, sizeof(line), fp) != NULL) {
    if(strstr(line, key) != NULL) {
      sprintf(prefix, "<entry key=\"%s\">", key);
      return strtok(line, prefix);
    }       
  }
  
  fclose(fp);
  
  return NULL;
}

/***********************************
* void connection_lost(void *context, char *cause)
***********************************/
void connection_lost(void *context, char *cause) {
  char msg[LENGTH_STRING];

  sprintf(msg, "\nConnection mqtt lost, cause: %s\n", cause);
  on_mqtt_failure(msg);
}

/***********************************
* int message_arrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
***********************************/
int message_arrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
  char *payloadptr;
  char c;
  int i;
  int j;
  char msg[LENGTH_STRING];
  int rc;

  payloadptr = (char *)message->payload;
  for (i = 0, j = 0; i < message->payloadlen; i++) {
    if((c = *payloadptr++) != '"') {
      msg[j++] = c;
    }
  }
  msg[j] = '\0';

  if ((rc = to_value_name(topicName, msg)) == 0) {
    on_mqtt_name_arrived(topicName);  
  } else {
    sprintf(msg, "Failure mqtt msg arrived, topic = %s, rc = %d\n", topicName, rc);
    on_mqtt_failure(msg);        
  }

  MQTTAsync_freeMessage(&message);
  MQTTAsync_free(topicName);
  
  return 1;
}

/***********************************
* void delivery_complete(void *context, MQTTAsync_token token)
***********************************/
void delivery_complete(void *context, MQTTAsync_token token) {
  // TODO
}

/***********************************
* void on_connect(void *context, MQTTAsync_successData *response)
***********************************/
void on_connect(void *context, MQTTAsync_successData *response) {
  debug(1, "Successful mqtt connection");  
  on_mqtt_connected();
}

/***********************************
* void on_disconnect(void *context, MQTTAsync_successData *response)
***********************************/
void on_disconnect(void *context, MQTTAsync_successData *response) {
  debug(1, "Successful mqtt disconnection");  
  on_mqtt_disconnected();
}

/***********************************
* void on_connect_failure(void *context, MQTTAsync_failureData *response)
***********************************/
void on_connect_failure(void *context, MQTTAsync_failureData *response) {
  char msg[LENGTH_STRING];

  sprintf(msg, "Failure mqtt connect, rc = %d\n", response ? response->code : 0);
  on_mqtt_failure(msg);      
}

/***********************************
* void on_subscribe(void *context, MQTTAsync_successData *response)
***********************************/
void on_subscribe(void *context, MQTTAsync_successData *response) {
  debug(1, "Successful mqtt subscribe");  
}

/***********************************
* void on_subscribe_failure(void *context, MQTTAsync_failureData *response)
***********************************/
void on_subscribe_failure(void *context, MQTTAsync_failureData *response) {
  char msg[LENGTH_STRING];
  
  sprintf(msg, "Failure mqtt subscribe, rc = %d\n", response ? response->code : 0);
  on_mqtt_failure(msg);    
}

/***********************************
* void on_send(void *context, MQTTAsync_successData *response)
***********************************/
void on_send(void *context, MQTTAsync_successData *response) {
//  debug(1, "Success mqtt sendMessage");
}

/***********************************
* void init_gateway_mqtt()
***********************************/
void init_gateway_mqtt() {
  MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
  int rc;
  char msg[LENGTH_STRING];
  char mqtt_host[LENGTH_STRING];
  char clientID[LENGTH_NAME];

  strcpy(mqtt_host, get_config("mqtt_broker"));
  strcpy(clientID, get_config("applicationID"));

  MQTTAsync_create(&client, mqtt_host, clientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

  MQTTAsync_setCallbacks(client, NULL, connection_lost, message_arrived, delivery_complete);  
  
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  conn_opts.onSuccess = on_connect;
  conn_opts.onFailure = on_connect_failure;
  conn_opts.context = client;
  
  if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
    sprintf(msg, "Failed to start mqtt connect, return code %d\n", rc);
    on_mqtt_failure(msg);    
  }
}

/***********************************
* void finalize_gateway_mqtt()
***********************************/
void finalize_gateway_mqtt() {
  MQTTAsync_disconnectOptions disconn_opts = MQTTAsync_disconnectOptions_initializer;
  int rc;
  char msg[LENGTH_STRING];

  disconn_opts.onSuccess = on_disconnect;
  disconn_opts.context = client;
  
  if ((rc = MQTTAsync_disconnect(client, &disconn_opts)) != MQTTASYNC_SUCCESS) {
    sprintf(msg, "Failed to start mqtt disconnect, return code %d\n", rc);
    on_mqtt_failure(msg);    
  }
}

/***********************************
* int publish_name(const char *name)
************************************/
int publish_name(const char *name) {
  MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
  MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
  int rc;
  char msg[LENGTH_STRING];
  char value[LENGTH_STRING];
  
  strcpy(value, to_string_name(name, value));

  opts.onSuccess = on_send;
  opts.context = client;
  pubmsg.payload = value;
  pubmsg.payloadlen = strlen(value);
  pubmsg.qos = QOS;
  pubmsg.retained = 0;
  
  if ((rc = MQTTAsync_sendMessage(client, name, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
    sprintf(msg, "Failed to start mqtt sendMessage, return code %d\n", rc);
    on_mqtt_failure(msg);
  }
}

/***********************************
* int publish(const char *name)
***********************************/
int publish(const char *name) {
  char *name_list;

  if (is_name_list(name)) {
    new_iterator_names(name);
    
    while (has_next_name(name)) {
      publish_name(next_name(name));
    }    
  } else {
    if (get_type_name(name) == NULL) {
      return NAME_UNKNOWN;
    }
    
    publish_name(name);
  }
  
  return 0;
}

/***********************************
* int subscribe(const char *name)
***********************************/
int subscribe(const char *name) {
  MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
  int rc;
  char msg[LENGTH_STRING];
  char value[LENGTH_STRING];
  
  strcpy(value, to_string_name(name, value));

  opts.onSuccess = on_subscribe;
  opts.onFailure = on_subscribe_failure;
  opts.context = client;
  
  if ((rc = MQTTAsync_subscribe(client, name, QOS, &opts)) != MQTTASYNC_SUCCESS) {
    sprintf(msg, "Failed to start mqtt subscribe, return code %d\n", rc);
    on_mqtt_failure(msg);
  }
}

