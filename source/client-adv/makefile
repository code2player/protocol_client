CXX = g++
CXXFLAGS = -Wall
RM = rm -f
OBJ = ts
SOURCE = client-adv_main.o client-adv_sub.o encrpty.o
SOURCEH = ../common/mydaemon.o ../common/parameter_process.o ../common/tcp_common.o ../common/socket_common.o ../common/parameter_config.o
DEF = ../include/mydaemon.h ../include/parameter_process.h ../include/tcp_common.h ../include/socket_common.h client-adv.h ../include/parameter_config.h

all : $(SOURCE) $(SOURCEH) $(DEF)
	$(CXX) $(CXXFLAGS) $(SOURCE) $(SOURCEH) -o $(OBJ)
	cp -f ts ..

.PHONY : clean
clean :
	$(RM) $(SOURCE) $(SOURCEH) $(OBJ)