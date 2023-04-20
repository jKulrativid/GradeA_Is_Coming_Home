FROM ubuntu:20.04

WORKDIR /app

RUN apt update

RUN apt install software-properties-common -y && add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN apt install gcc-9 -y
RUN apt install g++-9 -y
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 10 && update-alternatives --config g++
RUN apt install libtbb-dev -y

# RUN if [ "$MODE" = "profile" ] ; then apt install valgrind && rm valgrind.log*

COPY main.cpp .

RUN g++ -pthread -ggdb3 -O3 -lstdc++fs -std=c++17 -Wall -Wextra -pedantic -o prog main.cpp -fopenmp -ltbb -lstdc++fs
