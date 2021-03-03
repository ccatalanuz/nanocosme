/*
 *  name_server.cpp
 *
 *  v2.0 CCC 04/2018
 *  
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <math.h>	// defines NAN

//#include "mutexes.h"
#include "name_server.h"

enum {FREE, OCCUPIED, DELETED};

#define LENGTH_HASH_TABLE 997       // prime number, e.g. 997, 3001, 5003, 9973, 15013, 30011, 50021, 125003
#define LOAD_FACTOR_HASH_TABLE 0.7

struct name {
  char name[LENGTH_NAME];
  char type[LENGTH_NAME];
  void *addr;
  int length;  
  unsigned char properties;  
  unsigned char state;
};

struct name hash_names[LENGTH_HASH_TABLE];
int names;

struct list {
  char name[LENGTH_NAME];
  list *iterator;
  list *next;  
};

// INICIALIZACION POR HERENCIA DE PRIORIDAD CON ESTO??
// VER init_muttexes en runtime.cpp
pthread_mutex_t set_number_name_mutex = PTHREAD_MUTEX_INITIALIZER, 
                get_number_name_mutex = PTHREAD_MUTEX_INITIALIZER, 
                set_string_name_mutex = PTHREAD_MUTEX_INITIALIZER, 
                get_string_name_mutex = PTHREAD_MUTEX_INITIALIZER;

/*******************************************************
 * void init_name_server()
 ******************************************************/
void init_name_server() {
  int i;
  
  for (i = 0; i < LENGTH_HASH_TABLE; i++) {
    hash_names[i].addr = NULL;
    hash_names[i].state = FREE;
  }
  names = 0;
}

/************************************************
 * int long hash(char *name)
 *
 * djb2 hash function
 ************************************************/
int hash(const char *name) {
  int hash = 5381;
  int c;

  while ((c = *name++))
    hash = ((hash << 5) + hash) + c;

  return hash % LENGTH_HASH_TABLE;
}

/************************************************
*  int get_position(const char *name)
*************************************************/
int get_position(const char *name) {
  int i, d;
  
  i = hash(name);
  
  if ((hash_names[i].state == FREE) || (hash_names[i].state == DELETED)) {
    return NAME_UNKNOWN;
  } else {

    if ((strcmp(hash_names[i].name, name)) == 0)
      return i;
    else {
      d = 1;
      do {
        i = (i + d) % LENGTH_HASH_TABLE;
        d = d + 2;
      } while (((hash_names[i].state == OCCUPIED) && ((strcmp(hash_names[i].name, name)) != 0)) || 
               (hash_names[i].state == DELETED));

      if (hash_names[i].state == FREE) { 
        return NAME_UNKNOWN;
      }
      else return i;
    }
  }
}

/*********************************************************
* int new_name(const char *name, const char *type, int length, 
*              unsigned char properties, void *addr)
**********************************************************/
int new_name(const char *name, const char *type, int length, 
              unsigned char properties, void *addr) {
  int i, d, size;

  if (get_addr_name(name) != NULL) {
    return NAME_EXIST;
  }

  if (names > LENGTH_HASH_TABLE * LOAD_FACTOR_HASH_TABLE) {
    return NAMES_EXCEEDED;     
  }

  if(strlen(name) > LENGTH_NAME) {
    return LENGTH_NAME_EXCEEDED;   
  }

  if(strlen(type) > LENGTH_NAME) {
    return LENGTH_TYPE_EXCEEDED;   
  }

  i = hash(name);
 
  // with occupied position seeks another  
  if (hash_names[i].state == OCCUPIED) {
    d = 1;
    do {
      i = (i + d) % LENGTH_HASH_TABLE ; 
      d = d + 2;
      if (d == LENGTH_HASH_TABLE) {
        return NAME_NOT_REGISTERED;
      }      
    } while (hash_names[i].state == OCCUPIED);
  }

  // insert name  
  strcpy(hash_names[i].name, name);
  strcpy(hash_names[i].type, type);

  if (addr == NULL) {
    if (strcmp(type, INT_NAME) == 0) {
      size = sizeof(int);
    } 
    else if (strcmp(type, DOUBLE_NAME) == 0) {
      size = sizeof(double);
    }
    else if (strcmp(type, LIST_NAME) == 0) {
      size = sizeof(list);
    }
    else {
      size = length;
    }
    
    hash_names[i].addr = malloc(size);
    
    if (hash_names[i].addr == NULL) {
      return MEMORY_ERROR;
    }
  } 
  else {
    hash_names[i].addr = addr;
  }

  if (strcmp(type, LIST_NAME) == 0) {
    ((list *)hash_names[i].addr)->name[0] = '\0';  
    ((list *)hash_names[i].addr)->iterator = NULL;
    ((list *)hash_names[i].addr)->next = NULL; 
  } 
  else {
    *((int *)hash_names[i].addr) = 0;
  }  
  hash_names[i].length = length;
  hash_names[i].properties = properties;
  hash_names[i].state = OCCUPIED;

//  printf("--> name = %s\n", hash_names[i].name);

  return 0;
}  

