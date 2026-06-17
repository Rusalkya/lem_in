NAME		= lem-in
SRCDIR		= srcs
OBJDIR		= obj
CC			= gcc
CFLAGS		= -Wall -Wextra -Werror -I.

SOURCES		= srcs/main.c \
			  srcs/parsing/parsing.c \
			  srcs/algorithm/flow.c

OBJECTS		= $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.c=.o)))

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJECTS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: srcs/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: srcs/parsing/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: srcs/algorithm/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(NAME)
	rm -rf $(OBJDIR)

re: fclean all

.PHONY: all clean fclean re
