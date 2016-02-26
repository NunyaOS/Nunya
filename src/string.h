/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef STRING_H
#define STRING_H

void strcpy(char *d, const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, unsigned length);
unsigned strlen(const char *s);

char *strtok(char *s, const char *delim);

const char *strchr(const char *s, char ch);

void memset(void *d, char value, unsigned length);
void memcpy(void *d, const void *s, unsigned length);

void printf(const char *s, ...);

/**
 * @brief Make an upper-case copy of a string
 * @details Capitalize all letters in src and
 * copy into dest. All non-letter characters
 * are copied without conversion.
 * @param dest Destination buffer for the resultant
 * upper-case string
 * @param src Source string buffer to be capitalized.
 * This will not be modified.
 */
void to_upper(char *dest, const char *src);

/**
 * @brief Make a lower-case copy of a string
 * @details Lower-case all letters in src and
 * copy into dest. All non-letter characters
 * are copied without conversion.
 * @param dest Destination buffer for the resultant
 * lower-case string
 * @param src Source string buffer to be lower-cased.
 * This will not be modified.
 */
void to_lower(char *dest, const char *src);

/**
 * @brief Strip out backspace characters from buffer
 * @details Walks through character buffer src and
 * modifies the buffer by handling and then removing
 * all backspace characters. e.g. The buffer
 * [n,u,m,[backspace],n,u,a,[backspace],[backspace],y,a]
 * becomes [n,u,n,y,a].
 * Warning: this modifies the buffer src.
 * @param src The character buffer to be modified and
 * stripped of backspace characters.
 */
void strip_backspace(char *src);

#endif
