# Image
FROM ubuntu:24.04

# Environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
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
    iproute2 \
    tcpdump \
    tcpreplay \
    nano \
    && apt-get clean

# Build libdaq from source
WORKDIR /usr/src
RUN git clone https://github.com/snort3/libdaq.git && \
    cd libdaq && \
    ./bootstrap && \
    ./configure && \
    make install && \
    cd

# Packets to test 
RUN git clone https://github.com/StopDDoS/packet-captures.git

# Build and install Snort 3
RUN git clone https://github.com/snort3/snort3.git && \
cd snort3 && \
./configure_cmake.sh --prefix=/usr/local && \
cd build && \
make -j$(nproc) && \
make install && \
cd

# Objects to use in tests
RUN git clone https://github.com/MateusHerbele/packet-wash-snort.git
# Extract Snort rules to the appropriate location
RUN tar -xvzf /usr/src/packet-wash-snort/snortrules-snapshot-31210.tar.gz -C /usr/src/snort3/lua


# Clean up
RUN rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Set up entrypoint
ENTRYPOINT ["/bin/bash"]

# Snort -> Libdaq
ENV LD_LIBRARY_PATH=/usr/local/lib
