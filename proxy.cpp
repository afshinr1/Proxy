
/* standard libraries*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* libraries for socket programming */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*libraries for parsing strings*/
#include <string.h>
#include <strings.h>

/*h_addr address*/
#include <netdb.h>

/*clean exit*/
#include <signal.h>

#include <string>
#include <iostream>

/* port numbers */
#define HTTP_PORT 80

/* string sizes */
#define MESSAGE_SIZE 2048

int lstn_sock, data_sock, web_sock;

using namespace std;

void cleanExit(int sig){
	close(lstn_sock);
	close(data_sock);
	close(web_sock);
	exit(0);
}

int main(int argc, char* argv[]){

	if (argc != 2) {
        cout << "Please enter a port number as command line argument " << endl;
		exit(1);
    }

	char client_request[MESSAGE_SIZE], server_request[MESSAGE_SIZE], server_response[10*MESSAGE_SIZE], client_response[10*MESSAGE_SIZE];
	char url[MESSAGE_SIZE], host[MESSAGE_SIZE], path[MESSAGE_SIZE];
	int clientBytes, serverBytes, i;
	int port = atoi(argv[1]);


    /* to handle ungraceful exits */
    signal(SIGTERM, cleanExit);
    signal(SIGINT, cleanExit);

    //initialize proxy address
	printf("Initializing proxy address...\n");
	struct sockaddr_in proxy_addr;
	proxy_addr.sin_family = AF_INET;
	proxy_addr.sin_port = htons(port);
	proxy_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//changeeate listening socket
	printf("changeeating lstn_sock...\n");
	lstn_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (lstn_sock <0){
		perror("socket() call failed");
		exit(-1);
	}

	//bind listening socket
	printf("Binding lstn_sock...\n");
	if (bind(lstn_sock, (struct sockaddr*)&proxy_addr, sizeof(struct sockaddr_in)) < 0){
		perror("bind() call failed");
		exit(-1);
	}

	//listen for client connection requests
	printf("Listening on lstn_sock...\n");
	if (listen(lstn_sock, 20) < 0){
		perror("listen() call failed...\n");
		exit(-1);
	}

	//infinite while loop for listening
	while (1){
		printf("Accepting connections...\n");

		//accept client connection request
		data_sock = accept(lstn_sock, NULL, NULL);
		if (data_sock < 0){
			perror("accept() call failed\n");
			exit(-1);
		}

		//while loop to receive client requests
		while ((clientBytes = recv(data_sock, client_request, MESSAGE_SIZE, 0)) > 0){
			printf("%s\n", client_request);

			string change(client_request);
			int count;

			string working = "/~carey/CPSC441/Floppy-Working.jpg";
			string lunch = "/~carey/CPSC441/Lunch-Floppy.jpg";
			string garden = "/~carey/CPSC441/Garden-Floppy.jpg";
			string potty = "/~carey/CPSC441/Potty-Floppy.jpg";
			string cute = "/~carey/CPSC441/Cute-Floppy.jpg";

			if((count = change.find("/~carey/CPSC441/Cute-Floppy.jpg")) != string::npos) {
				change.replace(count, cute.length(), "/~carey/CPSC441/trollface.jpg");
			}

			if((count = change.find("/~carey/CPSC441/Lunch-Floppy.jpg")) != string::npos) {
				change.replace(count, lunch.length(), "/~carey/CPSC441/trollface.jpg");
			}

			if((count = change.find("/~carey/CPSC441/Garden-Floppy.jpg")) != string::npos) {
				change.replace(count, garden.length(), "/~carey/CPSC441/trollface.jpg");
			}

			if((count = change.find("/~carey/CPSC441/Potty-Floppy.jpg")) != string::npos) {
				change.replace(count, potty.length(), "/~carey/CPSC441/trollface.jpg");
			}

			if((count = change.find("/~carey/CPSC441/Floppy-Working.jpg")) != string::npos) {
			change.replace(count, working.length(), "/~carey/CPSC441/trollface.jpg");
			}

			//copy to server_request to preserve contents (client_request will be damaged in strtok())
			strcpy(client_request, change.c_str());
			cout << "New CLIENT REQUEST: " << client_request << endl;
			strcpy(server_request, client_request);

			//tokenize to get a line at a time
			char *line = strtok(client_request, "\r\n");

			//extract url
			sscanf(line, "GET http://%s", url);

			//separate host from path
			for (i = 0; i < strlen(url); i++){
				if (url[i] =='/'){
					strncpy(host, url, i);
					host[i] = '\0';
					break;
				}
			}
			bzero(path, MESSAGE_SIZE);
			strcat(path, &url[i]);

			printf("Host: %s, Path: %s\n", host, path);

			//initialize server address
			printf("Initializing server address...\n");
			struct sockaddr_in server_addr;
			struct hostent *server;
			server = gethostbyname(host);

			bzero((char*)&server_addr, sizeof(struct sockaddr_in));
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(HTTP_PORT);
			bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);

			//changeeate web_socket to communicate with web_server
			web_sock = socket(AF_INET, SOCK_STREAM, 0);
			if (web_sock < 0){
				perror("socket() call failed\n");
			}

			//send connection request to web server
			if (connect(web_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in))<0){
				perror("connect() call failed\n");
			}

			//send http request to web server
			if (send(web_sock, server_request, MESSAGE_SIZE, 0) < 0){
				perror("send(0 call failed\n");
			}

			//receive http response from server
			serverBytes = 0;
			while((serverBytes = recv(web_sock, server_response, MESSAGE_SIZE, 0)) > 0){

				string str2(server_response);

				//change floppy to trolly and italy to japan, without changing images
				for(int i = 0; i < str2.length(); i++) {

					count = str2.find("Floppy ", i);
					if(count > 0){
						str2.replace(count, 5, "Troll");
					}

					count = str2.find("Floppy,", i);
					if(count > 0){
						str2.replace(count, 5, "Troll");
					}

					count = str2.find("Italy", i);

					if(count > 0) {
						str2.replace(count, 5, "Japan");
					}
				}

				strcpy(server_response, str2.c_str());

				//we are not modifying here, just passing the response along
				printf("%s\n", server_response);

				//copy modified response back into server_response
				bcopy(server_response, client_response, serverBytes);

				//send http response to client
				if (send(data_sock, client_response, serverBytes, 0) < 0){
					perror("send() call failed...\n");
				}
				bzero(client_response, MESSAGE_SIZE);
				bzero(server_response, MESSAGE_SIZE);
			}//while recv() from server
		}//while recv() from client
		close(data_sock);
	}//infinite loop
	return 0;
}



/*

int found;

string str2(server_response);
found = 0;
while(found != string::npos){
found = str2.find("Floppy ", found);
if(found >0)
str2.replace(found, 5, "Troll");

}
found = 0;
while(found != string::npos){
found = str2.find("Floppy,", found);
if(found >0)
str2.replace(found, 5, "Troll");

}

found = 0;
while(found != string::npos){
found = str2.find("Italy", found);
if(found >0)
str2.replace(found, 5, "Japan");

}


strcpy(client_response, str2.c_str());

*/
