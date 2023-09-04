/*
 * helpers.h
 *
 *  Created on: Jun 7, 2022
 *      Author: dkalaitzakis
 */

#ifndef SRC_CORE_HELPERS_H_
#define SRC_CORE_HELPERS_H_

#include <stdint.h>

const char *float_to_char(float num);

const char *int_to_char(uint16_t num);

int itoa(int value, char *sp, int radix);

int float2char(float num, char *buffer);

uint16_t random(uint16_t min, uint16_t max);

char* strtoke(char *str, const char *delim);

long coorsAtol(char *coors, char sign);

long altAtol(char *str);

#endif /* SRC_CORE_HELPERS_H_ */
