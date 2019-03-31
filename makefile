#!bin/bash

all: Example
	g++ Example.cpp -o ttapp `pkg-config --cflags --libs opencv` -lopenalpr
