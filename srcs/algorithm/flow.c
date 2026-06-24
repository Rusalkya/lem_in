#include "../../lem_in.h"

typedef struct s_edge
{
	int	to;
	int	capacity;
	int	flow;
	int	rev_edge_idx;
}	t_edge;

typedef struct s_node
{
	t_edge	*edges;
	int		edge_count;
	int		edge_capacity;
}	t_node;

typedef struct s_flow_graph
{
	t_node	*nodes;
	int		nb_nodes;
}	t_flow_graph;

static int	add_directed_edge(t_flow_graph *fg, int from, int to, int capacity)
{
	t_node	*u = &fg->nodes[from];
	t_node	*v = &fg->nodes[to];

	if (u->edge_count >= u->edge_capacity)
	{
		int		new_cap = u->edge_capacity == 0 ? 4 : u->edge_capacity * 2;
		t_edge	*new_edges = realloc(u->edges, sizeof(t_edge) * new_cap);
		if (!new_edges)
			return (0);
		u->edges = new_edges;
		u->edge_capacity = new_cap;
	}
	if (v->edge_count >= v->edge_capacity)
	{
		int		new_cap = v->edge_capacity == 0 ? 4 : v->edge_capacity * 2;
		t_edge	*new_edges = realloc(v->edges, sizeof(t_edge) * new_cap);
		if (!new_edges)
			return (0);
		v->edges = new_edges;
		v->edge_capacity = new_cap;
	}

	u->edges[u->edge_count].to = to;
	u->edges[u->edge_count].capacity = capacity;
	u->edges[u->edge_count].flow = 0;
	u->edges[u->edge_count].rev_edge_idx = v->edge_count;

	v->edges[v->edge_count].to = from;
	v->edges[v->edge_count].capacity = 0;
	v->edges[v->edge_count].flow = 0;
	v->edges[v->edge_count].rev_edge_idx = u->edge_count;

	u->edge_count++;
	v->edge_count++;
	return (1);
}

void	free_flow_graph(t_flow_graph *fg)
{
	int	i;

	if (!fg)
		return ;
	if (fg->nodes)
	{
		i = 0;
		while (i < fg->nb_nodes)
		{
			if (fg->nodes[i].edges)
				free(fg->nodes[i].edges);
			i++;
		}
		free(fg->nodes);
	}
	free(fg);
}

t_flow_graph	*create_flow_graph(t_lemin *lemin)
{
	t_flow_graph	*fg;
	int				i;

	fg = malloc(sizeof(t_flow_graph));
	if (!fg)
		return (NULL);
	fg->nb_nodes = 2 * lemin->nb_rooms;
	fg->nodes = calloc(fg->nb_nodes, sizeof(t_node));
	if (!fg->nodes)
	{
		free(fg);
		return (NULL);
	}

	/* 1. Dédoubler les salles */
	i = 0;
	while (i < lemin->nb_rooms)
	{
		int	capacity = 1;
		if (i == lemin->start_id || i == lemin->end_id)
			capacity = lemin->nb_ants;
		if (!add_directed_edge(fg, 2 * i, 2 * i + 1, capacity))
		{
			free_flow_graph(fg);
			return (NULL);
		}
		i++;
	}

	/* 2. Relier les tunnels */
	i = 0;
	while (i < lemin->nb_links)
	{
		int	u = lemin->links[i].room1_id;
		int	v = lemin->links[i].room2_id;
		if (!add_directed_edge(fg, 2 * u + 1, 2 * v, 1) ||
			!add_directed_edge(fg, 2 * v + 1, 2 * u, 1))
		{
			free_flow_graph(fg);
			return (NULL);
		}
		i++;
	}
	return (fg);
}

int	find_augmenting_path(t_flow_graph *fg, int source, int sink, int *parent_node, int *parent_edge)
{
	int	*queue;
	int	*visited;
	int	front;
	int	rear;
	int	u;
	int	i;

	queue = malloc(sizeof(int) * fg->nb_nodes);
	visited = calloc(fg->nb_nodes, sizeof(int));
	if (!queue || !visited)
	{
		free(queue);
		free(visited);
		return (0);
	}
	front = 0;
	rear = 0;
	queue[rear++] = source;
	visited[source] = 1;

	while (front < rear)
	{
		u = queue[front++];
		if (u == sink)
		{
			free(queue);
			free(visited);
			return (1);
		}
		i = 0;
		while (i < fg->nodes[u].edge_count)
		{
			t_edge	*e = &fg->nodes[u].edges[i];
			int		v = e->to;
			int		residual = e->capacity - e->flow;
			if (!visited[v] && residual > 0)
			{
				visited[v] = 1;
				parent_node[v] = u;
				parent_edge[v] = i;
				queue[rear++] = v;
			}
			i++;
		}
	}
	free(queue);
	free(visited);
	return (0);
}

