#include "../srcs/lem_in.h"

typedef struct s_flow_graph
{
	int	**capacity;
	int	**flow;
	int	**residual;
	int	nb_nodes;
}	t_flow_graph;

/* Créer le graphe de flot à partir de lemin */
t_flow_graph	*create_flow_graph(t_lemin *lemin)
{
	t_flow_graph	*fg;
	int				i;
	int				j;

	fg = malloc(sizeof(t_flow_graph));
	if (!fg)
		return (NULL);
	
	fg->nb_nodes = lemin->nb_rooms;
	
	fg->capacity = malloc(sizeof(int *) * fg->nb_nodes);
	fg->flow = malloc(sizeof(int *) * fg->nb_nodes);
	fg->residual = malloc(sizeof(int *) * fg->nb_nodes);
	
	if (!fg->capacity || !fg->flow || !fg->residual)
		return (NULL);
	
	i = 0;
	while (i < fg->nb_nodes)
	{
		fg->capacity[i] = calloc(fg->nb_nodes, sizeof(int));
		fg->flow[i] = calloc(fg->nb_nodes, sizeof(int));
		fg->residual[i] = calloc(fg->nb_nodes, sizeof(int));
		i++;
	}
	
	/* Initialiser les capacités bidirectionnelles */
	i = 0;
	while (i < lemin->nb_links)
	{
		int	u = lemin->links[i].room1_id;
		int	v = lemin->links[i].room2_id;

		fg->capacity[u][v] = 1;
		fg->capacity[v][u] = 1;
		fg->residual[u][v] = 1;
		fg->residual[v][u] = 1;
		i++;
	}
	
	return (fg);
}

/* Trouver un chemin augmentant avec BFS */
int	find_augmenting_path(int **residual, int n, int source, int sink, int *parent)
{
	int	*queue;
	int	*visited;
	int	front;
	int	rear;
	int	u;
	int	i;

	queue = malloc(sizeof(int) * n);
	visited = calloc(n, sizeof(int));
	front = 0;
	rear = 0;
	
	if (!queue || !visited)
		return (0);
	
	queue[rear++] = source;
	visited[source] = 1;
	
	while (front < rear)
	{
		u = queue[front++];
		
		i = 0;
		while (i < n)
		{
			if (!visited[i] && residual[u][i] > 0)
			{
				visited[i] = 1;
				parent[i] = u;
				queue[rear++] = i;
				
				if (i == sink)
				{
					free(queue);
					free(visited);
					return (1);
				}
			}
			i++;
		}
	}
	
	free(queue);
	free(visited);
	return (0);
}

/* Calculer le flot maximum avec Edmonds-Karp */
int	edmonds_karp(t_flow_graph *fg, int source, int sink)
{
	int	*parent;
	int	max_flow_value;
	int	path_flow;
	int	u;
	int	v;
	int	i;

	parent = malloc(sizeof(int) * fg->nb_nodes);
	if (!parent)
		return (-1);
	
	max_flow_value = 0;
	
	while (find_augmenting_path(fg->residual, fg->nb_nodes, source, sink, parent))
	{
		/* Trouver la capacité minimale du chemin */
		path_flow = 10000;
		v = sink;
		while (v != source)
		{
			u = parent[v];
			if (fg->residual[u][v] < path_flow)
				path_flow = fg->residual[u][v];
			v = u;
		}
		
		/* Mettre à jour les capacités résiduelles */
		v = sink;
		while (v != source)
		{
			u = parent[v];
			fg->residual[u][v] -= path_flow;
			fg->residual[v][u] += path_flow;
			fg->flow[u][v] += path_flow;
			fg->flow[v][u] -= path_flow;
			v = u;
		}
		
		max_flow_value += path_flow;
	}
	
	free(parent);
	return (max_flow_value);
}

/* Libérer le graphe de flot */
void	free_flow_graph(t_flow_graph *fg)
{
	int	i;

	if (!fg)
		return ;
	
	if (fg->capacity)
	{
		i = 0;
		while (i < fg->nb_nodes)
		{
			free(fg->capacity[i]);
			i++;
		}
		free(fg->capacity);
	}
	
	if (fg->flow)
	{
		i = 0;
		while (i < fg->nb_nodes)
		{
			free(fg->flow[i]);
			i++;
		}
		free(fg->flow);
	}
	
	if (fg->residual)
	{
		i = 0;
		while (i < fg->nb_nodes)
		{
			free(fg->residual[i]);
			i++;
		}
		free(fg->residual);
	}
	
	free(fg);
}
