FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -y update && \
	apt-get -y dist-upgrade && \
	apt-get -y install gcc g++ cmake autoconf dpkg wget git curl libcurl4-openssl-dev && \
	apt-get -y install libboost-all-dev && \
	wget http://archive.ubuntu.com/ubuntu/pool/main/g/glibc/multiarch-support_2.27-3ubuntu1.6_amd64.deb && \
	dpkg -i multiarch-support_2.27-3ubuntu1.6_amd64.deb && \
	apt-get -y install libssl-dev libkrb5-dev zlib1g && \
	wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/dependencies/libuv/v1.35.0/libuv1_1.35.0-1_amd64.deb && \
	dpkg -i libuv1_1.35.0-1_amd64.deb && \
	wget http://archive.ubuntu.com/ubuntu/pool/main/o/openssl/libssl1.1_1.1.1f-1ubuntu2_amd64.deb && \
	dpkg -i libssl1.1_1.1.1f-1ubuntu2_amd64.deb && \
	wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/cassandra/v2.15.3/cassandra-cpp-driver_2.15.3-1_amd64.deb && \
	dpkg -i cassandra-cpp-driver_2.15.3-1_amd64.deb 

#RUN apt-get -y install libuv1
#RUN apt-get -y install libuv1-dev

#RUN apt-get -y install gdb python3 python3-pip && \
#	pip3 install cqlsh
	
COPY . .
RUN dpkg -i ./external/prometheus-cpp_0.12.3_amd64.deb
RUN mkdir build && \
	cd build && \
	cmake .. && \
	cmake --build .
ENTRYPOINT ["./build/Database-APIexe"]
