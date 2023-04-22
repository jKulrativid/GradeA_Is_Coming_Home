FROM ubuntu:20.04 AS build

WORKDIR /app

RUN apt update

RUN apt install software-properties-common -y && add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN apt install gcc-9 -y
RUN apt install g++-9 -y
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 10 && update-alternatives --config g++
RUN apt install libtbb-dev -y

# RUN if [ "$MODE" = "profile" ] ; then apt install valgrind && rm valgrind.log*

COPY main.cpp .

RUN g++ -pthread -ggdb3 -O3 -lstdc++fs -std=c++17 -mavx -march=znver1 -mtune=znver1 -mpopcnt -mbmi2 -Wall -Wextra -pedantic -o prog main.cpp -fopenmp -ltbb -lstdc++fs

FROM debian:11 AS prod

WORKDIR /app

RUN apt update

COPY --from=build /usr/lib /usr/lib
RUN ldconfig

COPY --from=build /app/prog .
