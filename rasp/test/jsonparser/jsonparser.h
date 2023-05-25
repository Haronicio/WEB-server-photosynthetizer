#include "cJSON.h"

#define MAX_JSON_SIZE 100000

cJSON * open_JSON_file(const char*);
void add_xydata_to_JSON(cJSON * ,char *,int);
int save_JSON_file(cJSON * ,const char*);
int msleep(long);
