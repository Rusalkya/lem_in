#include "../lem_in.h"

#define BUFFER_SIZE 4096

static char	*get_line_from_buffer(char **buffer, int *buffer_len)
{
	char	*line;
	int		i;
	int		len;

	if (!buffer || !*buffer || *buffer_len <= 0)
		return (NULL);
	
	i = 0;
	while (i < *buffer_len && (*buffer)[i] != '\n')
		i++;
	
	len = i;
	line = malloc(len + 1);
	if (!line)
		return (NULL);
	
	strncpy(line, *buffer, len);
	line[len] = '\0';
	
	/* Décaler le buffer */
	if (i < *buffer_len && (*buffer)[i] == '\n')
		i++;
	
	if (i < *buffer_len)
	{
		memmove(*buffer, *buffer + i, *buffer_len - i);
		*buffer_len -= i;
	}
	else
		*buffer_len = 0;
	
	return (line);
}

static char	*g_buffer = NULL;
static int	g_buffer_len = 0;
static int	g_eof = 0;

char	*get_next_line(int fd)
{
	char	*line;
	char	read_buffer[BUFFER_SIZE];
	int		read_len;

	while (1)
	{
		/* Essayer d'obtenir une ligne du buffer actuel */
		line = get_line_from_buffer(&g_buffer, &g_buffer_len);
		if (line)
			return (line);
		
		/* Si EOF, retourner NULL */
		if (g_eof)
			return (NULL);
		
		/* Lire plus de données */
		read_len = read(fd, read_buffer, BUFFER_SIZE - 1);
		if (read_len <= 0)
		{
			g_eof = 1;
			/* Retourner ce qui reste dans le buffer */
			if (g_buffer && g_buffer_len > 0)
			{
				line = malloc(g_buffer_len + 1);
				if (line)
				{
					strncpy(line, g_buffer, g_buffer_len);
					line[g_buffer_len] = '\0';
					free(g_buffer);
					g_buffer = NULL;
					g_buffer_len = 0;
					return (line);
				}
			}
			return (NULL);
		}
		
		/* Augmenter le buffer */
		char *new_buffer = malloc(g_buffer_len + read_len);
		if (!new_buffer)
			return (NULL);
		
		if (g_buffer)
		{
			memcpy(new_buffer, g_buffer, g_buffer_len);
			free(g_buffer);
		}
		memcpy(new_buffer + g_buffer_len, read_buffer, read_len);
		
		g_buffer = new_buffer;
		g_buffer_len += read_len;
	}
}

char	**ft_split(char const *s, char c)
{
	char	**result;
	int		count;
	int		i;
	int		j;
	int		start;

	count = 0;
	i = 0;
	while (s && s[i])
	{
		if (s[i] != c && (i == 0 || s[i - 1] == c))
			count++;
		i++;
	}
	result = malloc(sizeof(char *) * (count + 1));
	if (!result)
		return (NULL);
	i = 0;
	j = 0;
	while (j < count)
	{
		while (s[i] == c)
			i++;
		start = i;
		while (s[i] && s[i] != c)
			i++;
		result[j] = malloc(sizeof(char) * (i - start + 1));
		if (!result[j])
			return (NULL);
		strncpy(result[j], s + start, i - start);
		result[j][i - start] = '\0';
		j++;
	}
	result[j] = NULL;
	return (result);
}

void	free_lemin(t_lemin *lemin)
{
	int	i;

	if (!lemin)
		return ;
	if (lemin->rooms)
	{
		i = 0;
		while (i < lemin->nb_rooms)
		{
			free(lemin->rooms[i].name);
			i++;
		}
		free(lemin->rooms);
	}
	if (lemin->links)
		free(lemin->links);
	if (lemin->adjacency)
	{
		i = 0;
		while (i < lemin->nb_rooms)
		{
			free(lemin->adjacency[i]);
			i++;
		}
		free(lemin->adjacency);
	}
	if (lemin->capacity)
	{
		i = 0;
		while (i < lemin->nb_rooms)
		{
			free(lemin->capacity[i]);
			i++;
		}
		free(lemin->capacity);
	}
	if (lemin->flow_graph)
	{
		i = 0;
		while (i < lemin->nb_rooms)
		{
			free(lemin->flow_graph[i]);
			i++;
		}
		free(lemin->flow_graph);
	}
	if (lemin->paths)
	{
		i = 0;
		while (i < lemin->nb_paths)
		{
			free(lemin->paths[i].rooms);
			i++;
		}
		free(lemin->paths);
	}
}

void	ft_putendl_fd(const char *s, int fd)
{
	if (s)
	{
		write(fd, s, strlen(s));
		write(fd, "\n", 1);
	}
}

/* Affiche le résultat */
void	print_farm(t_lemin *lemin)
{
	int	i;

	printf("%d\n", lemin->nb_ants);
	
	i = 0;
	while (i < lemin->nb_rooms)
	{
		if (lemin->rooms[i].type == START)
			printf("##start\n");
		else if (lemin->rooms[i].type == END)
			printf("##end\n");
		
		printf("%s %d %d\n", lemin->rooms[i].name, lemin->rooms[i].x, lemin->rooms[i].y);
		i++;
	}
	
	i = 0;
	while (i < lemin->nb_links)
	{
		printf("%s-%s\n", 
			lemin->rooms[lemin->links[i].room1_id].name,
			lemin->rooms[lemin->links[i].room2_id].name);
		i++;
	}
}