#include "visu.h"

/* 
** Modifie la couleur d'un pixel spécifique dans la mémoire du framebuffer (t_img).
** Gère les limites de l'image pour éviter les dépassements de mémoire.
*/
void	put_pixel(t_img *img, int x, int y, int color)
{
	char	*dst;

	if (x >= 0 && x < img->width && y >= 0 && y < img->height)
	{
		dst = img->addr + (y * img->line_length + x * (img->bits_per_pixel / 8));
		*(unsigned int *)dst = color;
	}
}

/*
** Trace une ligne de couleur sur le framebuffer en utilisant l'algorithme de Bresenham.
*/
void	draw_line(t_img *img, int x0, int y0, int x1, int y1, int color)
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

/* Police de caractères 8x12 matricielle faite à la main pour l'affichage des chiffres */
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

/*
** Dessine un unique chiffre matriciel à l'écran, mis à l'échelle (scale).
*/
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

/*
** Dessine un nombre entier à l'écran en découpant ses chiffres de gauche à droite.
*/
void	draw_large_number(t_img *img, int x_start, int y_start, int num, int scale, int color)
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

/*
** Dessine un tunnel reliant deux salles sous forme d'une ligne noire épaisse de 7 pixels.
*/
void	draw_tunnel(t_img *img, int x0, int y0, int x1, int y1)
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

/*
** Copie l'image de fond entière sur le framebuffer actif de rendu.
*/
void	draw_bg_to_frame(t_img *dest, t_img *src)
{
	int	y;
	int	x;

	y = 0;
	while (y < src->height)
	{
		x = 0;
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

/*
** Copie une image source (ex: un sprite) vers le framebuffer de destination,
** à des coordonnées spécifiques et avec une taille mise à l'échelle.
** Ignore la couleur blanche pure (0x00FFFFFF, 0xFF000000 ou noir pur selon cas)
** pour simuler la transparence du canal alpha.
*/
void	draw_img_to_img(t_img *dest, t_img *src, int dest_x, int dest_y, int target_w, int target_h)
{
	int	y;
	int	x;

	y = 0;
	while (y < target_h)
	{
		x = 0;
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

/*
** Charge un fichier image XPM avec la bibliothèque MiniLibX et extrait son adresse de données
** pour pouvoir modifier ou copier ses pixels individuellement.
*/
t_img	load_xpm_file(void *mlx, char *path)
{
	t_img	img;

	img.ptr = mlx_xpm_file_to_image(mlx, path, &img.width, &img.height);
	if (img.ptr)
		img.addr = mlx_get_data_addr(img.ptr, &img.bits_per_pixel, &img.line_length, &img.endian);
	else
		printf("Error: failed to load sprite %s\n", path);
	return (img);
}
