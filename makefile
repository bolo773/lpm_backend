#!bin/bash

all: Example
	g++ --std=c++11 TTcore.cpp -o ttapp `pkg-config --cflags --libs opencv` -lopenalpr -lpthread -lmysqlcppconn -lsqlite3
