NAME		= lem-in
VISU_NAME	= visu-hex
SRCDIR		= srcs
OBJDIR		= obj
CC			= gcc
CFLAGS		= -Wall -Wextra -Werror -I. -Iminilibx-linux

SOURCES		= srcs/main.c \
			  srcs/parsing.c \
			  srcs/utils.c \
			  srcs/algorithm/flow.c \
			  srcs/algorithm/simulate.c

VISU_SOURCES = bonus/visu.c

OBJECTS		= $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.c=.o)))
VISU_OBJECTS = $(addprefix $(OBJDIR)/, $(notdir $(VISU_SOURCES:.c=.o)))

all: $(NAME)

bonus: $(NAME) $(VISU_NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJECTS)

$(VISU_NAME): $(VISU_OBJECTS) $(filter-out $(OBJDIR)/main.o,$(OBJECTS))
	$(CC) $(CFLAGS) -o $(VISU_NAME) $(VISU_OBJECTS) $(filter-out $(OBJDIR)/main.o,$(OBJECTS)) -Lminilibx-linux -lmlx -lXext -lX11 -lm

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

fclean: clean
	rm -f $(NAME)
	rm -f $(VISU_NAME)

re: fclean all

.PHONY: all bonus clean fclean re
