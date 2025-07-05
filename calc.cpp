#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

class CardGame {
private:
    static constexpr int HAND_SIZE = 7;
    static constexpr int DECK_SIZE = 52;
    
    std::vector<std::string> deck;
    std::vector<std::string> hand;
    std::unordered_set<std::string> valid_cards;
    std::mt19937 rng;
    
    void initialize_deck() {
        const std::vector<std::string> suits = {"H", "D", "C", "S"};
        const std::vector<std::string> values = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
        
        deck.clear();
        deck.reserve(DECK_SIZE);
        valid_cards.clear();
        
        for (const auto& suit : suits) {
            for (const auto& value : values) {
                std::string card = suit + value;
                deck.push_back(card);
                valid_cards.insert(card);
            }
        }
        
        std::shuffle(deck.begin(), deck.end(), rng);
    }
    
    void print_hand() const {
        std::cout << "Hand contains: ";
        for (const auto& card : hand) {
            std::cout << card << " ";
        }
        std::cout << std::endl;
    }
    
    std::vector<std::string> parse_discard_input(const std::string& input) const {
        std::vector<std::string> cards_to_discard;
        std::istringstream ss(input);
        std::string token;
        
        while (std::getline(ss, token, ',')) {
            // Trim whitespace
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            
            if (!token.empty()) {
                cards_to_discard.push_back(token);
            }
        }
        
        return cards_to_discard;
    }
    
    bool validate_discard_cards(const std::vector<std::string>& cards_to_discard) const {
        for (const auto& card : cards_to_discard) {
            if (valid_cards.find(card) == valid_cards.end()) {
                std::cout << "Invalid card: " << card << std::endl;
                return false;
            }
            
            if (std::find(hand.begin(), hand.end(), card) == hand.end()) {
                std::cout << "Card not in hand: " << card << std::endl;
                return false;
            }
        }
        return true;
    }
    
public:
    CardGame() : rng(std::random_device{}()) {
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
    
    bool discard_cards(const std::string& input) {
        auto cards_to_discard = parse_discard_input(input);
        
        if (cards_to_discard.empty()) {
            std::cout << "No cards specified for discard." << std::endl;
            return false;
        }
        
        if (!validate_discard_cards(cards_to_discard)) {
            return false;
        }
        
        // Remove cards from hand
        for (const auto& card : cards_to_discard) {
            auto it = std::find(hand.begin(), hand.end(), card);
            if (it != hand.end()) {
                hand.erase(it);
            }
        }
        
        std::cout << "Discarded " << cards_to_discard.size() << " card(s)." << std::endl;
        print_hand();
        return true;
    }
    
    void refill_hand() {
        while (hand.size() < HAND_SIZE && !deck.empty()) {
            hand.push_back(deck.back());
            deck.pop_back();
        }
        
        if (hand.size() < HAND_SIZE) {
            std::cout << "Warning: Not enough cards in deck to fill hand completely." << std::endl;
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
        std::cout << "Valid card format examples: H2, D10, CK, SA" << std::endl;
        std::cout << "Suits: H(Hearts), D(Diamonds), C(Clubs), S(Spades)" << std::endl;
        std::cout << "Values: 2-10, J, Q, K, A" << std::endl;
    }
};

int main() {
    CardGame game;
    
    std::cout << "=== Card Game ===" << std::endl;
    game.print_valid_cards();
    std::cout << std::endl;
    
    // Deal initial hand
    std::cout << "Dealing initial hand..." << std::endl;
    game.deal_initial_hand();
    
    while (true) {
        std::cout << "\nCards remaining in deck: " << game.cards_remaining() << std::endl;
        std::cout << "Enter cards to discard (comma-separated, or 'quit' to exit): ";
        
        std::string input;
        std::getline(std::cin, input);
        
        if (input == "quit" || input == "q") {
            break;
        }
        
        if (input.empty()) {
            std::cout << "No cards to discard." << std::endl;
            continue;
        }
        
        if (game.discard_cards(input)) {
            std::cout << "\nRefilling hand..." << std::endl;
            game.refill_hand();
        }
        
        if (game.cards_remaining() == 0) {
            std::cout << "\nDeck is empty! Game over." << std::endl;
            break;
        }
    }
    
    std::cout << "Thanks for playing!" << std::endl;
    return 0;
}