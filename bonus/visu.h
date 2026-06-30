#ifndef VISU_H
# define VISU_H

# pragma GCC diagnostic ignored "-Wcast-function-type"
# include "../lem_in.h"
# include <mlx.h>

/* Structures pour la gestion des mouvements */
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

/* Structures pour l'affichage graphique */
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

/* Variables globales exportées */
extern t_turn	*g_turns;
extern int		g_nb_turns;

/* Prototypes : Parsing et Simulation Interne */
int		parse_visualizer_input(t_lemin *lemin);
void	parse_moves(t_lemin *lemin);
void	generate_internal_turns(t_lemin *lemin);

/* Prototypes : Dessin et Framebuffer */
void	put_pixel(t_img *img, int x, int y, int color);
void	draw_line(t_img *img, int x0, int y0, int x1, int y1, int color);
void	draw_large_number(t_img *img, int x_start, int y_start, int num, int scale, int color);
void	draw_tunnel(t_img *img, int x0, int y0, int x1, int y1);
void	draw_bg_to_frame(t_img *dest, t_img *src);
void	draw_img_to_img(t_img *dest, t_img *src, int dest_x, int dest_y, int target_w, int target_h);
t_img	load_xpm_file(void *mlx, char *path);

/* Prototypes : Contrôle et Simulation */
void	trigger_next_turn(t_vars *vars);
void	reset_simulation(t_vars *vars);

#endif
