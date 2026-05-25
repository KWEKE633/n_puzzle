CXX = c++
CXX_FLAGS = -Wall -Wextra -Werror -std=c++20 -I .
NAME = n_puzzle
SRC_DIR = srcs
SRC = $(SRC_DIR)/main.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXX_FLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
