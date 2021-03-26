To run this project you will need some sort of Ubuntu machine(like a VM).
You will then have to perform the following commands in a folder that can contain some files:
* `sudo apt-get install libboost-all-dev`
* `wget http://archive.ubuntu.com/ubuntu/pool/main/g/glibc/multiarch-support_2.27-3ubuntu1.4_amd64.deb`
* `sudo dpkg -i multiarch-support_2.27-3ubuntu1.4_amd64.deb`
* `apt-get install libssl-dev libkrb5-dev zlib1g`
* `wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/dependencies/libuv/v1.35.0/libuv1_1.35.0-1_amd64.deb`
* `dpkg -i libuv1_1.35.0-1_amd64.deb`
* `wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/cassandra/v2.15.3/cassandra-cpp-driver_2.15.3-1_amd64.deb`
* `dpkg -i cassandra-cpp-driver_2.15.3-1_amd64.deb`

This will install all the dependencies.
After this you will need to clone the desired branch.
You can then use `cmake` and `cmake --build` to run the program.