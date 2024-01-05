# vim:set ft=dockerfile:
FROM ubuntu:20.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq && \
    apt-get install -y gcc g++ make cmake \
       libfcgi-dev libxml2-dev libmemcached-dev \
       libboost-program-options-dev \
       libcrypto++-dev libyajl-dev \
       libpqxx-dev zlib1g-dev libargon2-dev libfmt-dev \
       postgresql-12 postgresql-server-dev-all \
       --no-install-recommends && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the main application.
COPY . ./

# Compile, install and remove source
RUN mkdir build && cd build && \
     CXXFLAGS="-Wall -Wextra -Wpedantic -Wno-unused-parameter" cmake .. -DBUILD_SHARED_LIBS=OFF -DENABLE_FMT_HEADER=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release && \
     make -j${nproc} && \
     ctest --output-on-failure && \
     strip openstreetmap-cgimap && \
     cp openstreetmap-cgimap ../


FROM ubuntu:20.04

RUN apt-get update -qq && \
    apt-get install -y \
       libfcgi-bin libmemcached11 libboost-locale1.71.0 libboost-program-options1.71.0 \
       libxml2 libcrypto++6 libyajl2 libpqxx-6.4 zlib1g libargon2-1 \
       --no-install-recommends && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/openstreetmap-cgimap /usr/local/bin

RUN groupadd -g 61000 cgimap && \
    useradd -g 61000 -l -M -s /bin/false -u 61000 cgimap

USER cgimap

ENV CGIMAP_HOST db
ENV CGIMAP_DBNAME openstreetmap
ENV CGIMAP_USERNAME openstreetmap
ENV CGIMAP_PASSWORD openstreetmap
ENV CGIMAP_MEMCACHE memcached
ENV CGIMAP_RATELIMIT 204800
ENV CGIMAP_MAXDEBT 250
ENV CGIMAP_MODERATOR_RATELIMIT 1048576
ENV CGIMAP_MODERATOR_MAXDEBT 1024
ENV CGIMAP_PORT 8000
ENV CGIMAP_INSTANCES 10

EXPOSE 8000

ENTRYPOINT /usr/local/bin/openstreetmap-cgimap --pidfile /tmp/cgimap.pid --logfile=/proc/1/fd/1 --daemon && \
           tail --pid=$(cat /tmp/cgimap.pid) -f /dev/null
