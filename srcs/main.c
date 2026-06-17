#include "srcs/lem_in.h"

/* Initialise une matrice d'adjacence pour le graphe */
static int	init_adjacency_matrix(t_lemin *lemin)
{
	int	i;
	int	j;

	lemin->adjacency = malloc(sizeof(int *) * lemin->nb_rooms);
	if (!lemin->adjacency)
		return (-1);
	
	i = 0;
	while (i < lemin->nb_rooms)
	{
		lemin->adjacency[i] = malloc(sizeof(int) * lemin->nb_rooms);
		if (!lemin->adjacency[i])
			return (-1);
		j = 0;
		while (j < lemin->nb_rooms)
		{
			lemin->adjacency[i][j] = 0;
			j++;
		}
		i++;
	}
	
	i = 0;
	while (i < lemin->nb_links)
	{
		lemin->adjacency[lemin->links[i].room1_id][lemin->links[i].room2_id] = 1;
		lemin->adjacency[lemin->links[i].room2_id][lemin->links[i].room1_id] = 1;
		i++;
	}
	
	return (0);
}

/* Affiche le résultat */
static void	print_farm(t_lemin *lemin)
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

int	main(void)
{
	t_lemin	lemin;

	if (parse_input(&lemin) == -1)
	{
		free_lemin(&lemin);
		return (1);
	}
	
	if (init_adjacency_matrix(&lemin) == -1)
	{
		free_lemin(&lemin);
		ft_putendl_fd("ERROR", 2);
		return (1);
	}
	
	print_farm(&lemin);
	
	free_lemin(&lemin);
	return (0);
}
