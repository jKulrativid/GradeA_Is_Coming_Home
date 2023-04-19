FROM ubuntu:18.04

WORKDIR /app

RUN apt update && apt upgrade -y

RUN apt install gcc -y
RUN apt install g++ -y
RUN apt install libtbb-dev -y

COPY . .

RUN g++ -ggdb3 -O3 -lstdc++fs -std=c++17 -Wall -Wextra -pedantic -o main.out main.cpp -ltbb

ENTRYPOINT [ "./main.out" ]
