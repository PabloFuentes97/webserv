#include "charptr_n.hpp"

charptr_n::charptr_n() : ptr(NULL), cursor(NULL), size(0), filled(0), state(EMPTY)
{
	std::cout << "Constructor básico" << std::endl;
};

charptr_n::charptr_n(size_t bytes, int flag) : ptr((char *)malloc(bytes)), cursor(NULL), size(bytes), filled(0), state(ALLOC)
{
	if (flag == 1)
	{
		bzero(ptr, size);
		state = CALLOC;
	}
};

charptr_n::charptr_n(char *s, size_t len)
{
	ptr = (char *)malloc(len);
	for (size_t	i = 0; i < len; i++)
		ptr[i] = s[i];
	size = len;
	filled = size;
	state = FULL;
};

charptr_n::charptr_n(char *s, size_t len, size_t to_fill)
{
	ptr = (char *)malloc(len);
	for (size_t	i = 0; i < to_fill; i++)
		ptr[i] = s[i];
	size = len;
	filled = to_fill;
	if (size == filled)
		state = FULL;
	else
		state = PART;
};

charptr_n::charptr_n(const charptr_n &copy)
{
	*this = copy;
};

charptr_n	&charptr_n::operator=(const charptr_n &assign)
{
	std::cout << "Estado de objeto a copiar: " << state << std::endl;
	if (state > EMPTY)
	{
		std::cout << "Ptr no está vacío, hacer deep copy" << std::endl;
		free(ptr);
		ptr = (char *)malloc(assign.size);
		for (size_t i = 0; i < assign.filled; i++)
			ptr[i] = assign.ptr[i];
	}
	else
		ptr = NULL;
	size = assign.size;
	filled = assign.filled;
	state = assign.state;
	return (*this);
};

charptr_n charptr_n::operator+(const charptr_n &add)
{
	int			sum_size = filled + add.filled;
	charptr_n	sumptr(sum_size, 0);
	
	size_t i = 0;
	for (size_t j = 0; j < filled; i++, j++) {	
		sumptr.ptr[i] = ptr[j]; }
	for (size_t j = 0; j < add.filled; i++, j++) {	
		sumptr.ptr[i] = add.ptr[j]; }
	sumptr.filled = i;
	sumptr.state = FULL;
	return (sumptr);
};

int	charptr_n::operator+=(const charptr_n &add)
{
	size_t	sum_size = filled + add.filled;
	char *sum = (char *)malloc(sum_size);
	if (!sum)
		return (0);
	size_t i = 0;
	for (size_t j = 0; j < filled; j++, i++) {	
		sum[i] = ptr[j]; }
	for (size_t j = 0; j < add.filled; j++, i++) {	
		sum[i] = add.ptr[j]; }
	if (state > EMPTY)
		free(ptr);
	ptr = sum;
	cursor = ptr;
	size = sum_size;
	filled = size;
	state = FULL;
	return (1);
};

charptr_n charptr_n::operator+(const char *add)
{
	size_t		add_len = strlen(add);
	size_t		sum_size = filled + add_len;
	charptr_n	sumptr(sum_size, 0);
	
	size_t i = 0;
	for (size_t j = 0; j < filled; i++, j++) {	
		sumptr.ptr[i] = ptr[j]; }
	for (size_t j = 0; j < add_len; i++, j++) {	
		sumptr.ptr[i] = add[j]; }
	sumptr.filled = i;
	sumptr.state = FULL;
	return (sumptr);
};

int	charptr_n::operator+=(const char *add)
{
	size_t	add_len = strlen(add);
	size_t	sum_size = filled + add_len;
	char 	*sum = (char *)malloc(sum_size);

	if (!sum)
		return (0);
	size_t i = 0;
	for (size_t j = 0; j < filled; j++, i++) {	
		sum[i] = ptr[j]; }
	for (size_t j = 0; j < add_len; j++, i++) {	
		sum[i] = add[j]; }
	if (state != EMPTY)
		free(ptr);
	ptr = sum;
	cursor = ptr;
	size = sum_size;
	filled = size;
	state = FULL;
	return (1);
};

charptr_n::~charptr_n()
{
	if (state != EMPTY)
		free(ptr);
	ptr = NULL;
	cursor = NULL;
	size = 0;
	filled = 0;
	state = EMPTY;
};

int operator<<(int fd, charptr_n &charptr)
{
	if (write(fd, charptr.ptr, charptr.filled) < 0)
		return (0);
	return (1);
}

charptr_n	subcharptr(charptr_n &charptr, size_t pos, size_t size)
{
	charptr_n	sub;

	sub.ptr = (char *)malloc(size);
	for (size_t i = 0; i < size; i++, pos++)
	{
		sub.ptr[i] = charptr.ptr[pos];
	}
	sub.size = size;
	sub.filled = size;
	sub.state = sub.FULL;
	return (sub);
}