/*********************************************************
 * void unregister_name(const char *name)
 *********************************************************/
int unregister_name(const char*name) {
  int i;
    
  if ((i = get_position(name)) == NAME_UNKNOWN)
    return NAME_UNKNOWN;

  hash_names[i].state = DELETED;  
  names--;

  return 0;
}

/*********************************************************
 * void *get_addr_name(const char *name)
 *********************************************************/
void *get_addr_name(const char *name) {
  int i;
  
  if ((i = get_position(name)) == NAME_UNKNOWN) 
    return NULL;

  return hash_names[i].addr;
}

/*********************************************************
 * void *get_type_name(const char *name)
 *********************************************************/
char *get_type_name(const char*name) {
  int i;
  
  if ((i = get_position(name)) == NAME_UNKNOWN) 
    return NULL;

  return hash_names[i].type;
}

/*********************************************************
 * int get_properties_name(const char*name) 
 *********************************************************/
int get_properties_name(const char *name) {
  int i;
  
  if ((i = get_position(name)) == NAME_UNKNOWN) 
    return NAME_UNKNOWN;

  return hash_names[i].properties;
}

/*********************************************************
 * int set_properties_name(const char *name, unsigned char properties);
 *********************************************************/
int set_properties_name(const char *name, unsigned char properties) {
  int i;
  
  if ((i = get_position(name)) == NAME_UNKNOWN) 
    return NAME_UNKNOWN;

  hash_names[i].properties = properties;
  return 0;
}

/*********************************************************
 * int get_length(const char *name) 
 *********************************************************/
int get_length(const char *name) {
  int i;
  
  if ((i = get_position(name)) == NAME_UNKNOWN) 
    return NAME_UNKNOWN;

  return hash_names[i].length;
}

/*********************************************************
 * int to_string_name(const char *name, char *value)
 *********************************************************/
char *to_string_name(const char *name, char *value) {
  char *type;

  type = get_type_name(name);
  if (type == NULL) 
    return NULL;

  if (strcmp(type, INT_NAME) == 0) 
    sprintf(value, "%d", *(int *)get_addr_name(name));	

  else if (strcmp(type, DOUBLE_NAME) == 0) 
    sprintf(value, "%lf", *(double *)get_addr_name(name));	

  else if (strcmp(type, STRING_NAME) == 0) 
    sprintf(value, "\"%s\"", (char *)get_addr_name(name));	
//   strcpy(value, (char *)get_addr_name(name));

  else 
    sprintf(value, "%s", type);

  return value;
}

/*********************************************************
 * int to_value_name(const char *name, char *value)
 *********************************************************/
