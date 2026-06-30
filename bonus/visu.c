#pragma GCC diagnostic ignored "-Wcast-function-type"
#include "../lem_in.h"
#include <mlx.h>

/* ================= PARSING AND MOVES STRUCTURES ================= */

typedef struct s_move_line
{
	char				*line;
	struct s_move_line	*next;
}	t_move_line;

typedef struct s_ant_move
{
	int	ant_id;
	int	room_id;
}	t_ant_move;

typedef struct s_turn
{
	t_ant_move	*moves;
	int			nb_moves;
}	t_turn;

static t_move_line	*g_moves = NULL;
static t_move_line	*g_moves_tail = NULL;
static t_turn		*g_turns = NULL;
static int			g_nb_turns = 0;

static void	add_move_line(const char *line)
{
	t_move_line	*node = malloc(sizeof(t_move_line));
	if (!node)
		return ;
	node->line = strdup(line);
	node->next = NULL;
	if (!g_moves)
	{
		g_moves = node;
		g_moves_tail = node;
	}
	else
	{
		g_moves_tail->next = node;
		g_moves_tail = node;
	}
}

static int	parse_visualizer_input(t_lemin *lemin)
{
	char		*line;
	int			ret;
	t_room_type	next_type = NORMAL;

	lemin->nb_ants = 0;
	lemin->nb_rooms = 0;
	lemin->nb_links = 0;
	lemin->rooms = NULL;
	lemin->links = NULL;
	lemin->start_id = -1;
	lemin->end_id = -1;
	lemin->adjacency = NULL;
	lemin->capacity = NULL;
	lemin->flow_graph = NULL;
	lemin->paths = NULL;
	lemin->nb_paths = 0;

	line = get_next_line(0);
	if (!line)
		return (-1);
	while (line && (strncmp(line, "DEBUG:", 6) == 0 || line[0] == '#'))
	{
		free(line);
		line = get_next_line(0);
	}
	if (!line || sscanf(line, "%d", &lemin->nb_ants) != 1 || lemin->nb_ants <= 0)
	{
		if (line)
			free(line);
		return (-1);
	}
	free(line);

	while ((line = get_next_line(0)) != NULL)
	{
		if (line[0] == '\0')
		{
			free(line);
			break ;
		}
		if (strncmp(line, "DEBUG:", 6) == 0)
		{
			free(line);
			continue ;
		}
		if (line[0] == 'L')
		{
			add_move_line(line);
			free(line);
			break ;
		}
		if (line[0] == '#')
		{
			if (strcmp(line, "##start") == 0)
				next_type = START;
			else if (strcmp(line, "##end") == 0)
				next_type = END;
			free(line);
			continue ;
		}
		if (strchr(line, '-'))
			ret = parse_link(lemin, line);
		else
		{
			ret = parse_room(lemin, line, next_type);
			next_type = NORMAL;
		}
		free(line);
		if (ret == -1)
			return (-1);
	}

	while ((line = get_next_line(0)) != NULL)
	{
		if (line[0] == 'L')
			add_move_line(line);
		free(line);
	}

	if (lemin->start_id == -1 || lemin->end_id == -1)
		return (-1);
	return (0);
}

static void	parse_moves(t_lemin *lemin)
{
	t_move_line	*curr = g_moves;
	int			count = 0;

	while (curr)
	{
		count++;
		curr = curr->next;
	}
	if (count == 0)
		return ;

	g_turns = malloc(sizeof(t_turn) * count);
	g_nb_turns = count;

	curr = g_moves;
	int turn_idx = 0;
	while (curr)
	{
		char	**split = ft_split(curr->line, ' ');
		int		moves_count = 0;
		while (split && split[moves_count])
			moves_count++;

		g_turns[turn_idx].nb_moves = moves_count;
		g_turns[turn_idx].moves = malloc(sizeof(t_ant_move) * moves_count);

		int i = 0;
		while (i < moves_count)
		{
			char	*p = split[i];
			if (p[0] == 'L')
			{
				int		ant_id = atoi(p + 1);
				char	*dash = strchr(p, '-');
				if (dash)
				{
					char	*room_name = dash + 1;
					int		room_id = find_room_by_name(lemin, room_name);
					g_turns[turn_idx].moves[i].ant_id = ant_id;
					g_turns[turn_idx].moves[i].room_id = room_id;
				}
			}
			free(split[i]);
			i++;
		}
		free(split);
		curr = curr->next;
		turn_idx++;
	}
}

