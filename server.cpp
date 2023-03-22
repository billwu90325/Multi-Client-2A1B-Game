#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <typeinfo>
#include <cstdlib>
#include <time.h>
#include <sstream> 
#include <random>
#include <ctype.h>
#include <utility>
using namespace std;
#define MAXLINE 1024
#define PORT 8888
 

std::random_device rd;
std::mt19937 gen(rd());

int random(int low, int high);
string read_file(string filename);
vector<string> getelements(string cmdline);
bool not_number(const std::string& s);

int main(int argc , char *argv[]){
	//int PORT = atoi(argv[1]);
	int listenfd, connfd, udpfd, nready, maxfdp1;
	char buffer[MAXLINE];
	fd_set rset;
	// socket container
	ssize_t n;
	socklen_t len;
	const int on = 1;
	struct sockaddr_in cliaddr, servaddr;
	char message[500];
	char message_to[500];
	memset(&message, 0, sizeof(message));
	vector<string> username;
	vector<string> email;
	vector<string> password;
	vector<bool> online(100, false);
	vector<string> answer(100, "");
	vector<int> room(100, -1);
	vector<vector<int> > invitation(100);
	vector<vector<int> > gameroom;
	vector<vector<int> > joinorder;
	vector<int> turns;
	int max_roomid = 0;

	// TCP --listen
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	// binding server address & listen socket 
	bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	// maximun listen number
	listen(listenfd, 10);

	// UDP socket
	udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	// binding server address & UDP
	bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	int max_sd;
	int max_clients = 30;
	int sd;
	int client_socket[30];
	int new_socket;
	len = sizeof(cliaddr);
	bool exist[30];
	vector<string> user(30);
	int remain[30];
    for(int i = 0; i < max_clients; i ++){  
        client_socket[i] = 0;  
		exist[i] = false;
		user[i] = "";
		remain[i] = 5;
    }
	
	while(true){
		FD_ZERO(&rset);
		FD_SET(listenfd, &rset);
		FD_SET(udpfd, &rset);
		max_sd = listenfd;
		for(int i = 0; i < max_clients; i ++){  
            sd = client_socket[i];  
            if(sd > 0){
				FD_SET( sd , &rset);  
			} 
            if(sd > max_sd){
				max_sd = sd; 
			}   
        }
		//cout << "test1" << endl;
		int MAX = max(udpfd, max_sd);
		select(MAX + 1, &rset, NULL, NULL, NULL);
		//cout << "test2" << endl;

		if (FD_ISSET(listenfd, &rset)){  
            if ((new_socket = accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t*)&len))<0){  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
            
            for (int i=0; i<max_clients; i++){  
                if(client_socket[i] == 0){  
                    client_socket[i] = new_socket;   
                    break;  
                }  
            }  
        }

		// TCP
		for(int i = 0; i < max_clients; i++){
            sd = client_socket[i];   
            if (FD_ISSET(sd ,&rset)){
				bzero(buffer, sizeof(buffer));
				//cout << "TCP server is running." << "\n";
				int index  = -1;
				if(read(sd, buffer, sizeof(buffer)) == 0){
					for(int r=0; r<username.size(); r++){
						if(user[i] == username[r]){
							index = r; 
							break;
						}
					}
					if(index != -1){
						online[index] = false;
						if(room[index] != -1){
							bool manager = false;
							int gameroom_index;
							int roomstatus;
							for(int j = 0; j < gameroom.size(); j ++){
								if(room[index] == gameroom[j][0]){
									if(index == gameroom[j][3]){
										manager = true;
									}
									roomstatus = gameroom[j][2];
									gameroom_index = j;
								}
							}
							if(manager){
								for(int j = 0; j < room.size(); j ++){
									if(j != index && room[j] == room[index]){
										room[j] = -1;
									}
								}
								room[index] = -1;
								gameroom.erase(gameroom.begin() + gameroom_index);
							}else if(roomstatus != 0){
								room[index] = -1;
								gameroom[gameroom_index].pop_back();
								gameroom[gameroom_index].pop_back();
								gameroom[gameroom_index][2] = 0;
							}else{
								room[index] = -1;
							}
						}
					}
					exist[i] = false;
					user[i] = "";
					client_socket[i] = 0;
					close(sd);
				}else{
					string command = buffer;
					vector<string> elements;
					elements = getelements(command);
					if(elements[0] == "login"){
						if(elements.size() == 3){
							bool find = false;
							for(int r=0; r<username.size(); r++){
								if(elements[1] == username[r]){
									index = r;
									find = true;
								}
							}
							if(!find){
								strncpy(message, "Username does not exist\n", sizeof(message));
							}else if(exist[i]){
								string faillog = "You already logged in as " + user[i] + "\n";
								strncpy(message, faillog.c_str(), sizeof(message));
							}else{
								if(find && online[index]){
									string faillog = "Someone already logged in as " + username[index] + "\n";
									strncpy(message, faillog.c_str(), sizeof(message));
								}else{
									if(elements[2] != password[index]){
										strncpy(message, "Wrong password\n", sizeof(message));
									}else{
										string success = "Welcome, " + elements[1] + "\n";
										exist[i] = true;
										user[i] = elements[1];
										online[index] = true;
										strncpy(message, success.c_str(), sizeof(message));
									}
								}		
							}
						}else{
							strncpy(message, "Usage: login <username> <password>", sizeof(message));
						}
					}else if(elements[0] == "create" && elements[1] == "public" && elements[2] == "room"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r=0; r<username.size(); r++){
								if(user[i] == username[r]){
									index = r;
								}
							}
							if(room[index] != -1){
								string createroomfail = "You are already in game room " + to_string(room[index]) + ", please leave game room\n";
								strncpy(message, createroomfail.c_str(), sizeof(message));
							}else{
								bool reuse = false;
								for(int j = 0; j < gameroom.size(); j ++){
									if(elements[3] == to_string(gameroom[j][0])) reuse = true;
								}

								if(reuse){
									strncpy(message, "Game room ID is used, choose another one\n", sizeof(message));
								}else{
									string successcreate = "You create public game room " + elements[3] + "\n";
									strncpy(message, successcreate.c_str(), sizeof(message));
									if(stoi(elements[3]) > max_roomid){
										max_roomid = stoi(elements[3]);
									}
									room[index] = stoi(elements[3]);
									vector<int> roominfo;
									roominfo.push_back(stoi(elements[3]));
									roominfo.push_back(-1);
									roominfo.push_back(0);
									roominfo.push_back(index);
									gameroom.push_back(roominfo);
									vector<int> roomorder;
									roomorder.push_back(index);
									joinorder.push_back(roomorder);
									turns.push_back(index);

								}
							}
						}
					}else if(elements[0] == "create" && elements[1] == "private" && elements[2] == "room"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r=0; r<username.size(); r++){
								if(user[i] == username[r]){
									index = r;
								}
							}
							if(room[index] != -1){
								string createroomfail = "You are already in game room " + to_string(room[index]) + ", please leave game room\n";
								strncpy(message, createroomfail.c_str(), sizeof(message));
							}else{
								bool reuse = false;
								for(int j = 0; j < gameroom.size(); j ++){
									if(elements[3] == to_string(gameroom[j][0])) reuse = true;
								}
								if(reuse){
									strncpy(message, "Game room ID is used, choose another one\n", sizeof(message));
								}else{
									string successcreate = "You create private game room " + elements[3] + "\n";
									strncpy(message, successcreate.c_str(), sizeof(message));
									if(stoi(elements[3]) > max_roomid){
										max_roomid = stoi(elements[3]);
									}
									room[index] = stoi(elements[3]);
									vector<int> roominfo;
									roominfo.push_back(stoi(elements[3]));
									roominfo.push_back(stoi(elements[4]));
									roominfo.push_back(0);
									roominfo.push_back(index);
									gameroom.push_back(roominfo);
									vector<int> roomorder;
									roomorder.push_back(index);
									joinorder.push_back(roomorder);
									turns.push_back(index);
								}
							}
						}
					}else if(elements[0] == "join" && elements[1] == "room"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r = 0; r < username.size(); r ++){
								if(user[i] == username[r]){
									index = r;
								}
							}
							if(room[index] != -1){
								string joinroomfail = "You are already in game room " + to_string(room[index]) + ", please leave game room\n";
								strncpy(message, joinroomfail.c_str(), sizeof(message));
							}else{
								bool valid = false;
								int gameroom_index;
								for(int j = 0; j < gameroom.size(); j ++){
									if(elements[2] == to_string(gameroom[j][0])){
										valid = true;
										gameroom_index = j;
									}
								}
								if(!valid){
									string joinroomfail = "Game room " + elements[2] + " is not exist\n";
									strncpy(message, joinroomfail.c_str(), sizeof(message));
								}else{
									if(gameroom[gameroom_index][1] != -1){
										strncpy(message, "Game room is private, please join game by invitation code\n", sizeof(message));
									}else if(gameroom[gameroom_index][2] != 0){
										strncpy(message, "Game has started, you can't join now\n", sizeof(message));
									}else{
										string joinsuccess = "You join game room " + to_string(gameroom[gameroom_index][0]) + "\n";
										room[index] = gameroom[gameroom_index][0];
										joinorder[gameroom_index].push_back(index);
										strncpy(message, joinsuccess.c_str(), sizeof(message));

										for(int j = 0; j < room.size(); j ++){
											if(j != index && room[j] == gameroom[gameroom_index][0]){
												for(int k = 0; k < user.size(); k ++){
													if(username[j] == user[k]){
														int SD = client_socket[k];
														string broadcast =  "Welcome, " + username[index] + " to game!\n";
														strncpy(message_to, broadcast.c_str(), sizeof(message_to));
														write(SD, (const char*)message_to, strlen(message_to));
													}
												}
											}
										}
									}
								}
								
							}
						}
					}else if(elements[0] == "invite"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r = 0; r < username.size(); r ++){
								if(user[i] == username[r]){
									index = r;
								}
							}

							int invitee_index = -1;
							for(int r = 0; r < username.size(); r ++){
								if(elements[1] == email[r]){
									invitee_index = r;
								}
							}

							if(room[index] == -1){
								strncpy(message, "You did not join any game room\n", sizeof(message));
							}else{
								bool manager = false;
								int gameroom_index;
								for(int j = 0; j < gameroom.size(); j ++){
									if(room[index] == gameroom[j][0]){
										if(index == gameroom[j][3]){
											manager = true;
										}
										gameroom_index = j;
									}
								}
								if(!manager){
									strncpy(message, "You are not private game room manager\n", sizeof(message));
								}else if(!online[invitee_index]){
									strncpy(message, "Invitee not logged in\n", sizeof(message));
								}else{
									string invitesuccess = "You send invitation to " + username[invitee_index] + "<" + email[invitee_index] + ">\n";
									strncpy(message, invitesuccess.c_str(), sizeof(message));
									bool duplicate = false;
									for(int j = 0; j < invitation[invitee_index].size(); j ++){
										if(invitation[invitee_index][j] == index){
											duplicate = true;
										}
									}
									if(!duplicate){
										invitation[invitee_index].push_back(index);
									}

									for(int k = 0; k < user.size(); k ++){
										if(username[invitee_index] == user[k]){
											int SD = client_socket[k];
											string broadcast =  "You receive invitation from " + username[index] + "<" + email[index] + ">\n";
											strncpy(message_to, broadcast.c_str(), sizeof(message_to));
											write(SD, (const char*)message_to, strlen(message_to));
										}
									}


								}
							}
						}
					}else if(elements[0] == "list" && elements[1] == "invitations"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r = 0; r < username.size(); r ++){
								if(user[i] == username[r]){
									index = r;
								}
							}
							string listinvitation = "List invitations\n";
							int cnt = 1;
							
							for(int j = 0; j <= max_roomid; j++){
								for(int k = 0; k < invitation[index].size(); k ++){
									int inviter = invitation[index][k];
									if(room[inviter] == j){
										listinvitation += to_string(cnt) + ". " + username[inviter] + "<" + email[inviter] + ">";
										listinvitation += " invite you to join game room " + to_string(room[inviter]) + ", invitation code is ";
										for(int k = 0; k < gameroom.size(); k ++){
											if(room[inviter] == gameroom[k][0]){
												listinvitation += to_string(gameroom[k][1]);
											}
										}
										listinvitation += "\n";
										cnt ++;
									}
								}
							}
							

							if(cnt == 1){
								listinvitation += "No Invitations\n";
							}
							strncpy(message, listinvitation.c_str(), sizeof(message));
						}
					}else if(elements[0] == "accept"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r=0; r<username.size(); r++){
								if(user[i] == username[r]){
									index = r;
								}
							}
							if(room[index] != -1){
								string joinroomfail = "You are already in game room " + to_string(room[index]) + ", please leave game room\n";
								strncpy(message, joinroomfail.c_str(), sizeof(message));
							}else{
								bool noinvitation = true;
								for(int j = 0; j < invitation[index].size(); j ++){
									if(room[invitation[index][j]] != -1){
										noinvitation = false;
									}
								}
								if(noinvitation){
									strncpy(message, "Invitation not exist\n", sizeof(message));
								}else{
									string invitermail = elements[1];
									int invitationcode = stoi(elements[2]);
									int inviter_index;
									for(int j = 0; j < email.size(); j ++){
										if(email[j] == invitermail){
											inviter_index = j;
										}
									}
									int correctcode;
									int roomstatus;
									int gameroom_index;
									for(int j = 0; j < gameroom.size(); j ++){
										if(room[inviter_index] == gameroom[j][0]){
											correctcode = gameroom[j][1];
											roomstatus = gameroom[j][2];
											gameroom_index = j;
										}
									}
									if(invitationcode != correctcode){
										strncpy(message, "Your invitation code is incorrect\n", sizeof(message));
									}else if(roomstatus != 0){
										strncpy(message, "Game has started, you can't join now\n", sizeof(message));
									}else{
										string acceptsuccess = "You join game room " + to_string(room[inviter_index]) + "\n";
										strncpy(message, acceptsuccess.c_str(), sizeof(message));
										room[index] = room[inviter_index];
										joinorder[gameroom_index].push_back(index);

										for(int j = 0; j < room.size(); j ++){
											if(j != index && room[j] == gameroom[gameroom_index][0]){
												for(int k = 0; k < user.size(); k ++){
													if(username[j] == user[k]){
														int SD = client_socket[k];
														string broadcast =  "Welcome, " + username[index] + " to game!\n";
														strncpy(message_to, broadcast.c_str(), sizeof(message_to));
														write(SD, (const char*)message_to, strlen(message_to));
													}
												}
											}
										}
										
									}
								}
							}
						}
					}else if(elements[0] == "leave" && elements[1] == "room"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r = 0; r < username.size(); r ++){
								if(user[i] == username[r]){
									index = r;
								}
							}

							if(room[index] == -1){
								strncpy(message, "You did not join any game room\n", sizeof(message));
							}else{
								bool manager = false;
								int gameroom_index;
								int roomstatus;
								for(int j = 0; j < gameroom.size(); j ++){
									if(room[index] == gameroom[j][0]){
										if(index == gameroom[j][3]){
											manager = true;
										}
										roomstatus = gameroom[j][2];
										gameroom_index = j;
									}
								}
								if(manager){
									string leavesuccess = "You leave game room " + to_string(room[index]) + "\n";
									strncpy(message, leavesuccess.c_str(), sizeof(message));

									for(int j = 0; j < room.size(); j ++){
										if(j != index && room[j] == gameroom[gameroom_index][0]){
											for(int k = 0; k < user.size(); k ++){
												if(username[j] == user[k]){
													int SD = client_socket[k];
													string broadcast =  "Game room manager leave game room " + to_string(room[index]) + ", you are forced to leave too\n";
													strncpy(message_to, broadcast.c_str(), sizeof(message_to));
													write(SD, (const char*)message_to, strlen(message_to));
												}
											}
											room[j] = -1;
										}
									}
									room[index] = -1;
									gameroom.erase(gameroom.begin() + gameroom_index);

								}else if(roomstatus != 0){
									string leavesuccess = "You leave game room " + to_string(room[index]) + ", game ends\n";
									strncpy(message, leavesuccess.c_str(), sizeof(message));

									for(int j = 0; j < room.size(); j ++){
										if(j != index && room[j] == gameroom[gameroom_index][0]){
											for(int k = 0; k < user.size(); k ++){
												if(username[j] == user[k]){
													int SD = client_socket[k];
													string broadcast = username[index] + " leave game room " + to_string(room[index]) + ", game ends\n";
													strncpy(message_to, broadcast.c_str(), sizeof(message_to));
													write(SD, (const char*)message_to, strlen(message_to));
												}
											}
										}
									}	
									room[index] = -1;
									gameroom[gameroom_index].pop_back();
									gameroom[gameroom_index].pop_back();
									gameroom[gameroom_index][2] = 0;

								}else{
									string leavesuccess = "You leave game room " + to_string(room[index]) + "\n";
									strncpy(message, leavesuccess.c_str(), sizeof(message));
									for(int j = 0; j < room.size(); j ++){
										if(j != index && room[j] == gameroom[gameroom_index][0]){
											for(int k = 0; k < user.size(); k ++){
												if(username[j] == user[k]){
													int SD = client_socket[k];
													string broadcast = username[index] + " leave game room " + to_string(room[index]) + "\n";
													strncpy(message_to, broadcast.c_str(), sizeof(message_to));
													write(SD, (const char*)message_to, strlen(message_to));
												}
											}
										}
									}
									room[index] = -1;
								}
							}
						}
					}else if(elements[0] == "start" && elements[1] == "game"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r = 0; r < username.size(); r ++){
								if(user[i] == username[r]){
									index = r;
								}
							}

							if(room[index] == -1){
								strncpy(message, "You did not join any game room\n", sizeof(message));
							}else{
								bool ismanager = false;
								int manager;
								int roomstatus;
								int gameroom_index;
								for(int j = 0; j < gameroom.size(); j ++){
									if(room[index] == gameroom[j][0]){
										if(index == gameroom[j][3]){
											ismanager = true;
										}
										manager = gameroom[j][3];
										gameroom_index = j;
										roomstatus = gameroom[j][2];
									}
								}

								if(!ismanager){
									strncpy(message, "You are not game room manager, you can't start game\n", sizeof(message));
								}else if(roomstatus != 0){
									strncpy(message, "Game has started, you can't start again\n", sizeof(message));
								}else{
									if(elements.size() == 3){
										gameroom[gameroom_index].push_back(random(1000, 9000));
										gameroom[gameroom_index].push_back(stoi(elements[2]));
										gameroom[gameroom_index][2] = 1;
										string startsuccess = "Game start! Current player is " + username[manager] + "\n";
										strncpy(message, startsuccess.c_str(), sizeof(message));
										for(int j = 0; j < room.size(); j ++){
											if(j != index && room[j] == gameroom[gameroom_index][0]){
												for(int k = 0; k < user.size(); k ++){
													if(username[j] == user[k]){
														int SD = client_socket[k];
														string broadcast =  "Game start! Current player is " + username[manager] + "\n";
														strncpy(message_to, broadcast.c_str(), sizeof(message_to));
														write(SD, (const char*)message_to, strlen(message_to));
													}
												}
											}
										}
									}else if(not_number(elements[3]) || elements[3].length() != 4){
										strncpy(message, "Please enter 4 digit number with leading zero\n", sizeof(message));
									}else{
										gameroom[gameroom_index].push_back(stoi(elements[3]));
										gameroom[gameroom_index].push_back(stoi(elements[2]));
										gameroom[gameroom_index][2] = 1;
										string startsuccess = "Game start! Current player is " + username[manager] + "\n";
										strncpy(message, startsuccess.c_str(), sizeof(message));
										for(int j = 0; j < room.size(); j ++){
											if(j != index && room[j] == gameroom[gameroom_index][0]){
												for(int k = 0; k < user.size(); k ++){
													if(username[j] == user[k]){
														int SD = client_socket[k];
														string broadcast =  "Game start! Current player is " + username[manager] + "\n";
														strncpy(message_to, broadcast.c_str(), sizeof(message_to));
														write(SD, (const char*)message_to, strlen(message_to));
													}
												}
											}
										}
									}
								}
							}
						}
					
					}else if(elements[0] == "guess"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r = 0; r < username.size(); r ++){
								if(user[i] == username[r]){
									index = r;
								}
							}

							if(room[index] == -1){
								strncpy(message, "You did not join any game room\n", sizeof(message));
							}else{
								bool manager = false;
								int gameroom_index;
								for(int j = 0; j < gameroom.size(); j ++){
									if(room[index] == gameroom[j][0]){
										if(index == gameroom[j][3]){
											manager = true;
										}
										gameroom_index = j;
									}
								}

								if(manager && gameroom[gameroom_index][2] == 0){
									strncpy(message, "You are game room manager, please start game first\n", sizeof(message));
								}else if(!manager && gameroom[gameroom_index][2] == 0){
									strncpy(message, "Game has not started yet\n", sizeof(message));
								}else if(index != turns[gameroom_index]){
									int currentplayerindex = joinorder[gameroom_index][turns[gameroom_index]];
									string notyourturn = "Please wait..., current player is " + username[currentplayerindex] + "\n";
									strncpy(message, notyourturn.c_str(), sizeof(message));
								}else if(not_number(elements[1]) || elements[1].length() != 4){
									strncpy(message, "Please enter 4 digit number with leading zero\n", sizeof(message));
								}else{
									bool gameend = false;
									int numofpeople = joinorder[gameroom_index].size();
									if(turns[gameroom_index] + 1 >= numofpeople){
										gameroom[gameroom_index][5] --;
										if(gameroom[gameroom_index][5] == 0){
											gameend = true;
										}
									}
									turns[gameroom_index] = (turns[gameroom_index] + 1) % numofpeople;
									
									string currentanswer;
									if(to_string(gameroom[gameroom_index][4]).size() == 3){
										currentanswer = "0" + to_string(gameroom[gameroom_index][4]);
									}else if(to_string(gameroom[gameroom_index][4]).size() == 2){
										currentanswer = "00" + to_string(gameroom[gameroom_index][4]);
									}else if(to_string(gameroom[gameroom_index][4]).size() == 1){
										currentanswer = "000" + to_string(gameroom[gameroom_index][4]);
									}else if(to_string(gameroom[gameroom_index][4]).size() == 0){
										currentanswer = "0000";
									}else{
										currentanswer = to_string(gameroom[gameroom_index][4]);
									}

									int cntA = 0;
									int cntB = 0;
									bool availableA[4] = {true, true, true, true};
									bool availableB[4] = {true, true, true, true};
									for(int j = 0; j < 4; j ++){
										if(elements[1][j] == currentanswer[j]){
											cntA ++;
											availableA[j] = false;
											availableB[j] = false;
										}
									}

									for(int j = 0; j < 4; j ++){
										for(int k = 0; k < 4; k ++){
											if(elements[1][k] == currentanswer[j]){
												if(j != k && availableA[j] && availableB[k]){
													cntB ++;
													availableA[j] = false;
													availableB[k] = false;
												}
											}
										}
									}
									string hint = "'" + to_string(cntA) + "A" + to_string(cntB) + "B'";

									if(cntA == 4){
										string guesssuccess = username[index] + " guess '" + elements[1] +"' and got Bingo!!! " + username[index] + " wins the game, game ends\n";
										strncpy(message, guesssuccess.c_str(), sizeof(message));

										for(int j = 0; j < room.size(); j ++){
											if(j != index && room[j] == gameroom[gameroom_index][0]){
												for(int k = 0; k < user.size(); k ++){
													if(username[j] == user[k]){
														int SD = client_socket[k];
														strncpy(message_to, guesssuccess.c_str(), sizeof(message_to));
														write(SD, (const char*)message_to, strlen(message_to));
													}
												}
											}
										}
										gameroom[gameroom_index][2] = 0;
										gameroom[gameroom_index].pop_back();
										gameroom[gameroom_index].pop_back();

									}else if(gameend){
										string guesssuccess = username[index] + " guess '" + elements[1] +"' and got " + hint + "\n" + "Game ends, no one wins\n";
										strncpy(message, guesssuccess.c_str(), sizeof(message));

										for(int j = 0; j < room.size(); j ++){
											if(j != index && room[j] == gameroom[gameroom_index][0]){
												for(int k = 0; k < user.size(); k ++){
													if(username[j] == user[k]){
														int SD = client_socket[k];
														strncpy(message_to, guesssuccess.c_str(), sizeof(message_to));
														write(SD, (const char*)message_to, strlen(message_to));
													}
												}
											}
										}
										gameroom[gameroom_index][2] = 0;
										gameroom[gameroom_index].pop_back();
										gameroom[gameroom_index].pop_back();

									}else{
										string guesssuccess = username[index] + " guess '" + elements[1] +"' and got " + hint + "\n";
										strncpy(message, guesssuccess.c_str(), sizeof(message));

										for(int j = 0; j < room.size(); j ++){
											if(j != index && room[j] == gameroom[gameroom_index][0]){
												for(int k = 0; k < user.size(); k ++){
													if(username[j] == user[k]){
														int SD = client_socket[k];
														strncpy(message_to, guesssuccess.c_str(), sizeof(message_to));
														write(SD, (const char*)message_to, strlen(message_to));
													}
												}
											}
										}
									}
								}
							}
						}
					}else if(elements[0] == "logout"){
						if(!exist[i]){
							strncpy(message, "You are not logged in\n", sizeof(message));
						}else{
							for(int r=0; r<username.size(); r++){
								if(user[i] == username[r]){
									index = r;
								}
							}
							if(room[index] != -1){
								string logoutfail = "You are already in game room " + to_string(room[index]) + ", please leave game room\n";
								strncpy(message, logoutfail.c_str(), sizeof(message));
							}else{
								online[index] = false;
								exist[i] = false;
								string logout = "Goodbye, " + user[i] + "\n";
								user[i] = "";
								strncpy(message, logout.c_str(), sizeof(message));
							}
						}
					}else if(elements[0] == "exit"){
						for(int r=0; r<username.size(); r++){
							if(user[i] == username[r]){
								index = r; 
								break;
							}
						}
						if(index != -1){
							online[index] = false;
							if(room[index] != -1){
								bool manager = false;
								int gameroom_index;
								int roomstatus;
								for(int j = 0; j < gameroom.size(); j ++){
									if(room[index] == gameroom[j][0]){
										if(index == gameroom[j][3]){
											manager = true;
										}
										roomstatus = gameroom[j][2];
										gameroom_index = j;
									}
								}
								if(manager){
									for(int j = 0; j < room.size(); j ++){
										if(j != index && room[j] == room[index]){
											room[j] = -1;
										}
									}
									room[index] = -1;
									gameroom.erase(gameroom.begin() + gameroom_index);
								}else if(roomstatus != 0){
									room[index] = -1;
									gameroom[gameroom_index].pop_back();
									gameroom[gameroom_index].pop_back();
									gameroom[gameroom_index][2] = 0;
								}else{
									room[index] = -1;
								}
							}
						}
						exist[i] = false;
						user[i] = "";
						client_socket[i] = 0;
						close(sd);
					}
					//puts(buffer);
					write(sd, (const char*)message, strlen(message));
				}
            }
		}
    
		 
		// UDP
		if (FD_ISSET(udpfd, &rset)) {
			strncpy(message, "UDP", sizeof(message));
			len = sizeof(cliaddr);
			bzero(buffer, sizeof(buffer));
			//cout << "UDP server is running." << endl;
			n = recvfrom(udpfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);
			string command = buffer;
			vector<string> elements;
			elements = getelements(command);
			if(elements[0] == "New"){
				cout << "New connection" << "\n";
				string str = "Connected to server";
				strncpy(message, str.c_str(), sizeof(message));
			}else if(elements[0] == "register"){
				bool error = false;
				if(elements.size() == 4){
					for(int i=0; i<username.size(); i++){
						if(username[i] == elements[1] || email[i] == elements[2]){
							strncpy(message, "Username or Email is already used\n", sizeof(message));
							error = true;
							break;
						}
					}
					if(!error){
						username.push_back(elements[1]);
						email.push_back(elements[2]);
						password.push_back(elements[3]);
						strncpy(message, "Register Successfully\n", sizeof(message));
					}
				}
			}else if(elements[0] == "list" && elements[1] == "rooms"){
				string gamerooms = "List Game Rooms\n";
				int cnt = 1;
				for(int i = 0; i <= max_roomid; i ++){
					for(int j = 0; j < gameroom.size(); j ++){
						if(gameroom[j][0] == i){
							gamerooms += to_string(cnt) + ". (";
							if(gameroom[j][1] == -1){
								gamerooms += "Public) Game Room " + to_string(gameroom[j][0]);
							}else{
								gamerooms += "Private) Game Room " + to_string(gameroom[j][0]);
							}
							if(gameroom[j][2] == 0){
								gamerooms += " is open for players\n";
							}else{
								gamerooms += "  has started playing\n";
							}
							cnt ++;
						}
					}

				}
				if(cnt == 1){
					gamerooms += "No Rooms\n";
				}
				//gamerooms.erase(gamerooms.end() -1);
				strncpy(message, gamerooms.c_str(), sizeof(message));
			}else if(elements[0] == "list" && elements[1] == "users"){
				vector<string> info;
				for(int i = 0; i < username.size(); i ++){
					string tmp = username[i] + "<" + email[i] + ">" + " ";
					if(online[i]){
						tmp += "Online";
					}else{
						tmp += "Offline";
					}
					info.push_back(tmp);
				}
				sort(info.begin(), info.end());
				string listusers = "List Users\n";
				int cnt = 1;
				for(int i = 0; i < info.size(); i ++){
					listusers += to_string(cnt) + ". " + info[i] + "\n";
					cnt ++;
				}
				if(cnt == 1){
					listusers += "No Users\n";
				}
				//listusers.erase(listusers.end() -1);
				strncpy(message, listusers.c_str(), sizeof(message));
			}

			//puts(buffer);
			sendto(udpfd, (const char*)message, strlen(message), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
		}
	}
}

vector<string> getelements(string command){

	string cmd;
	int cnt = 0;
	int pre = 0;
	vector<string> a;
	for(int i=0 ; i<command.length() ; i++){
		if(command[i] == ' '){
			a.push_back(command.substr(pre, i - pre));
			pre = i+1;
			cnt ++;
		}
	}
	a.push_back(command.substr(pre, command.length()));
	if(a[a.size() - 1].back() == '\n'){
		a[a.size() - 1].pop_back();
	}
	return a;
}

string read_file(string filename){
	ifstream myfile; 
	myfile.open(filename);
	string content;
	if(myfile.is_open()){
		while(myfile){
			content += myfile.get();
		}
	}
	content.pop_back();
	return content;
}

int random(int low, int high){
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}

bool not_number(const string& s){
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !(!s.empty() && it == s.end());
}
