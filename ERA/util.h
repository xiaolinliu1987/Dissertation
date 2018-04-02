/* $Id: util.h,v 1.2 2006-05-01 18:32:08 rico Exp $ */

#ifndef PIPLIB_UTIL
#define PIPLIB_UTIL

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>     /* HUGE_VAL */
#include <limits.h>   /* INT_MAX, INT_MIN, LONG_MAX, LONG_MIN, etc. */
#include <assert.h>
#include <errno.h>

extern char *argv0;

void print_argv0(void);
#ifdef __GNUC__     /* avoid some "foo might be used uninitialized" warnings */
	void fatal(const char *msg) __attribute__ ((noreturn));
	void fatalf(const char *fmt, ...) __attribute__ ((noreturn));
	void fatalfr(const char *fmt, ...) __attribute__ ((noreturn));
#else
	void fatal(const char *msg);
	void fatalf(const char *fmt, ...);
	void fatalfr(const char *fmt, ...);
#endif
FILE *ckopen(const char *name, const char *mode);
void ckfree(void* p);
void *ckalloc(size_t amount);
void *ckallocz(size_t amount);
void *ckrealloc(void * p, size_t size);
int same_string(const char *s, const char *t);
int starts(const char *s, const char *t);
char *skip_ws(const char *s);
char *copy_string(const char *s);
char *copy_substring(const char *s, int n);
unsigned int roundup(unsigned int n, unsigned int m);
char *fasta_name(char *line);
void unix_error(const char *msg);


#undef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#undef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))

#endif
