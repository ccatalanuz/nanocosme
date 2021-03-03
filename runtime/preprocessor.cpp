/*
 *  preprocessor.cpp
 *
 *  Automatic code generator for nanocosme
 *
 *  v4.0 CCC 05/2018
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "name_server.h"

#define MAX_LOOPS 20
#define MAX_MUTEX 50
#define MAX_NAMES 50

#define APPLICATION_FILE_TMP2   "application_mod.cpp"
#define APPLICATION_H_TEMPLATE  "application.h.tem"
#define LOOP_HANDLERS_TEMPLATE  "loop_handlers.tem"
#define LOOP_HANDLERS_FILE      "loop_handlers.cpp"
#define SYNCHRONIZED_H_TEMPLATE "synchronized.tem"
#define SYNCHRONIZED_H          "synchronized.h"
#define MAX_CAR_LINE 		256
#define MAX_PERIOD_MS 		2000

#define SYNCHRONIZED_ANNOTATION "@synchronized"
#define END_ANNOTATION          "@end"
#define PUBLIC_ANNOTATION       "@public"
#define REMOTE_ANNOTATION       "@remote"
#define GET_ANNOTATION          "@get"
#define SET_ANNOTATION          "@set"
#define CONST_ANNOTATION        "const"
#define PERSISTENT_ANNOTATION   "persistent"

#define NON_PERSISTENT	0
#define PERSISTENT      1

#define MUTEX_PREFIX              "__mutex__"

#define COSME_DEFAULT_TOPICS_FILE "cosme_default_topics.xml"
#define COSME_TOPICS_DIRECTORY    "./config/"
#define DEFAULT_TOPIC             "default"
#define PERSISTENT_TOPIC          "persistent"

#define NAMES_FILE  "names.txt"

#define MAX_DEFINES 100

struct loop_param {
  int number;
  int priority;
  int period_ms;
};

struct define {
  char id[LENGTH_NAME];
  char name[LENGTH_NAME];
};

struct loop_param loops[MAX_LOOPS];

char application_file1[MAX_CAR_LINE];
char application_file2[MAX_CAR_LINE];
char application_file_h[MAX_CAR_LINE];
char application_file_h_template[MAX_CAR_LINE];
char loop_handlers_template[MAX_CAR_LINE];

struct define defines[MAX_DEFINES];

extern int errno;

/*****************************************************************
 * void read_loop_tokens(char *line, int *number, int *period) 
 ****************************************************************/
void read_loop_tokens(char *line, int *number, int *period) {
  char tokens[MAX_CAR_LINE];
  char *token;

  //  void loop1(int period_ms = 10)
  strcpy(tokens, line);

  strtok(tokens, "p");

  token = strtok(NULL, " (");
  *number = atoi(token);

  strtok(NULL, " ");
  strtok(NULL, " ");
  strtok(NULL, " ");
  token = strtok(NULL, " )");
    
  *period = atoi(token);
/*  
  if(*period > MAX_PERIOD_MS) {
    printf("%s\nError max. period %d\n", line, MAX_PERIOD_MS);
    exit(1);
  }
*/    
}  

/*****************************************************************
 * void read_loop_tokens(char *line, int *number, int *period) 
 ****************************************************************/
int loop_priority_cmp(const void *op1, const void *op2) {
  struct loop_param *lp1 = (struct loop_param *)op1;
  struct loop_param *lp2 = (struct loop_param *)op2;
  int ret = 0;

  if (lp1->period_ms < lp2->period_ms)
    ret = -1;
  else if (lp1->period_ms > lp2->period_ms)
    ret = 1;
  else
    ret = 0;
    
  // descending order  
  return -ret;
}  

/*****************************************************************
 * void read_loop_tokens(char *line, int *number, int *period) 
 ****************************************************************/    
void loops_parsing(struct loop_param loops[]) {
  char line[MAX_CAR_LINE];
  int n = 0;
  
  FILE *fapp = fopen(application_file1, "r");
  if (fapp == NULL) {
    printf("Error opening application file\n");
    exit(1);
  }

  while(fgets(line, MAX_CAR_LINE, fapp) != NULL) {
    if (strstr(line, "void loop") != NULL) {
      read_loop_tokens(line, &loops[n].number, &loops[n++].period_ms);
    }     
  }
  fclose(fapp);
} 

/*****************************************************************
 * void priority_loop_assign(struct loop_param loops[])
 ****************************************************************/    
void priority_loop_assign(struct loop_param loops[MAX_LOOPS]) {
  int i = 0;

  qsort(loops, MAX_LOOPS, sizeof(struct loop_param), loop_priority_cmp);

  while ((loops[i].number != 0) && (i < MAX_LOOPS)) {
    loops[i++].priority = MAX_LOOPS - i;
  }  
}    

/*****************************************************************
 * void generate_application_header(struct loop_param loops[]) 
 ****************************************************************/
