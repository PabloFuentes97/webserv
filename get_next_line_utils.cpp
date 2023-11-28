/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pfuentes <pfuentes@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/09 13:03:09 by pfuentes          #+#    #+#             */
/*   Updated: 2023/11/27 11:40:51 by pfuentes         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.hpp"

size_t	ft_strlen(const char *str)
{
	int	cont;

	cont = 0;
	while (str[cont] != '\0')
		cont++;
	return (cont);
}

void	*ft_calloc(size_t count, size_t size)
{
	size_t	cont;
	char	*array;

	if (size * count < count && (size * count) < size)
		return (NULL);
	array = (char *)malloc(size * count);
	cont = 0;
	if (!array)
		return (NULL);
	while (cont < (count * size))
	{
		array[cont] = 0;
		cont++;
	}
	return (array);
}

char	*ft_strdup(const char *s1)
{
	int		cont;
	char	*copy;

	cont = 0;
	copy = (char *)malloc(sizeof(char) * ft_strlen(s1) + 1);
	if (copy == NULL)
		return (NULL);
	while (s1[cont] != '\0')
	{
		copy[cont] = s1[cont];
		cont++;
	}
	copy[cont] = '\0';
	return (copy);
}

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	size_t	cont;
	char	*substring;

	cont = 0;
	if (!s || start >= ft_strlen(s))
		return ((char *)ft_calloc(1, 1));
	if (len > (ft_strlen(s) - start))
		substring = (char *)malloc(sizeof(char) * (ft_strlen(s) - start + 1));
	else
		substring = (char *)malloc(sizeof(char) * (len + 1));
	if (substring == NULL)
		return (NULL);
	while (cont < len && s[start])
	{
		substring[cont] = s[start];
		cont++;
		start++;
	}
	substring[cont] = '\0';
	return (substring);
}

char	*ft_strjoin(char const *s1, char const *s2)
{
	char	*array;
	int		cont_s;
	int		cont_join;

	if (!s1 || !s2)
		return (0);
	array = (char *)malloc(sizeof(char) * (ft_strlen(s1) + ft_strlen(s2) + 1));
	if (array == NULL)
		return (NULL);
	cont_s = 0;
	cont_join = 0;
	while (s1[cont_join] != '\0')
	{
		array[cont_join] = s1[cont_join];
		cont_join++;
	}
	while (s2[cont_s] != '\0')
	{
		array[cont_join] = s2[cont_s];
		cont_join++;
		cont_s++;
	}
	array[cont_join] = '\0';
	return (array);
}
