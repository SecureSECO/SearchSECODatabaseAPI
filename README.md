# Database API
The database API is responsible for handling the communication between the controller and the database.

# Installation
To install the program you should clone the git repository.

## Dependencies
The database api code has the following dependencies:
* Cassandra
* Datastax cpp-driver
* Boost-asio

For running the program using Docker you need the following dependencies installed:
* [Docker](https://docs.docker.com/get-docker/)
* [Docker compose](https://docs.docker.com/compose/install/)

If you want to build the program locally in Linux you will need to perform the following commands to get the required dependencies:
* `sudo apt-get install libboost-all-dev`
* `wget http://archive.ubuntu.com/ubuntu/pool/main/g/glibc/multiarch-support_2.27-3ubuntu1.4_amd64.deb`
* `sudo dpkg -i multiarch-support_2.27-3ubuntu1.4_amd64.deb`
* `apt-get install libssl-dev libkrb5-dev zlib1g`
* `wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/dependencies/libuv/v1.35.0/libuv1_1.35.0-1_amd64.deb`
* `dpkg -i libuv1_1.35.0-1_amd64.deb`
* `wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/cassandra/v2.15.3/cassandra-cpp-driver_2.15.3-1_amd64.deb`
* `dpkg -i cassandra-cpp-driver_2.15.3-1_amd64.deb`

## Building
There are two different ways of building the program. You can simply run it using Docker, this is the easiest if you just want to run the program. The other way is to locally build the program using `cmake` on Linux.

### Docker
In order to start the program using Docker you should first set the variables in the `.env` file. The _LOC_ is for the location of the data to store, _SEEDS_ is for the IP-addresses of the nodes to connect to and the _IP_ is for the public IP-address of the current computer. In order to contact the rest of the database you should also open ports `8001` and `8002`. You can then start the program using `docker-compose up -d` in the main folder of the repository. This will automatically have your computer join the distributed database. After this the API should be listening on port `8003` for requests.

### Linux
In order to build the program using `cmake` you should preform the following commands:
* `mkdir build`
* `cd build`
* `cmake ..`
* `cmake --build .`

The Database API can then be started using `./Database-API/Database-API`.

# Usage

The database API can handle multiple different requests. Every request has a different four-letter identifier which should be specified in the input. After the identifier the length of the input data should be specified, this is the number of ASCII characters (so `\n` has a length of 1). Following this should be the input data in the specific format required by the request. The database API supports the following requests for uploading to and extracting from the database:
* The `check` request can be used to check if a method or multiple methods are present in the database.
* The `upload` request can be used to upload or update a project with its specified methods.
* The `check upload` request can be used to match the input data to existing entries in the database, matches will show up on the command-line. After this, the project and methods will be added to the database.
* The `extract projects` request can be used to retrieve the information of projects given their `projectID`.
* The `get author` request can be used to get the name and email corresponding to an author ID.
* The `get method by author` request can be used to get the methods that an author has worked on.
* The `get previous project` request can be used to get the most recent project of a repository in the database.

The API also supports the following requests for the job distribution system:
* The `connect` request can be used to connect a new node to the network.
* The `upload job` request can be used to upload multiple jobs to the jobsqueue.
* The `upload crawl data` request can be used to both upload new jobs and update the crawl id.
* The `get tob job` request can be used to get a job.

### Stopping

To stop the program you should first tell it to leave the distributed database by using `docker exec cassandra nodetool decommision`. After this you can use `docker stop api` and `docker stop cassandra` to stop the containers.

### Running tests

To locally run the tests(including integration tests) you can first use `docker build -f TestingDockerfile -t testing .` in the main folder to build the container. After this you can use `docker run --name testContainer testing` to actually run the tests. This will also generate the code coverage. To copy the code coverage files to a local folder you can use `docker cp testContainer:/build/coverage ./coverage`. After this you can open _index.html_ in the coverage folder to see the code coverage.

# License
This program has been developed by students from the bachelor Computer Science at Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
