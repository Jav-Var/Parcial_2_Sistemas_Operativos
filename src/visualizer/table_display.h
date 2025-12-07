#include "../collector/parser.h"

#ifndef TABLE_DISPLAY_H
#define TABLE_DISPLAY_H

/* Muestra los datos de todos los hosts con .active = true */
void update_table(struct host_info *hosts);

#endif