int to_value_name(const char *name, char *svalue) {
  char *c;
  double dvalue;
  int ivalue;
  char *type = get_type_name(name);

  if (type == NULL) 
    return NAME_UNKNOWN;

  if (strcmp(type, INT_NAME) == 0) {
    ivalue = strtol(svalue, &c, 10);
    if (*c != '\0') {
      return NAME_VALUE_WRONG;
    }
    return set_name(name, ivalue);
  } 
  else if (strcmp(type, DOUBLE_NAME) == 0) {
    dvalue = strtod(svalue, &c);
    if (*c != '\0') {
      return NAME_VALUE_WRONG;
    }
    return set_name(name, dvalue);  
  }
  else if (strcmp(type, STRING_NAME) == 0) {
    return set_name(name, svalue);    
  }

  return NAME_TYPE_RDONLY; 
}

/*********************************************************
 * int set_name(const char *name, const int value)	
 *********************************************************/
int set_name(const char *name, const int value) {
  return set_name(name, (double)value);
}

/*********************************************************
 * int set_name(const char *name, const double value)	
 *********************************************************/
int set_name(const char *name, const double value) {
  char *type = get_type_name(name);
  
  if (type == NULL) {
    return NAME_UNKNOWN;
  }
/*
  if ((get_properties_name(name) & RDONLY) != 0) {
    return NAME_RDONLY;
  }
*/
  if (strcmp(type, INT_NAME) == 0) {
    pthread_mutex_lock(&set_number_name_mutex);  
    *((int *)get_addr_name(name)) = (int)value; 
    pthread_mutex_unlock(&set_number_name_mutex);
  } 
  else if (strcmp(type, DOUBLE_NAME) == 0) {
    pthread_mutex_lock(&set_number_name_mutex);  
    *((double *)get_addr_name(name)) = value; 
    pthread_mutex_unlock(&set_number_name_mutex);  
  } 
  else {
    return NOT_NUMBER_NAME;
  }

  return 0;
}

/*********************************************************
 * int set_name(const char *name, const char *value);
 *********************************************************/
int set_name(const char *name, const char *value) {
  char *type = get_type_name(name);

  if (type == NULL) {
    return NAME_UNKNOWN;
  }

  if ((get_properties_name(name) & RDONLY) != 0) {
    return NAME_RDONLY;
  }

  if (strcmp(type, STRING_NAME) == 0) {
    if (strlen(value) > (unsigned int)get_length(name)) {
      return LENGTH_VALUE_EXCEEDED;  
    }

    pthread_mutex_lock(&set_string_name_mutex);  
    strcpy((char *)get_addr_name(name), value);
    pthread_mutex_unlock(&set_string_name_mutex); 
    
    return 0;    
  } 
  else {
    return NOT_STRING_NAME;
  }

  return 0;
}

/*********************************************************
 * int get_name(const char *name, int *value)
 *********************************************************/
int get_name(const char *name, int *value) {
  char *type = get_type_name(name);

  if (type == NULL) { 
    return NAME_UNKNOWN;
  }

  if (strcmp(type, INT_NAME) == 0) { 
    pthread_mutex_lock(&get_number_name_mutex);  
    *value = *(int *)get_addr_name(name);
    pthread_mutex_unlock(&get_number_name_mutex);  
  }
  else {  
    return NOT_INT_NAME;
  }
  
  return 0;
}

/*********************************************************
 * int get_name(const char *name, double *value)
 *********************************************************/
int get_name(const char *name, double *value) {
  char *type = get_type_name(name);

  if (type == NULL) { 
    return NAME_UNKNOWN;
  }

  if (strcmp(type, DOUBLE_NAME) == 0) {
    pthread_mutex_lock(&get_number_name_mutex);  
    *value = *(double *)get_addr_name(name);	
    pthread_mutex_unlock(&get_number_name_mutex);        
  } 
  else {  
    return NOT_DOUBLE_NAME;
  }
  
  return 0;
}

/*********************************************************
 * int get_name(const char *name, char *value)
 *********************************************************/
