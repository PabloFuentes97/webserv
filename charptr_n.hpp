#pragma once
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <iostream>

typedef	struct	charptr_n
{
	typedef enum states {EMPTY, ALLOC, CALLOC, PART, FULL} states;
	char		*ptr;
	char		*cursor;
	size_t		pos;
	size_t		size;
	size_t		filled;
	enum states	state;

	charptr_n();
	charptr_n(size_t bytes, int flag);
	charptr_n(char *s, size_t len);
	charptr_n(char *s, size_t len, size_t to_fill);
	charptr_n(const charptr_n &copy);
	charptr_n	&operator=(const charptr_n &assign);
	charptr_n operator+(const charptr_n &add);
	int	operator+=(const charptr_n &add);
	charptr_n operator+(const char *add);
	int	operator+=(const char *add);
	~charptr_n();
} charptr_n;

charptr_n	subcharptr(charptr_n &charptr, size_t pos, size_t size);
int operator<<(int fd, charptr_n &charptr);