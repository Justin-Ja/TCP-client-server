# TCP Client/Server

## Description

Created by Justin Jakel
A client/server TCP pair that allows the client to transfer a file for the server to write to disk and save.
Works on localhost and across two servers on the same host.

## Using the files

### Making the executables

To make the executables, you can run 'make' or 'make all' (no quotes).
This will generate two executables, clientTCP and serverTCP
Running 'make clean' will remove these executables from the disk.

### Running serverTCP

To run the server, use the following:

```bash
./serverTCP portNumber bufferSize
```

- portNumber is the port number the server will live on
- bufferSize is an optional int value to set the buffer size of the server. If no value is given, the default size is 4096.

Running the executable with no arguments will display this information in the terminal.

### Running clientTCP

To run the client, use the following:

```bash
./clientTCP fileName IP:PortNumber bufferSize
```

- fileName is the name of a text file in the same directory of the client executable
- IP:PortNumber is an IPv4 address with a port number. An example would be 127.0.0.1:8080
- bufferSize is an optional int value to set the buffer size of the client. If no value is given, the default size is 4096.

To test on localhost, pass in the IP as 127.0.0.1 along with a port number.
The file name is expected to be a .txt file. The server will write the file to disk with a .txt extension
Ensure that the port number given here matches the one given to the server, otherwise the client will not connect.

Running the executable with no arguments will display this information in the terminal.

### Running the script

To test multiple clients, use the provided bash file to spawn 5 clients which will try to connect to the server at the same time.
To run the script, use the following command. It takes the same arugments as the TCP client, in the same order

```bash
./launchMutlipleClients.sh smallTest.txt 127.0.0.1:8080 1024
```

If the script isn't running/auto filling, the file may need to be modified to be an executable. Run the following:

```bash
chmod 755 launchMutlipleClients.sh 
```

## Additional notes

- The server is configured to queue up to 10 clients before dropping any more requests
- When using the script, the clients will not print to the console in order, so the messages from each program will be mixed together
- The server will not overwrite files if there are duplicates, it will instead append a incremental number to its name.
  - For example, if you pass in Test.txt multiple times, the server would have Test.txt, Test(1).txt, Test(2).txt etc.
- If you want to spawn more clients using the script, just modifiy this one line in the file with whatever maximum you want:

```bash
for i in {1..5}; do # Replace 5 with new max you want
```
