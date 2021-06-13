#include <iostream>
#include <set>
#include <unordered_map>
#include <unistd.h>
#include <vector>

#define MAXREQ 36

#define RED  "\x1B[31m"
#define RESET "\x1B[0m"
#define SPADES "\xE2\x99\xA0"
#define CLUBS "\xE2\x99\xA3"
#define HEARTS "\xE2\x99\xA5"
#define DIAMONDS "\xE2\x99\xA6"

using namespace std;

class Card{
    public:
    char suite;
    char num;
    Card(char s, int n){
        suite = s;
        
        if(n==1) num = 'A';
        else if(n==10) num = 'X'; // find a workaround
        else if(n==11) num = 'J';
        else if(n==12) num = 'Q';
        else if(n==13) num = 'K';
        else num = (char)n + '0';
    }

    // void print(){
    //     if(this->suite=='h')
    //         printf( RED HEARTS " %c  " RESET, this->num );
    //     else if(this->suite=='d')
    //         printf( RED DIAMONDS " %c  " RESET, this->num );
    //     else if(this->suite=='s')
    //         printf( SPADES " %c  ", this->num );
    //     else
    //         printf( CLUBS " %c  ", this->num );
    //     return;
    // }
};

// bool isOpened[52]; // flop, turn, river

// set<Card*> pack_cards;  // the ones still in pack -> unfolded
// set<Card*> table_cards;  // the ones still in pack -> unfolded
// set<Card*> p1_cards;  // the ones with player 1
// set<Card*> p2_cards;  // the ones with player 2
// set<Card*> p3_cards;  // the ones with player 3
// set<Card*> p4_cards;  // the ones with player 4


void initPack(set<Card*> &pack_cards){
    // clear pack if set initially
    char S[] = {'s', 'h', 'c', 'd'};
    for(int i=0;i<4;i++){
        for(int j=1;j<=13;j++){
            Card* c = new Card(S[i], j);
            pack_cards.insert(c);
        }
    }
    return;
}

void sendMessage(int &fd, string s){
	char* arr = new char[s.size() + 1];
	strcpy(arr, s.c_str());
	write(fd, arr, strlen(arr));
	delete[] arr;
}

void printPack(set<Card*> &pack_cards){
    set<Card*> :: iterator it;
    for(it = pack_cards.begin(); it != pack_cards.end();it++){
        // (*it)->print();
    }
    return;
}

void printTable(set<Card*> &table_cards, int &fd){
    set<Card*> :: iterator it;
    for(it = table_cards.begin(); it != table_cards.end();it++){
        char suite = (*it)->suite;
        char num = (*it)->num;
        string message = "";
        message += to_string(suite);
        message += to_string(num);
        sendMessage(fd, message);
    }
    return;
}

void pickCards(set<Card*> &pack_cards, set<Card*> &p_cards)
{
    int randNum;
    Card* randCard;
    for(int i = 0;i < 2;i++) 
    {
        randNum = rand() % pack_cards.size();
        randCard = *next(pack_cards.begin(), randNum);
        pack_cards.erase(randCard);
        p_cards.insert(randCard);
    }
    return;
}

void initPlayers(set<Card*> &pack_cards, set<Card*> &p1_cards, set<Card*> &p2_cards, set<Card*> &p3_cards, set<Card*> &p4_cards){ // modularize
    // for(set<Card*> &p_cards : {p1_cards, p2_cards, p3_cards, p4_cards})
    pickCards(pack_cards, p1_cards);
    pickCards(pack_cards, p2_cards);
    pickCards(pack_cards, p3_cards);
    pickCards(pack_cards, p4_cards);
    return;
}

void printPlayer(set<Card*> &player_cards, int &fd){
    set<Card*> :: iterator it;
    for(it = player_cards.begin(); it != player_cards.end();it++){
        char suite = (*it)->suite;
        char num = (*it)->num;
        string message = "";
        message += to_string(suite);
        message += to_string(num);
        sendMessage(fd, message);
    }
    return;
}

void play_blind(unordered_map<string, int> &player_amount, int &fd){
    int blind_amount = 100;
    unordered_map<string, int> :: iterator it;
    for (it = player_amount.begin(); it != player_amount.end(); it++) {
        it->second -= blind_amount;
    }
    sendMessage(fd, "Blind placed: 100 coins");
}

void open_cards(int num_cards, set<Card*> &pack_cards, set<Card*> &table_cards){
    for(int i = 0; i < num_cards; i++){
        int randNum = rand() % pack_cards.size();
        Card* randCard = *next(pack_cards.begin(), randNum);
        pack_cards.erase(randCard);
        table_cards.insert(randCard);
    }
    return;
}

void play_Round(int roundN, set<Card*> &pack_cards, set<Card*> &table_cards, vector<int> &list_fd, unordered_map<string, int> &player_amount, unordered_map<int, string> fd_user_map){
    // print flop
    if(roundN == 1)
        open_cards(3, pack_cards, table_cards);

    else
        open_cards(1, pack_cards, table_cards);
    
    for (int fd : list_fd){
        printTable(table_cards, fd);
    }


    // player 1
    int player_in_action = list_fd[0]; // ++  %list_fd.size()
    sendMessage(player_in_action, "Place your bet");

    char reqbuf[MAXREQ];
    memset(reqbuf, 0, MAXREQ);
	read(player_in_action, reqbuf, MAXREQ-1);

	string msg = string(reqbuf);
    msg.pop_back();
    int bet = stoi(msg);

    // player 2 3 4
    // loop
    for(int i = 1; i < list_fd.size(); i++){
        player_in_action = list_fd[i];
        string clientMsg = "Present bet = " + to_string(bet) + "\nMake a choice:\nc:Call\nr:Raise <bet>\nf:Fold\n";
        sendMessage(player_in_action, clientMsg);
        
        memset(reqbuf, 0, MAXREQ);
        read(player_in_action, reqbuf, MAXREQ-1);

        msg = string(reqbuf);
        msg.pop_back();
        
        if(msg[0] == 'c'){
            player_amount[fd_user_map[player_in_action]] -= bet;
        }
        else if (msg[0] == 'f') {
            list_fd.erase(list_fd.begin() + i);
            i--;
        }
        else{
            char* arr = new char[msg.size() + 1];
	        strcpy(arr, msg.c_str());
            char* reply = strtok(arr, " ");
            bet = stoi(to_string(reply[1]));
            player_amount[fd_user_map[player_in_action]] -= bet;
        }
    }

    if (roundN == 3){
        // judge(); // args : p1, p2, p3 => "1 or 2 or 3"; winner_amount += pot ; pot=0;
    }

}

// loop(init => blind => flop + play_round1 => turn + play r2 => river + play r3 => judgement)

// int main(){
//     initPack();
//     initPlayers();
//     set<Card*> :: iterator it;
    
//     // play_blind();

//     return 0;
// }