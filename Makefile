
CPP      = g++.exe
OBJ      = main.o font.o offset.o tga.o
BIN      = sh234font.exe
CXXFLAGS = -std=c++11

all: $(BIN)

clean:
	rm -f $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN)

%.o: %.cpp
	$(CPP) -c $< -o $@ $(CXXFLAGS)
