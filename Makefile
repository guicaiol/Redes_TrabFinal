SRC		= src
OBJ		= obj
BIN		= bin
CC		= gcc
FLAGS	= -Wall -pthread
TARGET	= $(BIN)/trabFinalGEN05

OBJECTS = \
	$(OBJ)/client.o \
	$(OBJ)/connection.o \
	$(OBJ)/global.o \
	$(OBJ)/main.o \
	$(OBJ)/messenger.o \
	$(OBJ)/server.o \
	$(OBJ)/timer.o
	
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) -o $(TARGET)
	
$(OBJ)/client.o:
	$(CC) $(FLAGS) -c $(SRC)/client.c -o $@
	
$(OBJ)/connection.o:
	$(CC) $(FLAGS) -c $(SRC)/connection.c -o $@
	
$(OBJ)/global.o:
	$(CC) $(FLAGS) -c $(SRC)/global.c -o $@
	
$(OBJ)/main.o:
	$(CC) $(FLAGS) -c $(SRC)/main.c -o $@
	
$(OBJ)/messenger.o:
	$(CC) $(FLAGS) -c $(SRC)/messenger.c -o $@
	
$(OBJ)/server.o:
	$(CC) $(FLAGS) -c $(SRC)/server.c -o $@
	
$(OBJ)/timer.o:
	$(CC) $(FLAGS) -c $(SRC)/timer.c -o $@
	
clean:
	rm -f $(OBJ)/* $(TARGET)
		
run: all
	@./$(TARGET)
	
valgrind: all
	@valgrind ./$(TARGET)

