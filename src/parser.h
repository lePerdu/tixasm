/**
 * @file parser.h
 * @author Zach Peltzer
 * @date Created: Fri, 02 Feb 2018
 * @date Last Modified: Sat, 03 Feb 2018
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdint.h>

#include "symbol_table.h"

typedef struct parser_data {
    char const *stream;

    section cur_sec;
    uint16_t cur_pc;

    symbol_table sym_table;
    reloc_table rel_table;
} parser_data;

int parser_init(parser_data *parser, const char *stream);
void parser_destroy(parser_data *parser);

int parser_parse(parser_data *parser);

#endif /* PARSER_H_ */

/* vim: set tw=80 ft=c: */
