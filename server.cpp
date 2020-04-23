//Server Program

/*
 * A simple TCP server that echos messages back to the client.
 * This server works with only a single client.  With a simple modification, it can take more clients.
* This example is adopted from CPSC 441 tutorials thought by Prof. Mea Wang 
 */
 
#include <iostream>
#include <sys/socket.h> // for socket(), connect(), send(), and recv()
#include <arpa/inet.h>  // for sockaddr_in and inet_addr()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()

using namespace std;

const int BUFFERSIZE = 32;   // Size the message buffers
const int MAXPENDING = 1;    // Maximum pending connections

int main(int argc, char *argv[])
{
    int serverSock;                  // server socket descriptor
    int clientSock;                  // client socket descriptor
    struct sockaddr_in serverAddr;   // address of the server
    struct sockaddr_in clientAddr;   // address of the client
    
    char inBuffer[BUFFERSIZE];       // Buffer for the message from the server
    int bytesRecv, bytes;            // Number of bytes received
    int bytesSent;                   // Number of bytes sent
       
    // Check for input errors
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <Listening Port>" << endl;
        exit(1);
    }
    
    // Create a TCP socket
    // * AF_INET: using address family "Internet Protocol address"
    // * SOCK_STREAM: Provides sequenced, reliable, bidirectional, connection-mode byte streams.
    // * IPPROTO_TCP: TCP protocol
    if ((serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        cout << "socket() failed" << endl;
        exit(1);
    }
    
    // Free up the port before binding
    // * sock: the socket just created
    // * SOL_SOCKET: set the protocol level at the socket level
    // * SO_REUSEADDR: allow reuse of local addresses
    // * &yes: set SO_REUSEADDR on a socket to true (1)
    // * sizeof(int): size of the value pointed by "yes"
    int yes = 1;
    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
    {
        cout << "setsockopt() failed" << endl;
        exit(1);
    }
    
    // Initialize the server information
    // Note that we can't choose a port less than 1023 if we are not privileged users (root)
    memset(&serverAddr, 0, sizeof(serverAddr));         // Zero out the structure
    serverAddr.sin_family = AF_INET;                    // Use Internet address family
    serverAddr.sin_port = htons(atoi(argv[1]));         // Server port number
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    
    // Bind to the local address
    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cout << "bind() failed" << endl;
        exit(1);
    }
    
    // Listen for connection requests
    if (listen(serverSock, MAXPENDING) < 0)
    {
        cout << "listen() failed" << endl;
        exit(1);
    }    
    
    // set the size of the client address structure
    socklen_t size = sizeof(clientAddr);

    // Waiting for connection requests
    if ((clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, &size)) < 0)
    {
        cout << "accept() failed" << endl;
        exit(1);
    }
    
    // The server will be blocked until a client is connected to it.
    cout << "Accepted a connection from " << inet_ntoa(clientAddr.sin_addr) << endl;
    
    // Start communication with the client (terminate when receive a "terminate" command)
    while (strncmp(inBuffer, "terminate", 9) != 0)
    {
        // Clear the buffers
        memset(&inBuffer, 0, BUFFERSIZE);

        // Receive the message from client
        bytesRecv = recv(clientSock, (char *) &inBuffer, BUFFERSIZE, 0);
 
        // Check for connection close (0) or errors (< 0)
        if (bytesRecv <= 0)
        {
            cout << "recv() failed, or the connection is closed. " << endl;
            exit(1); 
        }
        cout << "Client: " << inBuffer;
        
        // Echo the message back to the client
        bytesSent = send(clientSock, (char *) &inBuffer, bytesRecv, 0);
            
        if (bytesSent < 0 || bytesSent != bytesRecv)
        {
            cout << "error in sending" << endl;
            exit(1); 
        }
    }
    
    // Close the connection with the client
    close(clientSock);
    
    // Close the server socket
    close(serverSock);
}
   

