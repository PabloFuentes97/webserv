#include "unistd.h"
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

	charptr_n() : ptr(NULL), cursor(NULL), size(0), filled(0), state(EMPTY)
	{

	};

	charptr_n(size_t bytes, int flag) : ptr((char *)malloc(bytes)), cursor(NULL), size(bytes), filled(0), state(ALLOC)
	{
		if (flag == 1)
		{
			bzero(ptr, size);
			state = CALLOC;
		}
	};

	charptr_n(char *s, size_t len)
	{
		ptr = (char *)malloc(len);
		for (size_t	i = 0; i < len; i++)
			ptr[i] = s[i];
		size = len;
		filled = size;
		state = FULL;
	};

	charptr_n(const charptr_n &copy)
	{
		*this = copy;
	};

	charptr_n	&operator=(const charptr_n &assign)
	{
		if (state != EMPTY)
			free(ptr);
		ptr = (char *)malloc(assign.size);
		for (size_t i = 0; i < assign.filled; i++)
			ptr[i] = assign.ptr[i];
		size = assign.size;
		filled = assign.filled;
		state = assign.state;
		return (*this);
	};

	charptr_n operator+(const charptr_n &add)
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

	int	operator+=(const charptr_n &add)
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
		if (state != EMPTY)
			free(ptr);
		ptr = sum;
		cursor = ptr;
		size = sum_size;
		filled = size;
		state = FULL;
		return (1);
	};
	
	~charptr_n()
	{
		if (state != EMPTY)
			free(ptr);
		ptr = NULL;
		cursor = NULL;
		size = 0;
		filled = 0;
		state = EMPTY;
	};
} charptr_n;

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

/*int	main(void)
{
	//std::cout << strlen_unsafe("que pasa") << std::endl;
	//char	*substr = substr_unsafe("que p\0asa \0a\0 eee \0a", 12);
	//std::cout << substr << std::endl;
	charptr_n	str((char *)"h\0o\0l\0a", 8);
	std::cout << "Sizeof charptr_n: " << sizeof(charptr_n) << std::endl;
	1 << str;
	std::cout << std::endl;
	std::cout << " Contenido: " << str.ptr << " | Bytes alocados: " << str.size
			<< " | Bytes rellenados: " << str.filled << " | Estado: " << str.state;
	std::cout << std::endl;
	charptr_n	add((char *)"pepe", 4);
	1 << add;
	std::cout << std::endl;
	std::cout << " Contenido: " << add.ptr << " | Bytes alocados: " << add.size
			<< " | Bytes rellenados: " << add.filled << " | Estado: " << add.state;
	std::cout << std::endl;
	if ((str += add) == 0)
		std::cout << "Error al hacer join" << std::endl;
	1 << str;
	std::cout << std::endl;
	charptr_n	sum = str + add;
	1 << sum;
	std::cout << std::endl;
	charptr_n	add2((char *)"\0je\0je\0je", 10);
	sum += add2;
	1 << sum;
	std::cout << std::endl;
	charptr_n	sub = subcharptr(sum, 2, 5);
	1 << sub;
	std::cout << std::endl;
	system("leaks -q charptrpruebas");
}*/

int	search_str_charptr_pos(const charptr_n *haystack, const char *needle)
{
	for (size_t i = 0; i < haystack->filled; i++)
	{
		std::cout << "Caracter de haystack a buscar: " << haystack->ptr[i] << std::endl;
		if (haystack->ptr[i] == needle[0])
		{
			for (size_t j = 0; ; j++)
			{
				std::cout << "Caracter de needle a comparar: " << needle[j] << std::endl;
				std::cout << "Caracter de haystack a comparar: " << haystack->ptr[i + j] << std::endl;
				if (needle[j] == '\0')
					return (i);
				if (i + j > haystack->filled)
				{
					std::cout << "Needle se sale ya del haystack restante" << std::endl;
					return (-1);
				}
				if (needle[j] != haystack->ptr[i + j])
				{
					std::cout << "No coinciden" << std::endl;
					break ;
				}	
			}
		}
	}
	std::cout << "Llegó al final, no lo encuentra" << std::endl;
	return (-1);
}

int	main(void)
{
	charptr_n	e((char*)"egegegeasagege\r\n\r\n", 19);
	1 << e;
	std::cout << std::endl;
	int		lim = search_str_charptr_pos(&e, "\r\n\r\n");
	std::cout << lim << std::endl;
	if (lim != -1)
	{
		charptr_n	p(&e.ptr[lim], e.filled - lim);
		1 << p;
	}
}