int get_name(const char *name, char *value) {
  char *type = get_type_name(name);

  if (type == NULL) { 
    return NAME_UNKNOWN;
  }

  if (strcmp(type, STRING_NAME) == 0) { 
    pthread_mutex_lock(&get_string_name_mutex);    
    strcpy(value, (char *)get_addr_name(name));
    pthread_mutex_unlock(&get_string_name_mutex);
    return 0;
  }
  
  return NOT_STRING_NAME;
}

/*********************************************************
 * char *add_dot(char *name)
 *********************************************************/
char *add_dot(char *name) {
  // PROVISIONAL
  return name;
}

/*********************************************************
 * char *remove_dot(char *name)
 *********************************************************/
char *remove_dot(char *name) {
  int i;
  
  for(i = 0; i < (int)strlen(name); i++) {
    name[i] = name[i + 1];
  }
  return name;
}

/*********************************************************
 * int new_name_list(const char *name_list);
 *********************************************************/
int new_name_list(const char *name_list) {
  char name_list_dot[LENGTH_NAME];	// PASAR A add_dot
  strcpy(name_list_dot, ".");
  strcat(name_list_dot, name_list);

  return new_name(name_list_dot, LIST_NAME, RDONLY);
}

/*********************************************************
 * int add_name(const char *name_list, const char *name);
 *********************************************************/
int add_name(const char *name_list, const char *name) {
  char name_list_dot[LENGTH_NAME];
  
  strcpy(name_list_dot, ".");
  strcat(name_list_dot, name_list);

  list *next_element, *new_element;
  list *element = (list *)get_addr_name(name_list_dot);
  
  if (element == NULL) {
    return NAME_LIST_UNKNOWN; 
  }
  
  if (get_addr_name(name) == NULL) {
    return NAME_UNKNOWN;
  }

  // first name
  if (element->name[0] == '\0') {  // *element->name == NULL
    strcpy(element->name, name);
    return 0;  
  }
  // next names
  else {
    // create new element
    new_element = (list *)malloc(sizeof(list));
    if (new_element == NULL) {
      return MEMORY_ERROR;
    } 
    strcpy(new_element->name, name);
    new_element->next = NULL;    
  
    // insert at the end
    next_element = element->next;      
    while(next_element != NULL) {
      element = next_element;
      next_element = next_element->next;
    }
    element->next = new_element;  
  }  
  
  return 0;
}

/*********************************************************
 * int new_iterator_names(const char *name_list);
 *********************************************************/
int new_iterator_names(const char *name_list) {
  char name_list_dot[LENGTH_NAME];
  strcpy(name_list_dot, ".");
  strcat(name_list_dot, name_list);

  list *element = (list *)get_addr_name(name_list_dot);
  
  if (element == NULL) {
    return NAME_UNKNOWN; 
  }

  element->iterator = element;  
  return 0;
}

/*********************************************************
 * bool has_next(const char *name_list);
 *********************************************************/
bool has_next_name(const char *name_list) {
  char name_list_dot[LENGTH_NAME];
  strcpy(name_list_dot, ".");
  strcat(name_list_dot, name_list);

  list *element = (list *)get_addr_name(name_list_dot);

  if (element == NULL) {
    return false; 
  }

  return (element->name[0] != '\0') && (element->iterator != NULL);
}

/*********************************************************
 * char *next_name(const char *name_list);
 *********************************************************/
char *next_name(const char *name_list) {
  char name_list_dot[LENGTH_NAME];
  strcpy(name_list_dot, ".");
  strcat(name_list_dot, name_list);

  list *element = (list *)get_addr_name(name_list_dot);
  char *name = NULL;

  if (element == NULL) {
    return NULL; 
  }

  if (element->iterator != NULL) {
    name = element->iterator->name;
    element->iterator = element->iterator->next;
  }
  
  return name;
}

/*********************************************************
 * bool is_name_list(const char *name);
 *********************************************************/
bool is_name_list(const char *name) {
  char name_list[LENGTH_NAME];
  char *type;

  sprintf(name_list, ".%s", name);

  type = get_type_name(name_list);

  return (type != NULL) && (strcmp(type, LIST_NAME) == 0);
}