static void	generate_internal_turns(t_lemin *lemin)
{
	if (compute_max_flow(lemin) == -1)
		return ;

	typedef struct s_temp_ant {
		int current_room;
		int path_idx;
		int pos_in_path;
	} t_temp_ant;

	t_temp_ant *ants = malloc(sizeof(t_temp_ant) * (lemin->nb_ants + 1));
	int *path_assignment = malloc(sizeof(int) * (lemin->nb_ants + 1));
	if (!ants || !path_assignment)
		return ;

	int i = 0;
	while (i <= lemin->nb_ants)
	{
		ants[i].current_room = lemin->start_id;
		ants[i].path_idx = -1;
		ants[i].pos_in_path = 0;
		path_assignment[i] = -1;
		i++;
	}

	int *path_ants_count = calloc(lemin->nb_paths, sizeof(int));
	if (!path_ants_count)
	{
		free(ants);
		free(path_assignment);
		return ;
	}
	i = 1;
	while (i <= lemin->nb_ants)
	{
		if (lemin->nb_paths > 0)
		{
			int best_path = 0;
			int min_score = (lemin->paths[0].length - 1) + path_ants_count[0];
			int p = 1;
			while (p < lemin->nb_paths)
			{
				int score = (lemin->paths[p].length - 1) + path_ants_count[p];
				if (score < min_score)
				{
					min_score = score;
					best_path = p;
				}
				p++;
			}
			path_assignment[i] = best_path;
			path_ants_count[best_path]++;
		}
		i++;
	}
	free(path_ants_count);

	int ants_finished = 0;
	int turn_idx = 0;
	int max_turns = 1000;

	typedef struct s_turn_list {
		t_ant_move *moves;
		int nb_moves;
		struct s_turn_list *next;
	} t_turn_list;

	t_turn_list *head = NULL;
	t_turn_list *tail = NULL;
	int total_turns = 0;

	while (ants_finished < lemin->nb_ants && turn_idx < max_turns)
	{
		t_ant_move *temp_moves = malloc(sizeof(t_ant_move) * lemin->nb_ants);
		int moves_count = 0;

		i = 1;
		while (i <= lemin->nb_ants)
		{
			if (ants[i].current_room != lemin->end_id)
			{
				int path_idx = path_assignment[i];
				if (path_idx >= 0 && path_idx < lemin->nb_paths)
				{
					int next_room = lemin->paths[path_idx].rooms[ants[i].pos_in_path + 1];
					int can_move = 1;
					int j = 1;
					while (j <= lemin->nb_ants)
					{
						if (i != j && ants[j].current_room == next_room)
						{
							can_move = 0;
							break ;
						}
						j++;
					}
					if ((next_room == lemin->start_id || next_room == lemin->end_id) && 
						ants[i].pos_in_path + 1 > 0)
					{
						can_move = 1;
					}
					if (can_move)
					{
						ants[i].current_room = next_room;
						ants[i].pos_in_path++;
						if (ants[i].current_room == lemin->end_id)
							ants_finished++;
						temp_moves[moves_count].ant_id = i;
						temp_moves[moves_count].room_id = next_room;
						moves_count++;
					}
				}
			}
			i++;
		}

		if (moves_count > 0)
		{
			t_turn_list *node = malloc(sizeof(t_turn_list));
			node->nb_moves = moves_count;
			node->moves = malloc(sizeof(t_ant_move) * moves_count);
			memcpy(node->moves, temp_moves, sizeof(t_ant_move) * moves_count);
			node->next = NULL;
			if (!head)
			{
				head = node;
				tail = node;
			}
			else
			{
				tail->next = node;
				tail = node;
			}
			total_turns++;
		}
		free(temp_moves);
		turn_idx++;
	}

	free(ants);
	free(path_assignment);

	if (total_turns > 0)
	{
		g_turns = malloc(sizeof(t_turn) * total_turns);
		g_nb_turns = total_turns;
		t_turn_list *curr = head;
		int idx = 0;
		while (curr)
		{
			g_turns[idx].nb_moves = curr->nb_moves;
			g_turns[idx].moves = curr->moves;
			t_turn_list *next = curr->next;
			free(curr);
			curr = next;
			idx++;
		}
	}
}

