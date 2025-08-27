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

class CardGame {
private:
    static constexpr int HAND_SIZE = 7;
    static constexpr int DECK_SIZE = 52;
    
    vector<string> deck;
    vector<string> hand;
    unordered_set<string> valid_cards;
    mt19937 rng;
    
    void initialize_deck() {
        const vector<string> suits = {"H", "D", "C", "S"};
        const vector<string> values = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
        
        deck.clear();
        deck.reserve(DECK_SIZE);
        valid_cards.clear();
        
        for (const auto& suit : suits) {
            for (const auto& value : values) {
                string card = suit + value;
                deck.push_back(card);
                valid_cards.insert(card);
            }
        }
        
        shuffle(deck.begin(), deck.end(), rng);
    }
    
    void print_hand() const {
        cout << "Hand contains: ";
        for (const auto& card : hand) {
            cout << card << " ";
        }
        cout << endl;
    }
    
    vector<string> parse_discard_input(const string& input) const {
        vector<string> cards_to_discard;
        istringstream ss(input);
        string token;
        
        while (getline(ss, token, ',')) {
            // Trim whitespace
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            
            if (!token.empty()) {
                cards_to_discard.push_back(token);
            }
        }
        
        return cards_to_discard;
    }
    
    bool validate_discard_cards(const vector<string>& cards_to_discard) const {
        for (const auto& card : cards_to_discard) {
            if (valid_cards.find(card) == valid_cards.end()) {
                cout << "Invalid card: " << card << endl;
                return false;
            }
            
            if (find(hand.begin(), hand.end(), card) == hand.end()) {
                cout << "Card not in hand: " << card << endl;
                return false;
            }
        }
        return true;
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
    
    bool discard_cards(const string& input) {
        auto cards_to_discard = parse_discard_input(input);
        
        if (cards_to_discard.empty()) {
            cout << "No cards specified for discard." << endl;
            return false;
        }
        
        if (!validate_discard_cards(cards_to_discard)) {
            return false;
        }
        
        // Remove cards from hand
        for (const auto& card : cards_to_discard) {
            auto it = find(hand.begin(), hand.end(), card);
            if (it != hand.end()) {
                hand.erase(it);
            }
        }
        
        cout << "Discarded " << cards_to_discard.size() << " card(s)." << endl;
        print_hand();
        return true;
    }
    
    void refill_hand() {
        while (hand.size() < HAND_SIZE && !deck.empty()) {
            hand.push_back(deck.back());
            deck.pop_back();
        }
        
        if (hand.size() < HAND_SIZE) {
            cout << "Warning: Not enough cards in deck to fill hand completely." << endl;
        }
        
        print_hand();
    }
    
    void reset_game() {
        initialize_deck();
        hand.clear();
    }
    
    size_t cards_remaining() const {
        return deck.size();
    }
    
    void print_valid_cards() const {
        cout << "Valid card format examples: H2, D10, CK, SA" << endl;
        cout << "Suits: H(Hearts), D(Diamonds), C(Clubs), S(Spades)" << endl;
        cout << "Values: 2-10, J, Q, K, A" << endl;
    }
};

int main() {
    CardGame game;
    
    cout << "=== Card Game ===" << endl;
    game.print_valid_cards();
    cout << endl;
    
    // Deal initial hand
    cout << "Dealing initial hand..." << endl;
    game.deal_initial_hand();
    
    while (true) {
        cout << "\nCards remaining in deck: " << game.cards_remaining() << endl;
        cout << "Enter cards to discard (comma-separated, or 'quit' to exit): ";
        
        string input;
        getline(cin, input);
        
        if (input == "quit" || input == "q") {
            break;
        }
        
        if (input.empty()) {
            cout << "No cards to discard." << endl;
            continue;
        }
        
        if (game.discard_cards(input)) {
            cout << "\nRefilling hand..." << endl;
            game.refill_hand();
        }
        
        if (game.cards_remaining() == 0) {
            cout << "\nDeck is empty! Game over." << endl;
            break;
        }
    }
    
    cout << "Thanks for playing!" << endl;
    return 0;
}