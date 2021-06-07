# Usage

The database can handle three different requests. Every request has a different four-letter identifier which should be specified in the input. After the identifier the length of the input data should be specified, this is the number of ASCII characters (so `\n` has a length of 1). Following this should be the input data in a specific format, explained below.
* With the check request you can check if a method or multiple methods are present in the database. If they are present, they will be printed on the command-line. The identifier for this request is `chck`. The data should have the following format: `hash_1\nhash_2\n...\nhash_N\n`, where `hash_i` is the hash of a method you want to match to.
* With the upload request you can upload a project with its specified methods. The identifier for this request is `upld`. The data should have the following format: `projectID?version?license?project_name?url?author_name?author_mail\nmethod1_hash?method1_name?method1_fileLocation?method1_lineNumber?method1_numberOfAuthors?method1_author1_name?method1_author1_mail?<other authors>\n<method2_data>\n...\n<methodN_data>`, where `url` is the URL of the project on GitHub.
* With the checkupload request you can match the input data to existing entries in the database, matches will show up on the command-line. After this, the project and methods will be added to the database. The identifier for this request is `chup`. The format for the data is the same as for the upload request.

# Running in container

### Joining

You can run this program with the database using Docker. For this you will first need to install [Docker](https://docs.docker.com/get-docker/) and [Docker compose](https://docs.docker.com/compose/install/) (Included with Docker desktop on Windows). After this you should clone the repository. You should set the variables in the `.env` file. The _LOC_ is for the location of the data to store, _SEEDS_ is for the ips of the nodes to connect to and the _IP_ is for the public IP-addres of the current computer. In order to be able to have contact with the rest of the database you should also open ports `8001` and `8002`. You can then start the program using `docker-compose up -d` in the main folder of the repository. This will automatically have you computer join the distributed database. After this the API should be listening on port `8003` for requests.

### Leaving

To stop the program you should first tell it to leave the database by using `docker exec cassandra nodetool decommision`. After this you can use `docker stop api` and `docker stop cassandra` to stop the containers.

### Running tests

To locally run the tests(including integration tests) you can first use `docker build -f TestingDockerfile -t testing .` in the main folder to build the container. After this you can use `docker run --name testContainer testing` to actually run the tests. This will also generate the code coverage. To copy the code coverage files to a local folder you can use `docker cp testContainer:/build/coverage ./coverage`. After this you can open _index.html_ in the coverage folder to see the code coverage.

# Running seperately

If you don't want to run the program in a docker container you can build and run the API seperately using the instructions below.

### Setting up

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

### Building

This can be done using the following commands in the projects main folder:
* `mkdir build`
* `cd build`
* `cmake ..`
* `cmake --build .`

### Running

After this the program can be run using `./Database-API/Database-API`.
This will only work when you also have a [Cassandra](https://cassandra.apache.org/doc/latest/getting_started/installing.html) database running on your computer.

For testing you can run the tests after building by using `./Database-API_Test/tests`.

To generate the code coverage you van use `make coverage` after building using `cmake -DCMAKE_BUILD_TYPE=Debug ..` instead of `cmake ..`. After this there will be an `.html` file inside the coverage folder with detailed information about the code coverage.
