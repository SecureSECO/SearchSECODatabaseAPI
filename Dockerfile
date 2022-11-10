FROM ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
  && apt-get dist-upgrade -y \
  && apt-get install -y --no-install-recommends \
    ca-certificates \
    clang \
    curl \
    git \
    libcurl4-openssl-dev \
    libkrb5-dev \
    libssl-dev \
    make \
    software-properties-common \
    wget \
    zlib1g-dev \
  && rm -rf /var/lib/apt/lists/*

RUN cd /tmp \
  && apt-get update \
  && wget http://archive.ubuntu.com/ubuntu/pool/main/g/glibc/multiarch-support_2.27-3ubuntu1.6_amd64.deb \
  && dpkg -i multiarch-support_2.27-3ubuntu1.6_amd64.deb \
  && wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/dependencies/libuv/v1.35.0/libuv1_1.35.0-1_amd64.deb \
  && dpkg -i libuv1_1.35.0-1_amd64.deb \
  && wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/cassandra/v2.15.3/cassandra-cpp-driver_2.15.3-1_amd64.deb \
  && dpkg -i cassandra-cpp-driver_2.15.3-1_amd64.deb \
  && apt-get install -f \
  && rm -rf /var/lib/apt/lists/* \
  && rm -rf /tmp/*.deb

# install newer cmake from PPA, fixes:
# CMake 3.13 or higher is required. You are running version 3.10.2
# https://apt.kitware.com/
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null \
    | gpg --dearmor - \
    | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
  && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ bionic main' \
    | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
  && apt-get update \
  && apt-get install -y --no-install-recommends \
    cmake \
  && rm -rf /var/lib/apt/lists/*

# install cmake from source:
# ARG VERSION_CMAKE=3.25.0-rc3
# RUN cd /tmp \
#   && wget https://github.com/Kitware/CMake/releases/download/v${VERSION_CMAKE}/cmake-${VERSION_CMAKE}.tar.gz \
#   && tar -zxvf cmake-${VERSION_CMAKE}.tar.gz \
#   && cd cmake-${VERSION_CMAKE} \
#   && ./bootstrap \
#   && make -j $(nproc) \
#   && make install \
#   && rm -rf /tmp/cmake*

ARG VERSION_LIBBOOST=1.74
RUN add-apt-repository ppa:mhier/libboost-latest \
  && apt-get install -y \
    libboost${VERSION_LIBBOOST}-dev \
  && rm -rf /var/lib/apt/lists/*

ARG VERSION_PROMETHEUS_CPP=1.0.1
ENV URL_PROMETHEUS_CPP=https://github.com/jupp0r/prometheus-cpp/releases/download/v${VERSION_PROMETHEUS_CPP}/prometheus-cpp-with-submodules.tar.gz
RUN cd /tmp \
  && wget ${URL_PROMETHEUS_CPP} \
  && tar xzf prometheus-cpp-with-submodules.tar.gz \
  && cd prometheus-cpp-with-submodules \
  && mkdir _build \
  && cd _build \
  && cmake .. \
    -DBUILD_SHARED_LIBS=ON \
    -DENABLE_COMPRESSION=ON \
    -DENABLE_PUSH=ON \
  && cmake \
    --build . \
    --parallel 4 \
  && cmake --install . \
  && rm -rf /tmp/prometheus-cpp*

COPY . /src

WORKDIR /src/build

RUN cmake .. \
  && cmake \
    --build . \
    --parallel 4 \
    --verbose

ENTRYPOINT ["/src/build/Database-APIexe"]
