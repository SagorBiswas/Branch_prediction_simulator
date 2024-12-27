CC = g++
OPT = -O3
#OPT = -g
WARN = -Wall
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB)

# Source file
SIM_SRC = branch_predictor.cpp      

# Object file (named according to the source file)
SIM_OBJ = branch_predictor.o

#################################

# Default rule to create sim
all: sim
	@echo "my work is done here..."

# Rule for making sim_cache
sim: $(SIM_OBJ)
	$(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH SIM-----------"

# Generic rule for compiling .cpp files into .o files
.cpp.o:
	$(CC) $(CFLAGS) -c $<

# Rule to remove all .o files plus the sim_cache binary
clean:
	rm -f *.o sim

