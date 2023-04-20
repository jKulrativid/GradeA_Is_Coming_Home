FROM ubuntu:18.04

WORKDIR /app

RUN apt update && apt upgrade -y

RUN apt install libtbb-dev -y
RUN apt install gcc -y
RUN apt install g++-8 -y
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 10 && update-alternatives --config g++

COPY sort-*.txt .
COPY main.cpp .

RUN g++ -pthread -ggdb3 -lstdc++fs -std=c++17 -Wall -Wextra -pedantic -o main.out main.cpp -fopenmp -ltbb -lstdc++fs

ENTRYPOINT [ "./main.out" ]
