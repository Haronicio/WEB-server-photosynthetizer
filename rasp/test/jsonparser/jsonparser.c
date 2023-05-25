#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>  

#include "jsonparser.h"

int main()
{
    srand(time(NULL));
    cJSON * json;
    char xVal[9999];

    for (int i = 0; i < 100; i++)
    {
        sprintf(xVal,"%d",i);
        if(!(json = open_JSON_file("../bd.json")))
        {
            printf("Error in open\n");
            return -1;
        }
        add_xydata_to_JSON(json,xVal,rand()%100);
        if(!save_JSON_file(json,"../bd.json"))
        {
            printf("Error in save\n");
            return -1;
        }
        msleep(500);
    }
    
    cJSON_Delete(json);
}


cJSON *open_JSON_file(const char* pathToJSON)
{
    FILE *fp = fopen(pathToJSON, "r");
	if (fp == NULL) {
		printf("Error: Unable to open the file.\n");
		return NULL;
    
	}

	// read the file contents into a string
	char buffer[MAX_JSON_SIZE];
	int len = fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);

	// parse the JSON data
	cJSON *json = cJSON_Parse(buffer);
	if (json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			printf("Error: %s\n", error_ptr);
		}
		cJSON_Delete(json);
		return NULL;
	}

    return json;
}


void add_xydata_to_JSON(cJSON * json,char * x,int y)
{
    cJSON * coo = cJSON_GetObjectItem(json,"coordonnees");
    cJSON * new_entry = cJSON_CreateObject();
    cJSON_AddStringToObject(new_entry, "x", x);
    cJSON_AddNumberToObject(new_entry, "y", y);
    cJSON_AddItemToArray(coo,new_entry);
}

int save_JSON_file(cJSON * json,const char* pathToJSON)
{
    char *json_str = cJSON_Print(json);

	// write the JSON string to the file
	FILE * fp = fopen(pathToJSON, "w");
	if (fp == NULL) {
		printf("Error: Unable to open the file.\n");
		return -1;
	}
	// printf("%s\n", json_str);
	fputs(json_str, fp);
	fclose(fp);

	// free the JSON string and cJSON object
	cJSON_free(json_str);

	return 1;
}

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}