/* ================= GRAPHICS FUNCTIONS ================= */

typedef struct s_img
{
	void	*ptr;
	char	*addr;
	int		bits_per_pixel;
	int		line_length;
	int		endian;
	int		width;
	int		height;
}	t_img;

typedef struct s_ant_display
{
	int		id;
	int		current_room;
	int		target_room;
	double	x;
	double	y;
	int		is_moving;
	int		move_progress;
}	t_ant_display;

typedef struct s_vars
{
	void			*mlx;
	void			*win;
	t_lemin			lemin;
	int				*room_x;
	int				*room_y;
	t_img			frame;
	t_img			background_img;
	t_img			ant_walk_imgs[4];
	t_img			start_empty_img;
	t_img			start_ant_img;
	t_img			end_empty_img;
	t_img			end_ant_img;
	t_img			room_empty_img;
	t_img			room_ant_img;
	t_ant_display	*ants;
	int				current_turn;
	int				is_animating;
	int				autoplay;
	int				autoplay_delay;
	int				bg_width;
	int				bg_height;
}	t_vars;

static void	put_pixel(t_img *img, int x, int y, int color)
{
	if (x >= 0 && x < img->width && y >= 0 && y < img->height)
	{
		char	*dst = img->addr + (y * img->line_length + x * (img->bits_per_pixel / 8));
		*(unsigned int *)dst = color;
	}
}

