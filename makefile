#OBJS specifies which files to compile as part of the project
OBJS = main.c

#CC specifies which compiler we're using
CC = gcc

#FILES
FILES = $(wildcard ./*.c ./*.h)

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -w

#LINKER_FLAGS and LIBS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2 
LIBS = -lSDL2_image

#OBJ_NAME specifies the name of our executable
OBJ_NAME = game

#This is the target that compiles our executable
all : $(FILES)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME) $(LIBS) -w -lpthread
