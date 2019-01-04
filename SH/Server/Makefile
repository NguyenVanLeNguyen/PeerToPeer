# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -lpthread

# the list of header files
#INCL   = server.h
#the list of source files
SRC = Server.c
OBJ = $(SRC:.c=.o)
OBJ2 = $(SRC2:.c=.o)

# the build target executable:
TARGET = Server
$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS)
$(OBJ): $(INCL)

clean:
	$(RM) $(TARGET) $(OBJ)
