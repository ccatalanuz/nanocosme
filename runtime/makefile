# nanocosme makefile
# CCC 3/2021

APP_PATH = ..
RUNTIME_PATH = .

all: preprocessor name_server.o gateway_mqtt.o runtime.o

 
preprocessor: preprocessor.cpp name_server.h
	g++ -o preprocessor preprocessor.cpp -Wall
	
name_server.o: name_server.cpp name_server.h
	g++ -c name_server.cpp -Wall		

gateway_mqtt.o: gateway_mqtt.cpp gateway_mqtt.h name_server.h name_server.cpp
	g++ -c -I$(APP_PATH) gateway_mqtt.cpp -I./include

runtime.o: runtime.h runtime.cpp 
	g++ -c -I$(APP_PATH) runtime.cpp -lpthread -lrt -Wall -lm -lwiringPi
