#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
using std::string;
using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::cout;
using std::cin;
using std::endl;
using std::mt19937;
using std::random_device;
using std::istringstream;
using std::getline;
using std::find;
using std::shuffle;
using std::ostream;

// Card struct definition
struct Card {
    string suit;
    string value;
    
    // Method to convert card to string for printing
    string to_string() const {
        return suit + value;
    }
    
    // Overload << operator for easy printing
    friend ostream& operator<<(ostream& os, const Card& card) {
        return os << card.suit << card.value;
    }
};

class CardGame {
private:
    static constexpr int HAND_SIZE = 7;
    static constexpr int DECK_SIZE = 52;
   
    vector<Card> deck;        // Changed from vector<string> to vector<Card>
    vector<Card> hand;        // Changed from vector<string> to vector<Card>
    mt19937 rng;
   
    void initialize_deck() {
        const vector<string> suits = {"H", "D", "C", "S"};
        const vector<string> values = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
       
        deck.clear();
        deck.reserve(DECK_SIZE);
       
        for (const auto& suit : suits) {
            for (const auto& value : values) {
                deck.push_back({suit, value});  // Create Card struct
            }
        }
       
        shuffle(deck.begin(), deck.end(), rng);
    }
   
    void print_hand() const {
        cout << "Hand contains: ";
        for (const auto& card : hand) {
            cout << card << " ";  // Works with operator<< overload
        }
        cout << endl;
    }
   
public:
    CardGame() : rng(random_device{}()) {
        hand.reserve(HAND_SIZE);
        initialize_deck();
    }
   
    void deal_initial_hand() {
        hand.clear();
       
        // Deal from the top of the shuffled deck
        for (int i = 0; i < HAND_SIZE && !deck.empty(); ++i) {
            hand.push_back(deck.back());
            deck.pop_back();
        }
       
        print_hand();
    }
    
    void print_valid_cards() const {
        cout << "Valid card format examples: H2, D10, CK, SA" << endl;
        cout << "Suits: H(Hearts), D(Diamonds), C(Clubs), S(Spades)" << endl;
        cout << "Values: 2-10, J, Q, K, A" << endl;
    }
    
    // Getter methods
    const vector<Card>& get_deck() const {
        return deck;
    }
    
    const vector<Card>& get_hand() const {
        return hand;
    }
    
    int get_remaining_cards() const {
        return deck.size();
    }
    
    void print_deck() const {
        cout << "Remaining cards in deck (" << deck.size() << "): ";
        for (const auto& card : deck) {
            cout << card << " ";
        }
        cout << endl;
    }
};

int main() {
    CardGame game;
    string input2;
   
    cout << "=== Card Game ===" << endl;
    game.print_valid_cards();
    cout << endl;
   
    // Deal initial hand
    cout << "Dealing initial hand..." << endl;
    game.deal_initial_hand();
    
    // Access the deck and hand
    cout << "\nCards remaining in deck: " << game.get_remaining_cards() << endl;
    
    // Access individual cards from hand
    const vector<Card>& hand_ref = game.get_hand();
    cout << "First card in hand: " << hand_ref[0] << endl;
    cout << "First card suit: " << hand_ref[0].suit << endl;
    cout << "First card value: " << hand_ref[0].value << endl;
    
    

    return 0;
}