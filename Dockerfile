FROM gcc
RUN apt-get update && \
    apt-get -y install autoconf dpkg wget cmake && \
    update-alternatives --install /usr/bin/gfortran gfortran /usr/local/bin/gfortran 999 && \
    apt-get -y install libboost-all-dev && \
    wget http://archive.ubuntu.com/ubuntu/pool/main/g/glibc/multiarch-support_2.27-3ubuntu1.4_amd64.deb && \
    dpkg -i multiarch-support_2.27-3ubuntu1.4_amd64.deb && \
    apt-get install libssl-dev libkrb5-dev zlib1g && \
    wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/dependencies/libuv/v1.35.0/libuv1_1.35.0-1_amd64.deb && \
    dpkg -i libuv1_1.35.0-1_amd64.deb && \
    wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/cassandra/v2.15.3/cassandra-cpp-driver_2.15.3-1_amd64.deb && \
    dpkg -i cassandra-cpp-driver_2.15.3-1_amd64.deb
	COPY . .
RUN mkdir build && \
	cd build && \
	cmake .. && \
	cmake --build .
ENTRYPOINT ["./build/Database-API/Database-API"]