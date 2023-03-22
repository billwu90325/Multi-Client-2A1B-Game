#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
using namespace std;
#define MAXLINE 1024

// fetch first element of line as command
string getcmd(string cmdline);

int main(int argc , char *argv[]){
	// acquire IP & port from terminal
	char* address = argv[1];
	int PORT = atoi(argv[2]);

	// UDP
	int sockfd_udp;
	char buffer_udp[MAXLINE];
	char message_udp[500];
	struct sockaddr_in servaddr_udp;

	int n_udp;
	socklen_t len_udp;
	// 
	if((sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		/*socket(int domain, int type, int protocol)
			- Domain:
				- between process: AF_UNIX, AF_LOCAL
				- through internet: AF_INET(IPv4), AF_INET6(IPv6)
			- Type:
				- SOCK_STREAM: TCP(serial stream)
				- SOCK_DRAM: UDP(datagram)
			- Return:
				"Socket file descriptor" will be returned
				- Success -> we can manipulate socket through the sockfd
				- Fail -> return -1
		*/
		printf("socket creation failed");
		exit(0);
	}

	// set up the server info we are trying connecting to
	memset(&servaddr_udp, 0, sizeof(servaddr_udp)); // initialize
	servaddr_udp.sin_family = AF_INET; // type
	servaddr_udp.sin_port = htons(PORT); // port
	// htons(): to solve the problem that endian method might be different between client and server 
	servaddr_udp.sin_addr.s_addr = inet_addr(address); // ip
	// inet_addr(): convert IP type into integer(accurately binary)

	/*
	struct sockaddr_in {
		short            sin_family;
		unsigned short   sin_port;
		struct in_addr   sin_addr;
	};

	struct in_addr {
		unsigned long s_addr;
	};
	*/

	// New connection
	strncpy(message_udp, "New", sizeof(message_udp));
	sendto(sockfd_udp, (const char*)message_udp, strlen(message_udp), 0, (const struct sockaddr*)&servaddr_udp, sizeof(servaddr_udp));
	// flag is usually set to 0, except for some situations(blocking, non-blocking, ...)
	// return value of sendto() is a size of data; if fail, is -1
	n_udp = recvfrom(sockfd_udp, (char*)buffer_udp, MAXLINE, 0, (struct sockaddr*)&servaddr_udp, &len_udp);
	cout << buffer_udp << endl;


	//TCP
	int sockfd;
	char buffer[MAXLINE];
	struct sockaddr_in servaddr;

	int n; 
	socklen_t len;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket creation failed");
		exit(0);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(address);
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		printf("\n Error : Connect Failed \n");
	}

	while(true){
		//read(sockfd, buffer, sizeof(buffer));
		//puts(buffer);
		string command;
		getline(cin, command);
		string cmd = getcmd(command);
		if(cmd == "register"){
			strncpy(message_udp, command.c_str(), sizeof(message_udp));
			sendto(sockfd_udp, (const char*)message_udp, strlen(message_udp), 0, (const struct sockaddr*)&servaddr_udp, sizeof(servaddr_udp));
			n_udp = recvfrom(sockfd_udp, (char*)buffer_udp, MAXLINE, 0, (struct sockaddr*)&servaddr_udp, &len_udp);
			puts(buffer_udp);

		}else if(cmd == "login" || cmd == "logout" || cmd == "exit" || cmd == "start-game" || cmd == "create" || cmd == "join" ){
			memset(buffer, 0, sizeof(buffer));
			strncpy(buffer, command.c_str(), sizeof(buffer));
			write(sockfd, buffer, sizeof(buffer));
			read(sockfd, buffer, sizeof(buffer));
			string response = buffer;
			if(response == "close"){
				close(sockfd);
				break;
			}else{
				puts(buffer);
			}
		}else{
			memset(buffer, 0, sizeof(buffer));
			strncpy(buffer, command.c_str(), sizeof(buffer));
			write(sockfd, buffer, sizeof(buffer));
			read(sockfd, buffer, sizeof(buffer));
			puts(buffer);
		}
	}
}


string getcmd(string command){
	string cmd;
	int cnt = 0;
	int pre = 0;
	string a[100];
	for(int i=0 ; i<command.length() ; i++){
		if(command[i] == ' '){
			a[cnt] = command.substr(pre,i-pre);
			pre = i+1;
			cnt ++;
		}
	}
	a[cnt] = command.substr(pre,command.length());
	return a[0];
}