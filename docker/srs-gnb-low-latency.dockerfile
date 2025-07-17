FROM ubuntu:jammy

ENTRYPOINT ["/lib/systemd/systemd"]

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    git \
    build-essential \ 
    cmake \
    libfftw3-dev \
    libmbedtls-dev \
    libboost-program-options-dev \
    libconfig++-dev \
    libsctp-dev \
    libzmq3-dev \
    software-properties-common \
    g++ \
    gcc \
    clang \
    libncurses5-dev \
    libtecla1 \
    libtecla-dev \
    pkg-config \
    libyaml-cpp-dev \
    libgtest-dev \
    iproute2 \
    dnsutils \
    iputils-ping \
    libtool \
    nano \
    bc \
    ccache \
    protobuf-compiler \
    libprotobuf-dev \
    libboost-all-dev \
    gnupg \
    dirmngr \
    vim

RUN add-apt-repository -y ppa:ettusresearch/uhd

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends \
    libusb-1.0-0 \
    udev \
    uhd-host \
    libuhd-dev 

RUN uhd_images_downloader

COPY srsRAN_Project_Low_Latency/ /srsRAN_Project_Low_Latency

RUN cd /srsRAN_Project_Low_Latency && \ 
    mkdir build && \
    cd build && \
    cmake ../ && \
    make gnb srscu srsdu -j && \
    make install 
