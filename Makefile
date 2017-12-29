
CPP      = g++.exe
OBJ      = font.o
BIN      = sh234font.exe
CXXFLAGS = -std=c++11

all: $(BIN)

clean:
	rm -f $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN)

font.o: font.cpp
	$(CPP) -c font.cpp -o font.o $(CXXFLAGS)
