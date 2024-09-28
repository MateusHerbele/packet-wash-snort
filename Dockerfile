FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    wget \
    git \
    libdumbnet-dev \
    flex \
    g++ \
    libhwloc-dev \
    libluajit-5.1-dev \
    libssl-dev \
    libpcap-dev \
    libpcre3-dev \
    pkg-config \
    zlib1g-dev \
    asciidoc \
    cpputest \
    dblatex \
    flatbuffers-compiler \
    libhyperscan-dev \
    libunwind-dev \
    liblzma-dev \
    libsafec-dev \
    source-highlight \
    w3m \
    uuid-dev \
    && apt-get clean

# Libdaq
WORKDIR /usr/src
RUN git clone https://github.com/snort3/libdaq.git && \
    cd libdaq && \
    ./bootstrap && \
    ./configure && \
    make install && \
    cd

# Snort 3
RUN git clone https://github.com/snort3/snort3.git && \
    cd snort3 && \
    ./configure_cmake.sh --prefix=/usr/local && \
    cd build && \
    make -j$(nproc) && \
    make install

# Clean up
RUN rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ENTRYPOINT ["/usr/local/bin/snort"]

# Default command
CMD ["--help"]
