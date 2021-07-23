// # -----------------------------------------------------------
// # Project Chat Room
// # __________________
// #
// #  All Rights Reserved.
// #
// # NOTICE:  All information contained herein is, and remains
// # the property of Omesh Sharma and its suppliers,
// # if any.  The intellectual and technical concepts contained
// # herein are proprietary to Omesh Sharma
// # and its suppliers and may be covered by Indian and Foreign Patents,
// # trade or copyright law.
// # patents in process, Indian Patents Act 1970, and are protected by secret
// # Dissemination of this information or reproduction of this material
// # is strictly forbidden unless prior written permission is obtained
// # from Omesh Sharma.
// # -----------------------------------------------------------

// # -----------------------------------------------------------------
// #   Class:            Client.cpp.
// #   Description:      This Source file will send the mesage to server
// #   Owner :           Omesh Sharma
// #   Author:           Omesh Sharma
// #   Created On :      23 JUL 9:00 AM 2021
// #   Notes:            NULL
// #   Revision History: NULL
// #   Name              Date.            Description
// #
// #   you may not use this file except in compliance with the License.
// #
// # -----------------------------------------------------------------

// # -----------------------------------------------------------
// #                       USAGE
// #                        NULL
// # -----------------------------------------------------------

/*--------------------------------------------------------------
Header File Loading Start
--------------------------------------------------------------*/

#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <mutex>
#define MAX_LEN 200
#define NUM_COLORS 6

/*--------------------------------------------------------------
Header File Loading End
--------------------------------------------------------------*/

/*
// The using namespace statement just means that in the scope it is present,
   make all the things under the std namespace available without having to prefix std:: before each of them.
*/

using namespace std;

// Bool flag to check interrupt signal catched status
bool exit_flag=false;

// Threadin object defined for sending  and receiving.
thread t_send, t_recv;

int client_socket;

// Set Color code to print the cout stream in the colored text.
string def_col="\033[0m";
string colors[]={"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};

// This Funtion will be called in case of any INTERRUPT signal catched.
void catch_ctrl_c(int signal);

// Print the specified color from the array of colors[]  with passed index as a argument.
string color(int code);

// Clear the text from the terminal
int eraseText(int cnt);

// Function to send the message to server
void send_message(int client_socket);

// Function to recieve the message from the server.
void recv_message(int client_socket);


// Main function starts
int main()
{	
	// Checking Address family and socket stream validation.
	/*
	SOCK_STREAM means that it is a TCP socket. SOCK_DGRAM means that it is a UDP socket.
	These are used 99% of the time F_INET is an address family that is used to designate
	the type of addresses that your socket can communicate with (in this case, Internet Protocol v4 addresses)
	*/
	if((client_socket=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket: ");
		exit(-1);
	}

	// passing IP address and port number
	struct sockaddr_in client;
	client.sin_family=AF_INET;
	client.sin_port=htons(10000); // Port no. of server
	client.sin_addr.s_addr=INADDR_ANY;
	//client.sin_addr.s_addr=inet_addr("127.0.0.1"); // Provide IP address of server

	
	/*
	The bzero() function erases the data in the n bytes of the memory
    starting at the location pointed to by s, by writing zeros (bytes
    containing '\0') to that area.
	*/
	bzero(&client.sin_zero,0);

	// Checking the ipaddress, and port number are active or available. Also checking server availability.
	if((connect(client_socket,(struct sockaddr *)&client,sizeof(struct sockaddr_in)))==-1)
	{
		perror("connect: ");
		exit(-1);
	}

	// Run Signal in async to watch any termination interrupt from user.
	signal(SIGINT, catch_ctrl_c);

	// Declare the array of character.
	char name[MAX_LEN];
	cout<<"Enter your name : ";

	cin.getline(name,MAX_LEN);

	// Send the request to server so that they can register you with your name.
	send(client_socket,name,sizeof(name),0);

	// Main chat window started.
	cout<<colors[NUM_COLORS-1]<<"\n\t  ====== Welcome to the chat-room ======   "<<endl<<def_col;

	// Run threads to run in loop untill the end of program. so that sending and recieving message can be handle in sync.
	thread t1(send_message, client_socket);
	thread t2(recv_message, client_socket);

	//Move the one thread object to another.
	/*
	std::move is used to indicate that an object t may be "moved from", i.e. 
	allowing the efficient transfer of resources from t to another object.
	In particular, std::move produces an xvalue expression that identifies 
	its argument t. It is exactly equivalent to a static_cast to an rvalue reference type.
	*/
	t_send=move(t1);
	t_recv=move(t2);

	// Waiting for the  the thread to get complete.
	if(t_send.joinable())
		t_send.join();
	if(t_recv.joinable())
		t_recv.join();
			
	return 0;
}

// Handler for "Ctrl + C"
void catch_ctrl_c(int signal) 
{
	char str[MAX_LEN]="#exit";
	send(client_socket,str,sizeof(str),0);
	exit_flag=true;
	t_send.detach();
	t_recv.detach();
	close(client_socket);
	exit(signal);
}

string color(int code)
{
	return colors[code%NUM_COLORS];
}

// Erase text from terminal
int eraseText(int cnt)
{
	char back_space=8;
	for(int i=0; i<cnt; i++)
	{
		cout<<back_space;
	}	
}

// Send message to everyone
void send_message(int client_socket)
{
	while(1)
	{
		cout<<colors[1]<<"You : "<<def_col;
		char str[MAX_LEN];
		cin.getline(str,MAX_LEN);
		send(client_socket,str,sizeof(str),0);
		if(strcmp(str,"#exit")==0)
		{
			exit_flag=true;
			t_recv.detach();	
			close(client_socket);
			return;
		}	
	}		
}

// Receive message
void recv_message(int client_socket)
{
	while(1)
	{
		if(exit_flag)
			return;
		char name[MAX_LEN], str[MAX_LEN];
		int color_code;
		int bytes_received=recv(client_socket,name,sizeof(name),0);
		if(bytes_received<=0)
			continue;
		recv(client_socket,&color_code,sizeof(color_code),0);
		recv(client_socket,str,sizeof(str),0);
		eraseText(6);
		if(strcmp(name,"#NULL")!=0)
			cout<<color(color_code)<<name<<" : "<<def_col<<str<<endl;
		else
			cout<<color(color_code)<<str<<endl;
		cout<<colors[1]<<"You : "<<def_col;
		fflush(stdout);
	}	
}
