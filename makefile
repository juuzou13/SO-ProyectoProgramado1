  CC = gcc 
  CFLAGS  = -g -Wall
  RM = rm -f
  TARGET = main
 
  all: $(TARGET) run
  
  $(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -w
 
  clean:
	@$(RM) $(TARGET)
		
  run:
	@./$(TARGET)
	
  	
