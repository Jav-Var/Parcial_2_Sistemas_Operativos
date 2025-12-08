#define _GNU_SOURCE
#include "common.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> // mover time_now_ms

#ifndef PARSER_H

#define PARSER_H



void parse_mem_data(char *line, struct host_info *info_ptr);
void parse_cpu_data(char *line, struct host_info *info_ptr);




#endif