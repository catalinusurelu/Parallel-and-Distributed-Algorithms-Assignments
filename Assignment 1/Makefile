#Nume: Usurelu
#Prenume: Catalin-Constantin
#Grupa: 333CA

CXX = gcc

CCFLAGS = -Wall
CCFLAGS_OMP = -Wall -fopenmp 

SERIAL = serial
SERIAL_SRC = serial.c
SERIAL_OPT = serial_optimizat
SERIAL_OPT_SRC = serial_optimizat.c
PARALEL = paralel
PARALEL_SRC = paralel.c
PARALEL_OPT = paralel_optimizat
PARALEL_OPT_SRC = paralel_optimizat.c

.PHONY : build, clean

build:
	$(CXX) -o $(SERIAL) $(SERIAL_SRC) $(CCFLAGS)
	$(CXX) -o $(SERIAL_OPT) $(SERIAL_OPT_SRC) $(CCFLAGS)
	$(CXX) -o $(PARALEL) $(PARALEL_SRC) $(CCFLAGS_OMP)
	$(CXX) -o $(PARALEL_OPT) $(PARALEL_OPT_SRC) $(CCFLAGS_OMP)

clean :
	rm -f $(SERIAL) $(SERIAL_OPT) $(PARALEL) $(PARALEL_OPT)
	rm -f *.o
