CXX=g++
CXXFLAGS=-std=c++17 -Wall -I/usr/local/include
LDFLAGS=-L/usr/local/lib -lpaho-mqttpp3 -lpaho-mqtt3as

all: mqttClient

mqttClient: src/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f mqttClient
