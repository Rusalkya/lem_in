NAME		= lem-in
VISU_NAME	= visu-hex
SRCDIR		= srcs
OBJDIR		= obj
MINILIBX_DIR = minilibx-linux
MINILIBX     = $(MINILIBX_DIR)/libmlx.a

CC			= gcc
CFLAGS		= -Wall -Wextra -Werror -I. -I$(MINILIBX_DIR)

SOURCES		= srcs/main.c \
			  srcs/parsing.c \
			  srcs/utils.c \
			  srcs/algorithm/flow.c \
			  srcs/algorithm/simulate.c

VISU_SOURCES = bonus/visu_parsing.c \
			   bonus/visu_draw.c \
			   bonus/visu_main.c

OBJECTS		= $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.c=.o)))
VISU_OBJECTS = $(addprefix $(OBJDIR)/, $(notdir $(VISU_SOURCES:.c=.o)))

all: $(NAME)

$(MINILIBX):
	@if [ ! -d "$(MINILIBX_DIR)" ]; then \
		echo "Cloning minilibx-linux..."; \
		git clone https://github.com/42Paris/minilibx-linux.git $(MINILIBX_DIR); \
	fi
	@echo "Building minilibx-linux..."
	@make -C $(MINILIBX_DIR)

bonus: $(NAME) $(MINILIBX) $(VISU_NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJECTS)

$(VISU_OBJECTS): | $(MINILIBX)

$(VISU_NAME): $(MINILIBX) $(VISU_OBJECTS) $(filter-out $(OBJDIR)/main.o,$(OBJECTS))
	$(CC) $(CFLAGS) -o $(VISU_NAME) $(VISU_OBJECTS) $(filter-out $(OBJDIR)/main.o,$(OBJECTS)) -L$(MINILIBX_DIR) -lmlx -lXext -lX11 -lm

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: srcs/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: srcs/algorithm/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: bonus/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
	@if [ -d "$(MINILIBX_DIR)" ]; then \
		make clean -C $(MINILIBX_DIR); \
	fi

fclean: clean
	rm -f $(NAME)
	rm -f $(VISU_NAME)
	rm -rf $(MINILIBX_DIR)

re: fclean all

.PHONY: all bonus clean fclean re
