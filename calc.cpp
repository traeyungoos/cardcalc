#include <iostream>
#include <string>
#include <algorithm>  
#include <random>    
#include <sstream>

using std::cout; using std::cin; using std::endl; using std::string;

void populate_hand(string hand[], string deck[], int y_size){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, y_size - 1); 
    for(int i = 0; i < 7; ++i){
        if(hand[i] == "0"){
            int index;
            do {
                index = distr(gen);
            } while (deck[index] == "0");
            hand[i] = deck[index];
            deck[index] = "0"; 
        }
    }
    cout << "Hand contains:" << endl;
    for (int i = 0; i < 7; ++i) {
        cout << hand[i] << " ";
    }
    cout << endl;    
}

long long fact(long long x){
    if ( x == 0 || x ==1 ){
        return 1;
    }
    else {
        return x * fact(x-1);
    }
}

void discard(std::string hand[], std::string deck[], const std::string& sentence) {
    std::istringstream ss(sentence);
    string token;
    while (std::getline(ss, token, ',')) {
        for (int z = 0; z < 52; ++z) {
            if (deck[z] == token) {
                deck[z] = "0";
            }
        }
        for (int z = 0; z < 7; ++z) {
            if (hand[z] == token) {
                hand[z] = "0";
            }
        }
    }
    cout << "Hand contains:" << endl;
    for (int i = 0; i < 7; ++i) {
        cout << hand[i] << " ";
    }
    cout << endl;
}

int main() {
    
    string hand[7] = {"0","0", "0", "0", "0", "0", "0"};
    string deck[52];

    const char* suits[4] = { "H", "D", "C", "S" };
    const char* values[13] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A" };

    int index = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 13; ++j) {
            deck[index++] = string(suits[i]) + values[j];
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck, deck + 52, g);
    
    populate_hand(hand, deck, 52);

    string sent;
    cout << "enter cards to discard with commas: " << endl;
    std::getline(std::cin, sent);

    discard(hand, deck, sent);

    populate_hand(hand, deck, 52);


    return 0;
}