int	edmonds_karp(t_flow_graph *fg, int source, int sink)
{
	int	*parent_node;
	int	*parent_edge;
	int	max_flow_value;

	parent_node = malloc(sizeof(int) * fg->nb_nodes);
	parent_edge = malloc(sizeof(int) * fg->nb_nodes);
	if (!parent_node || !parent_edge)
	{
		free(parent_node);
		free(parent_edge);
		return (-1);
	}
	max_flow_value = 0;

	while (find_augmenting_path(fg, source, sink, parent_node, parent_edge))
	{
		int	path_flow = 1000000;
		int	v = sink;
		while (v != source)
		{
			int		u = parent_node[v];
			int		edge_idx = parent_edge[v];
			t_edge	*e = &fg->nodes[u].edges[edge_idx];
			int		residual = e->capacity - e->flow;
			if (residual < path_flow)
				path_flow = residual;
			v = u;
		}

		v = sink;
		while (v != source)
		{
			int		u = parent_node[v];
			int		edge_idx = parent_edge[v];
			t_edge	*e = &fg->nodes[u].edges[edge_idx];
			t_edge	*rev = &fg->nodes[v].edges[e->rev_edge_idx];
			e->flow += path_flow;
			rev->flow -= path_flow;
			v = u;
		}
		max_flow_value += path_flow;
	}

	free(parent_node);
	free(parent_edge);
	return (max_flow_value);
}

int	extract_paths(t_lemin *lemin, t_flow_graph *fg)
{
	int	*visited;
	int	path_count;
	int	start_out;
	int	i;

	visited = calloc(lemin->nb_rooms, sizeof(int));
	if (!visited)
		return (-1);
	lemin->paths = malloc(sizeof(t_path) * (lemin->nb_rooms + 1));
	if (!lemin->paths)
	{
		free(visited);
		return (-1);
	}
	path_count = 0;
	start_out = 2 * lemin->start_id + 1;

	i = 0;
	while (i < fg->nodes[start_out].edge_count)
	{
		t_edge	*e = &fg->nodes[start_out].edges[i];
		if (e->flow > 0)
		{
			int	*path_rooms = malloc(sizeof(int) * lemin->nb_rooms);
			if (!path_rooms)
			{
				free(visited);
				return (-1);
			}
			int	path_len = 0;
			path_rooms[path_len++] = lemin->start_id;

			int	current = e->to;
			int	current_room = current / 2;
			visited[lemin->start_id] = 1;

			while (current_room != lemin->end_id)
			{
				path_rooms[path_len++] = current_room;
				visited[current_room] = 1;

				int	out_node = current + 1;
				int	next_node = -1;
				int	j = 0;
				while (j < fg->nodes[out_node].edge_count)
				{
					t_edge	*e_out = &fg->nodes[out_node].edges[j];
					if (e_out->flow > 0)
					{
						next_node = e_out->to;
						break ;
					}
					j++;
				}
				if (next_node == -1)
					break ;
				current = next_node;
				current_room = current / 2;
			}

			if (current_room == lemin->end_id)
			{
				path_rooms[path_len++] = lemin->end_id;
				lemin->paths[path_count].rooms = path_rooms;
				lemin->paths[path_count].length = path_len;
				lemin->paths[path_count].flow = 1;
				path_count++;
			}
			else
			{
				free(path_rooms);
			}
		}
		i++;
	}

	lemin->nb_paths = path_count;
	free(visited);
	return (0);
}

int	compute_max_flow(t_lemin *lemin)
{
	t_flow_graph	*fg;
	int				max_flow;
	int				source;
	int				sink;

	fg = create_flow_graph(lemin);
	if (!fg)
		return (-1);
	source = 2 * lemin->start_id + 1;
	sink = 2 * lemin->end_id;

	max_flow = edmonds_karp(fg, source, sink);
	if (max_flow == -1)
	{
		free_flow_graph(fg);
		return (-1);
	}
	if (extract_paths(lemin, fg) == -1)
	{
		free_flow_graph(fg);
		return (-1);
	}
	free_flow_graph(fg);
	return (max_flow);
}

int	find_paths(t_lemin *lemin)
{
	(void)lemin;
	return (0);
}
