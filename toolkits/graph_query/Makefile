CXX = g++
CXXFLAGS = -g -Wall -Wextra -O3

all: bin bin/main

bin:
	mkdir -p bin

bin/main: main.cpp pllwidestpath.h pruned_landmark_labeling.h SystemFeatures.h 2-hop-labels.h
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $^
	
clean:
	rm -rf bin
