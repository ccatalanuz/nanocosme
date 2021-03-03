# nanocosme makefile
# CCC 3/2021

APP_PATH = .
RUNTIME_PATH = ./runtime

all: application.o loop_handlers.o nanocosme_app

application.o: application.cpp
	gcc -fpreprocessed -dD -E application.cpp > application.tmp1.cpp
	$(RUNTIME_PATH)/preprocessor application.h application.tmp1.cpp $(RUNTIME_PATH)/
	g++ -c -I$(RUNTIME_PATH) application.cpp -lm
	rm application.tmp1.cpp

loop_handlers.o: application.cpp
	g++ -c -I$(RUNTIME_PATH) loop_handlers.cpp -lpthread -lrt -Wall
	rm loop_handlers.cpp

nanocosme_app: application.cpp
	g++ -o nanocosme_app $(RUNTIME_PATH)/runtime.o application.o loop_handlers.o \
	$(RUNTIME_PATH)/gateway_mqtt.o $(RUNTIME_PATH)/name_server.o -lpthread -lrt -Wall -lm -lwiringPi -L$(RUNTIME_PATH)/lib -lpaho-mqtt3as
	rm loop_handlers.o	
	rm application.o