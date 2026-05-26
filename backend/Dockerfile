FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y gcc g++ cmake libjsoncpp-dev uuid-dev zlib1g-dev libpq-dev libdrogon-dev \
    libsqlite3-dev libmariadb-dev libbrotli-dev libhiredis-dev libyaml-cpp-dev

WORKDIR /server
COPY . .
RUN cmake -S . -B build
RUN cmake --build build
CMD ["./build/server"]