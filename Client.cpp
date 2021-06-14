// Client.c
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <thread>
#include <iostream>

#define MAXIN 20
#define MAXSE 36
#define MAXOUT 1024
#define FLUSH “\e[1;1H\e[2J”
#define RED  "\x1B[31m"
#define RESET "\x1B[0m"
#define SPADES "\xE2\x99\xA0"
#define CLUBS "\xE2\x99\xA3"
#define HEARTS "\xE2\x99\xA5"
#define DIAMONDS "\xE2\x99\xA6"

using namespace std;

int sockfd;

thread rThread, wThread;

void parseAndDisplay(string s){
	if(s[0]=='^'){
    cout.flush();
		char suite=s[1], num=s[2];

		if(suite=='h')
			cout << ( RED HEARTS ) << num << RESET << " ";
		else if(suite=='d')
			cout << ( RED DIAMONDS ) << num << RESET << " ";
		else if(suite=='s')
			cout << ( SPADES ) << num << RESET << " ";
		else
			cout << ( CLUBS ) << num << RESET << " ";
    cout.flush();
    // printf("\n");
		return;
	}

	else
		// cout << s << "\n";
    printf("%s\n", s.c_str());
}

void displayMessage(int consockfd) 
{
  int n;
  char rcvbuf[MAXOUT];
  while (1) 
  {
    memset(rcvbuf, 0, MAXOUT);               /* clear */
    n = read(sockfd, rcvbuf, MAXOUT - 1);  

    if (n <= 0)
      return;

    cout << string(rcvbuf) << endl;
    // parseAndDisplay(string(rcvbuf));
  }

  return;
}


void sendMessage(int fd, string s)
{
  if (s == "")
    return;

  char* arr = new char[s.size() + 1];
  strcpy(arr, s.c_str());
  write(fd, arr, strlen(arr));
  delete[] arr;
}


void readAndSendData(int consockfd)
{
  int n;
  char payload[MAXIN];
  char tempbuf[MAXIN];

  while (1)
  {
    string s;
    cin >> s;

    sendMessage(consockfd, s);

    if (n <= 0)
      return;
  }
}

void initialConnect(string username)
{

  // int n = write(sockfd, username, strlen(username));
  sendMessage(sockfd, username);

  rThread = thread(displayMessage, sockfd);
  readAndSendData(sockfd);

  rThread.join();
  // wThread.join();
}

void closeHandler(int sig)
{
  string s = "CLOSE";

  sendMessage(sockfd, s);

  cout << "\nPlease wait for 2 Seconds\n";
  sleep(2);

  close(sockfd);
  exit(0);
}

// Server address
struct hostent *buildServerAddr(struct sockaddr_in *serv_addr, char *serverIP, int portno) 
{
  /* Construct an address for remote server */
  memset((char *) serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr->sin_family = AF_INET;
  inet_aton(serverIP, &(serv_addr->sin_addr));
  serv_addr->sin_port = htons(portno);
}


int main() 
{
  signal(SIGINT, closeHandler);
	//Client protocol
	// char serverIP[] = "192.168.29.51";
  char serverIP[] = "127.0.0.1";
	int portno = 9000;
	struct sockaddr_in serv_addr;
	
	buildServerAddr(&serv_addr, serverIP, portno);

	/* Create a TCP socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* Connect to server on port */
	connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	// printf("Connected to %s:%d\n",serverIP, portno);

  char userID[12];
  printf("%s: ", "Enter Name");
  scanf("%s", userID);

	/* Carry out Client-Server protocol */
  initialConnect(userID);
	/* Clean up on termination */
	// close(sockfd);
}