#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <bits/stdc++.h>
#include <pthread.h>
#include <fstream>
#include <cstdint>

#define MAXREQ 36
#define MAXQUEUE 5

using namespace std;

unordered_map<string, unordered_set<string>> allRooms;
unordered_map<string, unordered_set<string>> availableRooms;

unordered_map<pthread_t, pthread_cond_t> cond_map; 
pthread_mutex_t mLock = PTHREAD_MUTEX_INITIALIZER; 

unordered_map<int, string> fd_user_map;
unordered_map<string, int> user_fd_map;
unordered_map<thread::id, string> thread_users;
unordered_map<string, thread::id> user_threads;
unordered_set<string> active_users;
unordered_set<string> available_users;

void sendMessage(int &fd, string s)
{
  char* arr = new char[s.size() + 1];
  strcpy(arr, s.c_str());
  write(fd, arr, strlen(arr));
  delete[] arr;
}

void sendMsgToUser(string s, string t)
{
  int fd = user_fd_map[t];
  string sender = thread_users[this_thread::get_id()];

  string res = "FROM " + sender + ": " + s + "\n";
  sendMessage(fd, res);
}

void closeSocket(int &fd)
{
  string user = fd_user_map[fd];
  active_users.erase(user);
  fd_user_map.erase(fd);
  user_fd_map.erase(user);
  thread_users.erase(this_thread::get_id());
  user_threads.erase(user);
  available_users.erase(user);

  string closingMsg = "Socket has been closed.\n";
  sendMessage(fd, closingMsg);
  close(fd);
  // terminate();
}

void allAvailableRooms(int fd, string username)
{
  return;
}

void* serveClient(void* fd) 
{
  int consockfd = *((int *)fd);

  int n;
  char reqbuf[MAXREQ];

  n = read(consockfd, reqbuf, MAXREQ-1);
  
  string username = string(reqbuf) + "::" + to_string(rand() % 1000);

  fd_user_map[consockfd] = username;
  user_fd_map[username] = consockfd;
  active_users.insert(username);
  available_users.insert(username);
  thread_users[this_thread::get_id()] = username;
  user_threads[username] = this_thread::get_id();


  string instructionMsg = "1 Join a Random Room\n2 See all available Rooms\n3 Create a New Room\nq Quit\n";
  string welcomeMsg = "Welcome " + username + " to Arcade Studio\n\n" + instructionMsg;
  sendMessage(consockfd, welcomeMsg);

  while (1)
  {
    memset(reqbuf, 0, MAXREQ);
    n = read(consockfd, reqbuf, MAXREQ-1);

    string msg = string(reqbuf);
    cout << msg << "\n";

    if (msg == "CLOSE\n")
      closeSocket(consockfd);
    else if (msg == "1\n")
    {
      available_users.erase(username);
      // joinRandomRoom(consockfd);
      available_users.insert(username);
      sendMessage(consockfd, instructionMsg);
      cout << msg;
      continue;
    }
    else if (msg == "2\n")
    {
      allAvailableRooms(consockfd, username);
      sendMessage(consockfd, instructionMsg);
    }
    else if (msg == "3\n")
    {
      available_users.erase(username);
      // createNewRoom(consockfd);
      available_users.insert(username);
      sendMessage(consockfd, instructionMsg);
    }
    else if (msg == "q\n" || msg == "Q\n")
    {
      cout << "clicked";
      closeSocket(consockfd);
      return NULL;
    }
    else
    {
      sendMessage(consockfd, instructionMsg);
      // continue;
    }
  }

  return NULL;
}

int main() 
{
  int lstnsockfd, consockfd, portno = 4321;
  unsigned int clilen;
  struct sockaddr_in serv_addr, cli_addr;

  memset((char *) &serv_addr,0, sizeof(serv_addr));
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port        = htons(portno);

  lstnsockfd = socket(AF_INET, SOCK_STREAM, 0);

  bind(lstnsockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
  printf("\nBounded to port\n");

  thread tid[99];
  int tcounter = 0;
  printf("\nListening for incoming connections\n");

  while (1) 
  {
    listen(lstnsockfd, MAXQUEUE); 

    int newsocketfd = accept(lstnsockfd, (struct sockaddr *) &cli_addr, &clilen);

    tid[tcounter] = thread(serveClient, &newsocketfd);

    tcounter++;

    printf("Accepted connection\n");
  }
  close(lstnsockfd);
}