void generate_application_header(struct loop_param loops[]) {
  int i = 0;
  char line[MAX_CAR_LINE];
  FILE *ftemp, *fapp;

//  ftemp = fopen(APPLICATION_H_TEMPLATE, "r");
  ftemp = fopen(application_file_h_template, "r");
  if (ftemp == NULL) {
    printf("Error opening application.h template\n");
    
//    printf("--> errno = %d\n", errno);
//    system("pwd");    
    
    exit(1);
  }

  fapp = fopen(application_file_h, "w");
  if (fapp == NULL) {
    printf("Error opening application.h file\n");
    exit(1);
  }

  // copy template
  while (fgets(line, MAX_CAR_LINE, ftemp) != NULL) {
    fputs(line, fapp);
  } 
  fclose(ftemp);

  // append loop prototypes
  fprintf(fapp, "\n");
  while ((loops[i].number != 0) && (i < MAX_LOOPS)) {
    sprintf(line, "void loop%d(int period_ms);\n", loops[i].number);
    i++;
    fprintf(fapp, "%s", line);   
  }

  fclose(fapp);
}

/*****************************************************************
 * void generate_loop_handlers(struct loop_param loops[])
 ****************************************************************/
void generate_loop_handlers(struct loop_param loops[]) {
  int i = 0;
  char line[MAX_CAR_LINE];
  FILE *ftemp, *floop;

  ftemp = fopen(loop_handlers_template, "r");
  if (ftemp == NULL) {
    printf("Error opening loop_handlers template\n");
    exit(1);
  }

  floop = fopen(LOOP_HANDLERS_FILE, "w");
  if (floop == NULL) {
    printf("Error opening loop_handlers file\n");
    exit(1);
  }

  // copy template
  while (fgets(line, MAX_CAR_LINE, ftemp) != NULL) {
    fputs(line, floop);
  } 
  fclose(ftemp);

  // append loop handlers
  fprintf(floop, "\n");
  while ((loops[i].number != 0) && (i < MAX_LOOPS)) {
//    sprintf(line, "LOOP_HANDLER(loop_handler%d)\n", loops[i].number);

    // PARA VAR TIEMPO CICLO AUTOMÁTICO
    sprintf(line, "LOOP_HANDLER(\".loop%d_cycles\", \".loop%d_period\", loop_handler%d)\n", 
                  loops[i].number, loops[i].number, loops[i].number);

    i++;
    fprintf(floop, "%s", line);   
  }

  fprintf(floop, "\nvoid init_loop_handlers() {\n");

  i = 0;  
  while ((loops[i].number != 0) && (i < MAX_LOOPS)) {
    sprintf(line, "  START_LOOP_HANDLER(lhp%d, loop%d, loop_handler%d, %d, %d);\n",
                    loops[i].number, loops[i].number, loops[i].number, loops[i].priority, loops[i].period_ms);  
    i++;
    fprintf(floop, "%s", line);
  }

  fprintf(floop, "}\n");

  fclose(floop);
}

/*****************************************************************
 * void generate_names()
 ****************************************************************/    
void generate_names() {
  char line[MAX_CAR_LINE];
  char *name;
//  char *id;
  int num_defines = 0;

  FILE *fapp = fopen(application_file1, "r");
  if (fapp == NULL) {
    printf("Error opening application file\n");
    exit(1);
  }  

  FILE *fnames = fopen(NAMES_FILE, "w");
  if (fnames == NULL) {
    printf("Error opening names file\n");
    exit(1);
  }

  while(fgets(line, MAX_CAR_LINE, fapp) != NULL) {
    if(strstr(line, "#define") != NULL) {
      strtok(line, " ");
      strcpy(defines[num_defines].id, strtok(NULL, " "));
      strcpy(defines[num_defines++].name, strtok(NULL, "\""));
    }
    else if(strstr(line, "new_name") != NULL) {
      strtok(line, "\"");
      name = strtok(NULL, "\"");
      
      // nombre con define
      if (name == NULL) {
        strtok(line, "(");
        name = strtok(NULL, ",)");
        
        for(int n = 0; n < num_defines; n++) {
          if (strcmp(defines[n].id, name) == 0) {
            name = defines[n].name;
            break;
          }
        }
      }
    
      if(strstr(line, "new_name_list") != NULL) {
        fprintf(fnames, ".%s\n", name);
      } else  {
        fprintf(fnames, "%s\n", name);
      }
    } 
  } 
  
  // system names
  int i = 0;
  while ((loops[i].number != 0) && (i < MAX_LOOPS)) {
    fprintf(fnames, ".loop%d_cycles\n", loops[i].number);
    fprintf(fnames, ".loop%d_period\n", loops[i++].number);   
  }

  fclose(fapp);  
  fclose(fnames);
}

/*****************************************************************
 * int main(int argc, char *argv[])
 ****************************************************************/    
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("%s <application>.h <application>.cpp\n", argv[0]);
    exit(1);
  } else {
    strcpy(application_file_h, argv[1]);
    strcpy(application_file1, argv[2]);
    
    strcpy(application_file_h_template, argv[3]);
    strcat(application_file_h_template, APPLICATION_H_TEMPLATE);
 
    strcpy(loop_handlers_template, argv[3]);
    strcat(loop_handlers_template, LOOP_HANDLERS_TEMPLATE);
    
  }

  // loops
  loops_parsing(loops);
  priority_loop_assign(loops);     
  generate_application_header(loops);
  generate_loop_handlers(loops);

  generate_names();
} 
 
 