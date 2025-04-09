CC = gcc
CFLAGS = -Wall -g
SRC = main.c Weather.c Binary.c
OBJ = $(SRC:.c=.o)
OUT = program

#Default target
all: $(OUT)

#Link object file to create exe
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT)

#Compile source file in obj file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#Clean
# rm -f $(OBJ) $(OUT)