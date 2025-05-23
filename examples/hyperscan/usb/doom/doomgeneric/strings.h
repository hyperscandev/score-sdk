#ifndef _STRINGS_H_
#define _STRINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>  /* for size_t */

/* Memory operations */
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *ptr, int value, size_t num);
int   memcmp(const void *ptr1, const void *ptr2, size_t num);

/* String operations */
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);

int    strcmp(const char *s1, const char *s2);
int    strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);

char *strtok(char *str, const char *delim);

#ifdef __cplusplus
}
#endif

#endif /* _STRINGS_H_ */
