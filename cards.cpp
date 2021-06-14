#include <iostream>
#include <set>
#include <unordered_map>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <string>

#define MAXREQ 36

#define RED  "\x1B[31m"
#define RESET "\x1B[0m"
#define SPADES "\xE2\x99\xA0"
#define CLUBS "\xE2\x99\xA3"
#define HEARTS "\xE2\x99\xA5"
#define DIAMONDS "\xE2\x99\xA6"
char* FLUSH = "\e[1;1H\e[2J";

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

    void print(){
        if(this->suite=='h')
            printf( RED HEARTS " %c  " RESET, this->num );
        else if(this->suite=='d')
            printf( RED DIAMONDS " %c  " RESET, this->num );
        else if(this->suite=='s')
            printf( SPADES " %c  ", this->num );
        else
            printf( CLUBS " %c  ", this->num );
        return;
    }
};

// bool isOpened[52]; // flop, turn, river

set<Card*> pack_cards;  // the ones still in pack -> unfolded
// set<Card*> table_cards;  // the ones still in pack -> unfolded
// set<Card*> p1_cards;  // the ones with player 1
// set<Card*> p2_cards;  // the ones with player 2
// set<Card*> p3_cards;  // the ones with player 3
// set<Card*> p4_cards;  // the ones with player 4


size_t split(string txt, vector<string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}

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
        (*it)->print();
    }
    return;
}