static void	draw_line(t_img *img, int x0, int y0, int x1, int y1, int color)
{
	int	dx = abs(x1 - x0);
	int	dy = abs(y1 - y0);
	int	sx = x0 < x1 ? 1 : -1;
	int	sy = y0 < y1 ? 1 : -1;
	int	err = dx - dy;

	while (1)
	{
		put_pixel(img, x0, y0, color);
		if (x0 == x1 && y0 == y1)
			break ;
		int	e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

static const unsigned short g_font_digits[10][12] = {
	{0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C},
	{0x18, 0x38, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E},
	{0x3C, 0x66, 0xC3, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0xC3, 0xFF},
	{0x3C, 0x66, 0xC3, 0x03, 0x03, 0x1E, 0x03, 0x03, 0x03, 0xC3, 0x66, 0x3C},
	{0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0x06, 0x06, 0x06, 0x06, 0x0F},
	{0xFF, 0xC0, 0xC0, 0xC0, 0xFC, 0x06, 0x03, 0x03, 0x03, 0xC3, 0x66, 0x3C},
	{0x3C, 0x66, 0xC0, 0xC0, 0xFC, 0xC6, 0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C},
	{0xFF, 0xC3, 0x03, 0x06, 0x06, 0x0C, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x18},
	{0x3C, 0x66, 0xC3, 0xC3, 0x66, 0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0x66, 0x3C},
	{0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0x63, 0x3F, 0x03, 0x03, 0x06, 0x66, 0x3C}
};

static void	draw_digit(t_img *img, int x_start, int y_start, int digit, int scale, int color)
{
	int	y;
	int	x;
	int	sy;
	int	sx;

	if (digit < 0 || digit > 9)
		return ;
	y = 0;
	while (y < 12)
	{
		unsigned short	row = g_font_digits[digit][y];
		x = 0;
		while (x < 8)
		{
			if ((row >> (7 - x)) & 1)
			{
				sy = 0;
				while (sy < scale)
				{
					sx = 0;
					while (sx < scale)
					{
						put_pixel(img, x_start + x * scale + sx, y_start + y * scale + sy, color);
						sx++;
					}
					sy++;
				}
			}
			x++;
		}
		y++;
	}
}

static void	draw_large_number(t_img *img, int x_start, int y_start, int num, int scale, int color)
{
	char	str[32];
	int		i;

	sprintf(str, "%d", num);
	i = 0;
	while (str[i])
	{
		draw_digit(img, x_start + i * 9 * scale, y_start, str[i] - '0', scale, color);
		i++;
	}
}

static void	draw_tunnel(t_img *img, int x0, int y0, int x1, int y1)
{
	int	offset;

	offset = -3;
	while (offset <= 3)
	{
		draw_line(img, x0 + offset, y0, x1 + offset, y1, 0x000000);
		draw_line(img, x0, y0 + offset, x1, y1 + offset, 0x000000);
		offset++;
	}
}

static void	draw_bg_to_frame(t_img *dest, t_img *src)
{
	int	y = 0;
	while (y < src->height)
	{
		int x = 0;
		while (x < src->width)
		{
			char			*src_pixel = src->addr + (y * src->line_length + x * (src->bits_per_pixel / 8));
			unsigned int	color = *(unsigned int *)src_pixel;
			put_pixel(dest, x, y, color);
			x++;
		}
		y++;
	}
}

static void	draw_img_to_img(t_img *dest, t_img *src, int dest_x, int dest_y, int target_w, int target_h)
{
	int	y = 0;
	while (y < target_h)
	{
		int x = 0;
		while (x < target_w)
		{
			int	src_x = x * src->width / target_w;
			int	src_y = y * src->height / target_h;
			int	dx = dest_x + x;
			int	dy = dest_y + y;
			if (dx >= 0 && dx < dest->width && dy >= 0 && dy < dest->height)
			{
				char			*src_pixel = src->addr + (src_y * src->line_length + src_x * (src->bits_per_pixel / 8));
				unsigned int	color = *(unsigned int *)src_pixel;
				if ((color & 0x00FFFFFF) != 0x00FFFFFF && color != 0x000000 && color != 0xFF000000)
					put_pixel(dest, dx, dy, color);
			}
			x++;
		}
		y++;
	}
}

static t_img	load_xpm_file(void *mlx, char *path)
{
	t_img	img;

	img.ptr = mlx_xpm_file_to_image(mlx, path, &img.width, &img.height);
	if (img.ptr)
		img.addr = mlx_get_data_addr(img.ptr, &img.bits_per_pixel, &img.line_length, &img.endian);
	else
		printf("Error: failed to load sprite %s\n", path);
	return (img);
}

static void	trigger_next_turn(t_vars *vars)
{
	if (vars->current_turn >= g_nb_turns)
		return ;

	t_turn	*turn = &g_turns[vars->current_turn];
	int		i = 0;
	while (i < turn->nb_moves)
	{
		int	ant_id = turn->moves[i].ant_id;
		int	dest_room = turn->moves[i].room_id;
		if (ant_id > 0 && ant_id <= vars->lemin.nb_ants)
		{
			vars->ants[ant_id].target_room = dest_room;
			vars->ants[ant_id].is_moving = 1;
			vars->ants[ant_id].move_progress = 0;
		}
		i++;
	}
	vars->current_turn++;
	vars->is_animating = 1;
}

static void	reset_simulation(t_vars *vars)
{
	vars->current_turn = 0;
	vars->is_animating = 0;
	int	i = 1;
	while (i <= vars->lemin.nb_ants)
	{
		vars->ants[i].current_room = vars->lemin.start_id;
		vars->ants[i].target_room = vars->lemin.start_id;
		vars->ants[i].x = vars->room_x[vars->lemin.start_id];
		vars->ants[i].y = vars->room_y[vars->lemin.start_id];
		vars->ants[i].is_moving = 0;
		vars->ants[i].move_progress = 0;
		i++;
	}
}

static int	update_and_render(t_vars *vars)
{
	int	still_animating = 0;
	int	i = 1;

	/* 1. Update Ant Positions */
	while (i <= vars->lemin.nb_ants)
	{
		if (vars->ants[i].is_moving)
		{
			vars->ants[i].move_progress += 4;
			if (vars->ants[i].move_progress >= 100)
			{
				vars->ants[i].move_progress = 100;
				vars->ants[i].current_room = vars->ants[i].target_room;
				vars->ants[i].x = vars->room_x[vars->ants[i].current_room];
				vars->ants[i].y = vars->room_y[vars->ants[i].current_room];
				vars->ants[i].is_moving = 0;
			}
			else
			{
				double	t = (double)vars->ants[i].move_progress / 100.0;
				int		r_start = vars->ants[i].current_room;
				int		r_end = vars->ants[i].target_room;
				vars->ants[i].x = vars->room_x[r_start] + (vars->room_x[r_end] - vars->room_x[r_start]) * t;
				vars->ants[i].y = vars->room_y[r_start] + (vars->room_y[r_end] - vars->room_y[r_start]) * t;
				still_animating = 1;
			}
		}
		i++;
	}
	vars->is_animating = still_animating;

	/* 2. Autoplay Trigger */
	if (!vars->is_animating && vars->autoplay)
	{
		if (vars->current_turn < g_nb_turns)
		{
			if (vars->autoplay_delay > 25)
			{
				trigger_next_turn(vars);
				vars->autoplay_delay = 0;
			}
			else
				vars->autoplay_delay++;
		}
	}

	/* 3. Render Canvas */
	draw_bg_to_frame(&vars->frame, &vars->background_img);

	/* Draw tunnels (thick black lines) */
	i = 0;
	while (i < vars->lemin.nb_links)
	{
		int	u = vars->lemin.links[i].room1_id;
		int	v = vars->lemin.links[i].room2_id;
		draw_tunnel(&vars->frame, vars->room_x[u], vars->room_y[u], vars->room_x[v], vars->room_y[v]);
		i++;
	}

	/* Draw chambers/rooms (centered at rx - 60, ry - 60, scaled to 120x120) */
	i = 0;
	while (i < vars->lemin.nb_rooms)
	{
		int	rx = vars->room_x[i];
		int	ry = vars->room_y[i];
		if (i == vars->lemin.start_id)
		{
			int has_ant = 0;
			int j = 1;
			while (j <= vars->lemin.nb_ants)
			{
				if (!vars->ants[j].is_moving && vars->ants[j].current_room == vars->lemin.start_id)
				{
					has_ant = 1;
					break ;
				}
				j++;
			}
			if (has_ant)
				draw_img_to_img(&vars->frame, &vars->start_ant_img, rx - 60, ry - 60, 120, 120);
			else
				draw_img_to_img(&vars->frame, &vars->start_empty_img, rx - 60, ry - 60, 120, 120);
		}
		else if (i == vars->lemin.end_id)
		{
			int has_ant = 0;
			int j = 1;
			while (j <= vars->lemin.nb_ants)
			{
				if (!vars->ants[j].is_moving && vars->ants[j].current_room == vars->lemin.end_id)
				{
					has_ant = 1;
					break ;
				}
				j++;
			}
			if (has_ant)
				draw_img_to_img(&vars->frame, &vars->end_ant_img, rx - 60, ry - 60, 120, 120);
			else
				draw_img_to_img(&vars->frame, &vars->end_empty_img, rx - 60, ry - 60, 120, 120);
		}
		else
		{
			int has_ant = 0;
			int j = 1;
			while (j <= vars->lemin.nb_ants)
			{
				if (!vars->ants[j].is_moving && vars->ants[j].current_room == i)
				{
					has_ant = 1;
					break ;
				}
				j++;
			}
			if (has_ant)
				draw_img_to_img(&vars->frame, &vars->room_ant_img, rx - 60, ry - 60, 120, 120);
			else
				draw_img_to_img(&vars->frame, &vars->room_empty_img, rx - 60, ry - 60, 120, 120);
		}
		i++;
	}

	/* Draw moving ants (animated walk, centered at x - 50, y - 50, scaled to 100x100) */
	i = 1;
	while (i <= vars->lemin.nb_ants)
	{
		if (vars->ants[i].is_moving)
		{
			int frame_idx = (vars->ants[i].move_progress / 5) % 4;
			draw_img_to_img(&vars->frame, &vars->ant_walk_imgs[frame_idx], (int)vars->ants[i].x - 50, (int)vars->ants[i].y - 50, 100, 100);
		}
		i++;
	}

	/* 4. Statistics calculations */
	int	ants_at_spawn = 0;
	int	ants_in_transit = 0;
	int	ants_at_end = 0;

	i = 1;
	while (i <= vars->lemin.nb_ants)
	{
		if (vars->ants[i].is_moving)
			ants_in_transit++;
		else if (vars->ants[i].current_room == vars->lemin.start_id)
			ants_at_spawn++;
		else if (vars->ants[i].current_room == vars->lemin.end_id)
			ants_at_end++;
		i++;
	}

	/* Render ONLY the ant numbers in the top left, drawn directly to the frame, shifted right and color-coded */
	draw_large_number(&vars->frame, 400, 20, vars->lemin.nb_ants, 2, 0xFF8C00);      // Orange (Total)
	draw_large_number(&vars->frame, 400, 50, ants_at_spawn, 2, 0x2ECC71);       // Green (Spawn)
	draw_large_number(&vars->frame, 400, 80, ants_in_transit, 2, 0x8B4513);     // Brown (Transit)
	draw_large_number(&vars->frame, 400, 110, ants_at_end, 2, 0xE74C3C);         // Red (End)

	/* Render ONLY the turn count in the top right, drawn directly to the frame, larger (scale 4) */
	draw_large_number(&vars->frame, vars->bg_width / 2 + 80, 40, vars->current_turn, 4, 0xFFFFFF); // White (scale 4)

	/* Push framebuffer to window */
	mlx_put_image_to_window(vars->mlx, vars->win, vars->frame.ptr, 0, 0);

	/* Render Room Labels under rooms (Color-coded) */
	i = 0;
	while (i < vars->lemin.nb_rooms)
	{
		int	rx = vars->room_x[i];
		int	ry = vars->room_y[i];
		int	color = 0x000000; // Always black labels to fit background nicely
		mlx_string_put(vars->mlx, vars->win, rx - 15, ry + 75, color, vars->lemin.rooms[i].name);
		i++;
	}
	return (0);
}

static int	handle_keypress(int keycode, t_vars *vars)
{
	if (keycode == 65307 || keycode == 53 || keycode == 9)
	{
		mlx_destroy_window(vars->mlx, vars->win);
		exit(0);
	}
	else if (keycode == 32)
	{
		if (!vars->is_animating && vars->current_turn < g_nb_turns)
			trigger_next_turn(vars);
	}
	else if (keycode == 114 || keycode == 82)
		reset_simulation(vars);
	else if (keycode == 97 || keycode == 65)
		vars->autoplay = !vars->autoplay;
	return (0);
}

static int	handle_close(t_vars *vars)
{
	(void)vars;
	exit(0);
}

int	main(void)
{
	t_vars	vars;

	if (parse_visualizer_input(&vars.lemin) == -1)
	{
		free_lemin(&vars.lemin);
		ft_putendl_fd("ERROR", 2);
		return (1);
	}

	parse_moves(&vars.lemin);
	if (g_nb_turns == 0)
	{
		generate_internal_turns(&vars.lemin);
	}

	vars.mlx = mlx_init();
	if (!vars.mlx)
	{
		free_lemin(&vars.lemin);
		return (1);
	}

	vars.background_img = load_xpm_file(vars.mlx, "bonus/sprites/background.xpm");
	if (!vars.background_img.ptr)
	{
		ft_putendl_fd("Error: background sprite not found.", 2);
		free_lemin(&vars.lemin);
		return (1);
	}
	vars.bg_width = vars.background_img.width;
	vars.bg_height = vars.background_img.height;

	vars.win = mlx_new_window(vars.mlx, vars.bg_width, vars.bg_height, "Lem-in Visualizer");
	if (!vars.win)
	{
		free_lemin(&vars.lemin);
		return (1);
	}

	vars.ant_walk_imgs[0] = load_xpm_file(vars.mlx, "bonus/sprites/ant1.xpm");
	vars.ant_walk_imgs[1] = load_xpm_file(vars.mlx, "bonus/sprites/ant2.xpm");
	vars.ant_walk_imgs[2] = load_xpm_file(vars.mlx, "bonus/sprites/ant3.xpm");
	vars.ant_walk_imgs[3] = load_xpm_file(vars.mlx, "bonus/sprites/ant4.xpm");
	vars.start_empty_img = load_xpm_file(vars.mlx, "bonus/sprites/start_empty.xpm");
	vars.start_ant_img = load_xpm_file(vars.mlx, "bonus/sprites/start_ant.xpm");
	vars.end_empty_img = load_xpm_file(vars.mlx, "bonus/sprites/end_empty.xpm");
	vars.end_ant_img = load_xpm_file(vars.mlx, "bonus/sprites/end_ant.xpm");
	vars.room_empty_img = load_xpm_file(vars.mlx, "bonus/sprites/room_empty.xpm");
	vars.room_ant_img = load_xpm_file(vars.mlx, "bonus/sprites/room_ant.xpm");

	vars.frame.ptr = mlx_new_image(vars.mlx, vars.bg_width, vars.bg_height);
	vars.frame.addr = mlx_get_data_addr(vars.frame.ptr, &vars.frame.bits_per_pixel, &vars.frame.line_length, &vars.frame.endian);
	vars.frame.width = vars.bg_width;
	vars.frame.height = vars.bg_height;

	/* Calculate scaling margins fit to background size */
	vars.room_x = malloc(sizeof(int) * vars.lemin.nb_rooms);
	vars.room_y = malloc(sizeof(int) * vars.lemin.nb_rooms);
	if (vars.lemin.nb_rooms > 0)
	{
		int	min_x = vars.lemin.rooms[0].x;
		int	max_x = vars.lemin.rooms[0].x;
		int	min_y = vars.lemin.rooms[0].y;
		int	max_y = vars.lemin.rooms[0].y;

		int i = 1;
		while (i < vars.lemin.nb_rooms)
		{
			if (vars.lemin.rooms[i].x < min_x)
				min_x = vars.lemin.rooms[i].x;
			if (vars.lemin.rooms[i].x > max_x)
				max_x = vars.lemin.rooms[i].x;
			if (vars.lemin.rooms[i].y < min_y)
				min_y = vars.lemin.rooms[i].y;
			if (vars.lemin.rooms[i].y > max_y)
				max_y = vars.lemin.rooms[i].y;
			i++;
		}

		i = 0;
		while (i < vars.lemin.nb_rooms)
		{
			int margin_x = 150;
			int margin_y_top = 300;
			int margin_y_bottom = 150;
			int width_range = vars.bg_width - 2 * margin_x;
			int height_range = vars.bg_height - margin_y_top - margin_y_bottom;

			if (max_x - min_x == 0)
				vars.room_x[i] = vars.bg_width / 2;
			else
				vars.room_x[i] = margin_x + (vars.lemin.rooms[i].x - min_x) * width_range / (max_x - min_x);

			if (max_y - min_y == 0)
				vars.room_y[i] = margin_y_top + height_range / 2;
			else
				vars.room_y[i] = margin_y_top + (vars.lemin.rooms[i].y - min_y) * height_range / (max_y - min_y);
			i++;
		}
	}

	/* Initialize ants positions */
	vars.ants = malloc(sizeof(t_ant_display) * (vars.lemin.nb_ants + 1));
	int i = 1;
	while (i <= vars.lemin.nb_ants)
	{
		vars.ants[i].id = i;
		vars.ants[i].current_room = vars.lemin.start_id;
		vars.ants[i].target_room = vars.lemin.start_id;
		vars.ants[i].x = vars.room_x[vars.lemin.start_id];
		vars.ants[i].y = vars.room_y[vars.lemin.start_id];
		vars.ants[i].is_moving = 0;
		vars.ants[i].move_progress = 0;
		i++;
	}

	vars.current_turn = 0;
	vars.is_animating = 0;
	vars.autoplay = 0;
	vars.autoplay_delay = 0;

	mlx_hook(vars.win, 2, 1L << 0, (int (*)())handle_keypress, &vars);
	mlx_hook(vars.win, 17, 0, (int (*)())handle_close, &vars);
	mlx_loop_hook(vars.mlx, (int (*)())update_and_render, &vars);
	mlx_loop(vars.mlx);

	free(vars.room_x);
	free(vars.room_y);
	free(vars.ants);
	free_lemin(&vars.lemin);
	return (0);
}
