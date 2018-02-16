g++ -c miniz.c
g++ -c SMiniz.cpp
g++ -c main.cpp
g++ -o main miniz.o SMiniz.o main.o
