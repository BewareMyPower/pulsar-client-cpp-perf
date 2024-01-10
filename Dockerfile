FROM ubuntu:22.04

WORKDIR /app
RUN apt-get update -y \
    && apt-get install -y vim gcc g++ curl ca-certificates cmake git zip unzip ninja-build pkg-config
RUN git clone https://github.com/BewareMyPower/pulsar-client-cpp-perf.git && \
    cd pulsar-client-cpp-perf && \
    git submodule update --init --recursive
RUN cd pulsar-client-cpp-perf && cmake -B build && cmake --build build -j8

WORKDIR /app
FROM ubuntu:22.04
RUN apt-get update -y \
    && apt-get install -y vim gcc g++ curl ca-certificates
COPY pulsar_config.ini /etc/pulsar/config.ini
COPY --from=0 /app/pulsar-client-cpp-perf/build/vcpkg_installed /app/vcpkg_installed
COPY --from=0 /app/pulsar-client-cpp-perf/build/stress_producer_tool /app/stress_producer_tool
