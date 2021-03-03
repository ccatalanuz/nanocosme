/*  
 * name_server.h
 *
 *  v2.0 CCC 05/2018
 *  
 */
 

#define NAME_UNKNOWN	      -1
#define NAME_EXIST            -2
#define NAMES_EXCEEDED	      -3
#define LENGTH_NAME_EXCEEDED  -4
#define LENGTH_TYPE_EXCEEDED  -5
#define NAME_NOT_REGISTERED   -6
#define NAME_VALUE_WRONG      -7
#define NAME_TYPE_RDONLY      -8
#define LENGTH_VALUE_EXCEEDED -9
#define NAME_RDONLY          -10
#define MEMORY_ERROR	     -11
#define NOT_INT_NAME	     -12
#define NOT_DOUBLE_NAME      -13
#define NOT_NUMBER_NAME      -14
#define NOT_STRING_NAME      -15
#define NAME_LIST_UNKNOWN    -16

#define LENGTH_NAME   64
#define LENGTH_STRING 256

#define RDWR   0
#define RDONLY 1

#define INT_NAME       "int"
#define DOUBLE_NAME    "double" 
#define STRING_NAME    "string"
#define LIST_NAME      "list"

void init_name_server();
int new_name(const char *name, const char *type, int length = 0, 
             unsigned char properties = RDWR,
             void *addr = NULL);

void *get_addr_name(const char *name);
char *get_type_name(const char *name);
int get_length_name(const char *name);
int get_properties_name(const char *name);
int set_properties_name(const char *name, unsigned char properties);
char *to_string_name(const char *name, char *value);
int to_value_name(const char *name, char *value);
bool is_name_list(const char *name);


int set_name(const char *name, const int value);
int set_name(const char *name, const double value);
int set_name(const char *name, const char *value);

int get_name(const char *name, int *value);
int get_name(const char *name, double *value);
int get_name(const char *name, char *value);

char *remove_dot(char *name);
int new_name_list(const char *name_list);
int add_name(const char *name_list, const char *name);
//int remove_name(const char *name_list, const char *name);
int new_iterator_names(const char *name_list);
bool has_next_name(const char *name_list);
char *next_name(const char *name_list);
