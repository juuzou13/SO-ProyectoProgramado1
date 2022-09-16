  CC = gcc 
  CFLAGS  = -g -Wall
  RM = rm -f
  TARGET = main
  FILES = $(wildcard ./*.c ./*.h)
 
  all: $(TARGET) run
  
  $(TARGET): $(FILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -w -lpthread
 
  clean:
	@$(RM) $(TARGET)
		
  run:
	@./$(TARGET)
	
  	