void printTable(set<Card*> &table_cards, vector<int> &list_fd){
    set<Card*> :: iterator it;
    for(it = table_cards.begin(); it != table_cards.end(); it++){
        char suite = (*it)->suite;
        char num = (*it)->num;
        string message = "";
        message.push_back('|'); // to signify printing a card
        message.push_back(suite);
        message.push_back(num);
        message.push_back(' ');
        message.push_back('|'); // to signify printing a card
        message.push_back('\n');
        // printf("%s\n", message);
        for(auto fd : list_fd){
            sendMessage(fd, message);
        }
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

void play_blind(unordered_map<string, int> &user_fd_map, unordered_map<string, int> &player_amount){
    int blind_amount = 100;
    unordered_map<string, int> :: iterator it;
    for (it = player_amount.begin(); it != player_amount.end(); it++) {
        it->second -= blind_amount;
        sendMessage(user_fd_map[it->first], FLUSH);
        sendMessage(user_fd_map[it->first], "Blind placed: 100 coins");
    }
}

void printPlayer(set<Card*> &player_cards, int fd){
    set<Card*> :: iterator it;
    for(it = player_cards.begin(); it != player_cards.end();it++){
        char suite = (*it)->suite;
        char num = (*it)->num;
        string message = "";
        message.push_back('|'); // to signify printing a card
        message.push_back(suite);
        message.push_back(num);
        message.push_back('|');
        message.push_back('\n');
        sendMessage(fd, message);
    }
    return;
}

//  modularize
void initPlayers(set<Card*> &pack_cards, unordered_map<int, set<Card*> > &p_cards, unordered_map<string, int> &user_fd_map, unordered_map<string, int> &player_amount, vector<int> &list_fd){
    unordered_map<int, set<Card*> > :: iterator it;
    for(it = p_cards.begin(); it != p_cards.end(); it++){
        pickCards(pack_cards, it->second);
    }

    play_blind(user_fd_map, player_amount);
    for(it = p_cards.begin(); it != p_cards.end(); it++){
        printPlayer(it->second, it->first);
    }
    return;
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

int judge(vector<int> list_fd){
    int idx = rand() % list_fd.size();
    return list_fd[idx];
}

bool play_Round(int roundN, set<Card*> &pack_cards, set<Card*> &table_cards, vector<int> &list_fd,
                unordered_map<string, int> &player_amount, unordered_map<int, string> fd_user_map,
                unordered_map<int, set<Card*>> &p_cards, int &pot){
    
    // Edge case: Only one player left at the table
    if(list_fd.size() == 1) return 0;
    
    // print flop
    if(roundN == 1)
        open_cards(3, pack_cards, table_cards);

    else
        open_cards(1, pack_cards, table_cards);
    
    // printTable(table_cards, list_fd);

    // player 1 -> always the dealer
    int player_in_action = list_fd[0]; // ++  %list_fd.size()

    for(auto fd : list_fd){
        sendMessage(fd, FLUSH);
        sendMessage(fd, "Playing round " + to_string(roundN) + "\n");

        sendMessage(fd, "\nYour Cards:\n");
        printPlayer(p_cards[fd], fd);
        
        sendMessage(fd, "\nCards on table:\n");
    }

    printTable(table_cards, list_fd);

    int max_bet = -1;
    unordered_map<int, int> bets;

    while (!bets.count(player_in_action) || bets[player_in_action] < max_bet)
    {
        if (max_bet > bets[player_in_action])
        {
            string clientMsg = "\nYour Money = " + to_string(player_amount[fd_user_map[player_in_action]]) + "\n";
            sendMessage(player_in_action, clientMsg);
            
            sendMessage(player_in_action, "\nCurrent bet: " + to_string(max_bet) + "\nDo you wanna raise [y/n]?\n");

            char reqbuf[MAXREQ];
            memset(reqbuf, 0, MAXREQ);
            read(player_in_action, reqbuf, MAXREQ-1);

            string msg = string(reqbuf);

            if (msg == "y")
            {
                player_amount[fd_user_map[player_in_action]] -= max_bet - bets[player_in_action];
                bets[player_in_action] = max_bet;
            }
            else
            {
                list_fd.erase(list_fd.begin());
            }
        }
        else
        {
            string clientMsg = "\nYour Money = " + to_string(player_amount[fd_user_map[player_in_action]]) + "\n";
            sendMessage(player_in_action, clientMsg);
            
            sendMessage(player_in_action, "\nPlace your bet: \n");
            
            char reqbuf[MAXREQ];
            memset(reqbuf, 0, MAXREQ);
            read(player_in_action, reqbuf, MAXREQ-1);

            string msg = string(reqbuf);
            if (msg != "")
                bets[player_in_action] = stoi(msg);
            else
                return 0;

            max_bet = bets[player_in_action];

            player_amount[fd_user_map[player_in_action]] -= bets[player_in_action];

            pot += bets[player_in_action];
        }

        for(int i = 1; i < list_fd.size(); i++){
            player_in_action = list_fd[i];
            
            if (max_bet == bets[player_in_action])
                continue;

            string clientMsg = "Your Money = " + to_string(player_amount[fd_user_map[player_in_action]]) + "\nPresent bet = " + to_string(max_bet) + "\nMake a choice:\nc:Call\n<number>:Raise\nf:Fold\n";
            sendMessage(player_in_action, clientMsg);
            
            char reqbuf[MAXREQ];
            memset(reqbuf, 0, MAXREQ);
            read(player_in_action, reqbuf, MAXREQ-1);

            string msg = string(reqbuf);
            
            if(msg[0] == 'c'){
                player_amount[fd_user_map[player_in_action]] -= max_bet;
                pot += max_bet;
            }
            else if (msg[0] == 'f') {
                list_fd.erase(list_fd.begin() + i);
                i--;
            }
            else{
                bets[player_in_action] = stoi(msg);
                max_bet = bets[player_in_action];
                cout << bets[player_in_action] << " " << max_bet << endl;
                pot += bets[player_in_action];
                player_amount[fd_user_map[player_in_action]] -= bets[player_in_action];
            }
        }
        player_in_action = list_fd[0];
    }

    if (list_fd.size() == 1)
    {
        player_amount[fd_user_map[list_fd[0]]] += pot;
        sendMessage(list_fd[0], "You have won " + to_string(pot) + "\nWait for 5 Seconds\n");
        sleep(5);
        return -1;
    }

    if (roundN == 3){
        int fd = judge(list_fd); // args : p1, p2, p3 => "1 or 2 or 3"; winner_amount += pot ; pot=0;
        player_amount[fd_user_map[fd]] += pot;
        sendMessage(fd, "You have won " + to_string(pot) + "\nWait for 5 Seconds\n");
        sleep(5);
    }

    return 1;
}