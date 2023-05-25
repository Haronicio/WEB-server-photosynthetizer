#include "cJSON.h"

#define WARN_JSON_SIZE 16364 //créé un nouveau fichier 
#define MAX_JSON_SIZE 999999  //1 Mo max

cJSON * open_JSON_file(const char*);
void add_xydata_to_JSON(cJSON * ,char *,int);
int save_JSON_file(cJSON * ,const char*);
//void clean_JSON_file(FILE *,long);
int msleep(long);
