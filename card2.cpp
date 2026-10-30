#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <random>
#include <string_view>
#include <charconv>
#include <cstdint>
#include <iomanip>
using std::string;
using std::string_view;
using std::vector;
using std::array;
using std::cout;
using std::cin;
using std::endl;
using std::mt19937;
using std::random_device;
using std::getline;
using std::find_if;
using std::shuffle;
using std::ostream;
using std::fixed;
using std::setprecision;

// Card uses uint8_t for suit/value indices instead of strings.
// Compact indices keep comparisons and copies cheap, and we only
// convert to a string when printing.
struct Card {
    uint8_t suit;   // 0=H, 1=D, 2=C, 3=S
    uint8_t value;  // 0=2 .. 12=A

    // static constexpr lookup tables - zero runtime cost
    static constexpr array<string_view, 4> SUIT_NAMES = {"H", "D", "C", "S"};
    static constexpr array<string_view, 13> VALUE_NAMES = {
        "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"
    };
    // precomputed rank table (avoids stoi/branch chain)
    static constexpr array<int, 13> RANKS = {
        2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
    };

    string to_string() const {
        string result;
        result.reserve(3);
        result += SUIT_NAMES[suit];
        result += VALUE_NAMES[value];
        return result;
    }

    friend ostream& operator<<(ostream& os, const Card& card) {
        return os << SUIT_NAMES[card.suit] << VALUE_NAMES[card.value];
    }
};

// Stores every hand-quality result in one place so each check can be
// answered without re-scanning the hand.
struct HandAnalysis {
    array<uint8_t, 13> value_counts{};   // count per value index
    array<uint8_t, 4>  suit_counts{};    // count per suit
    bool has_flush = false;
    bool has_straight = false;
    bool has_straight_flush = false;
    bool has_royal_flush = false;
    bool has_four_of_a_kind = false;
    bool has_three_of_a_kind = false;
    bool has_pair = false;
    bool has_two_pair = false;
    bool has_full_house = false;
    int max_value_count = 0;
    int pair_count = 0;
    int triple_count = 0;
};

class CardGame {
private:
    static constexpr int HAND_SIZE = 7;
    static constexpr int DECK_SIZE = 52;

    vector<Card> deck;
    vector<Card> hand;
    mt19937 rng;

    // cached analysis, invalidated on hand change
    mutable HandAnalysis cached_analysis;
    mutable bool analysis_dirty = true;

    void initialize_deck() {
        deck.clear();
        deck.reserve(DECK_SIZE);

        for (uint8_t suit = 0; suit < 4; ++suit) {
            for (uint8_t value = 0; value < 13; ++value) {
                deck.push_back({suit, value});
            }
        }

        shuffle(deck.begin(), deck.end(), rng);
    }

    void invalidate_analysis() { analysis_dirty = true; }

    void print_hand() const {
        cout << "Hand contains: ";
        for (const auto& card : hand) {
            cout << card << " ";
        }
        cout << endl;
    }

    // Parse a single card token directly into a Card.
    // Trims whitespace and avoids building intermediate strings.
    bool parse_card_token(string_view token, Card& out) const {
        // Trim
        while (!token.empty() && (token.front() == ' ' || token.front() == '\t'))
            token.remove_prefix(1);
        while (!token.empty() && (token.back() == ' ' || token.back() == '\t'))
            token.remove_suffix(1);

        if (token.size() < 2) return false;

        // Suit
        int suit = -1;
        switch (token[0]) {
            case 'H': suit = 0; break;
            case 'D': suit = 1; break;
            case 'C': suit = 2; break;
            case 'S': suit = 3; break;
            default: return false;
        }

        string_view val = token.substr(1);

        // Value lookup - direct comparison, no stoi
        int value = -1;
        if (val.size() == 1) {
            char c = val[0];
            if (c >= '2' && c <= '9') value = c - '2';
            else if (c == 'J') value = 9;
            else if (c == 'Q') value = 10;
            else if (c == 'K') value = 11;
            else if (c == 'A') value = 12;
        } else if (val == "10") {
            value = 8;
        }

        if (value < 0) return false;
        out = {static_cast<uint8_t>(suit), static_cast<uint8_t>(value)};
        return true;
    }

    // Analysis works on any list of cards (not just the hand),
    // so the probability checker can reuse it for every possible draw.
    static HandAnalysis analyze_cards(const vector<Card>& cards) {
        HandAnalysis a{};

        // bitmask for straight detection (no vector<bool> alloc)
        uint16_t rank_mask = 0;

        for (const auto& card : cards) {
            a.value_counts[card.value]++;
            a.suit_counts[card.suit]++;
            int rank = Card::RANKS[card.value];
            rank_mask |= (1u << rank);
            if (rank == 14) rank_mask |= (1u << 1); // Ace low
        }

        for (int v = 0; v < 13; ++v) {
            int c = a.value_counts[v];
            if (c > a.max_value_count) a.max_value_count = c;
            if (c >= 4) a.has_four_of_a_kind = true;
            if (c >= 3) { a.has_three_of_a_kind = true; a.triple_count++; }
            if (c >= 2) { a.has_pair = true; a.pair_count++; }
        }

        a.has_two_pair = a.pair_count >= 2;
        // pair_count also counts the triple itself, so a full house
        // needs at least one triple AND at least 2 values with 2+ cards
        a.has_full_house = a.triple_count >= 1 && a.pair_count >= 2;

        // Flush: any suit count >= 5
        for (int s = 0; s < 4; ++s) {
            if (a.suit_counts[s] >= 5) { a.has_flush = true; break; }
        }

        // straight detection via bitmask - scans the rank mask once
        auto has_run_of_5 = [](uint16_t mask) {
            int run = 0;
            for (int r = 1; r <= 14; ++r) {
                if (mask & (1u << r)) { if (++run >= 5) return true; }
                else run = 0;
            }
            return false;
        };

        a.has_straight = has_run_of_5(rank_mask);

        // Straight flush / royal flush: only need to check suits with >=5 cards
        if (a.has_flush) {
            for (int s = 0; s < 4; ++s) {
                if (a.suit_counts[s] < 5) continue;
                uint16_t suit_mask = 0;
                for (const auto& card : cards) {
                    if (card.suit == s) {
                        int rank = Card::RANKS[card.value];
                        suit_mask |= (1u << rank);
                        if (rank == 14) suit_mask |= (1u << 1);
                    }
                }
                if (has_run_of_5(suit_mask)) {
                    a.has_straight_flush = true;
                    // Royal flush: 10,J,Q,K,A = ranks 10,11,12,13,14
                    uint16_t royal = (1u<<10)|(1u<<11)|(1u<<12)|(1u<<13)|(1u<<14);
                    if ((suit_mask & royal) == royal) {
                        a.has_royal_flush = true;
                    }
                    break; // found one straight flush, enough
                }
            }
        }

        return a;
    }

    // Walk the hand once and fill in every quality flag and count.
    const HandAnalysis& analyze_hand() const {
        if (!analysis_dirty) return cached_analysis;
        cached_analysis = analyze_cards(hand);
        analysis_dirty = false;
        return cached_analysis;
    }

    // Locate a card in the hand by identity, without any string work.
    auto find_in_hand(const Card& target) const {
        return find_if(hand.begin(), hand.end(), [&](const Card& c) {
            return c.suit == target.suit && c.value == target.value;
        });
    }

    // Parse + validate a comma-separated card list (shared by
    // discard_cards and the probability checker).
    bool parse_card_list(const string& input, vector<Card>& cards) const {
        cards.reserve(7);
        size_t start = 0;
        bool any_invalid = false;

        auto process_token = [&](string_view token) {
            Card parsed{};
            if (!parse_card_token(token, parsed)) {
                cout << "Invalid card: " << token << endl;
                any_invalid = true;
                return;
            }
            if (find_in_hand(parsed) == hand.end()) {
                cout << "Card not in hand: " << token << endl;
                any_invalid = true;
                return;
            }
            // stop the same card being entered twice (e.g. "H2, H2")
            for (const auto& c : cards) {
                if (c.suit == parsed.suit && c.value == parsed.value) {
                    cout << "Card entered twice: " << token << endl;
                    any_invalid = true;
                    return;
                }
            }
            cards.push_back(parsed);
        };

        for (size_t i = 0; i <= input.size(); ++i) {
            if (i == input.size() || input[i] == ',') {
                if (i > start) {
                    process_token(string_view(input).substr(start, i - start));
                }
                start = i + 1;
            }
        }

        if (any_invalid) return false;
        if (cards.empty()) {
            cout << "No cards specified." << endl;
            return false;
        }
        return true;
    }

    // Tries every possible set of cards you could draw from the deck
    // and counts which hands each resulting hand contains.
    // counts: 0=Royal Flush ... 8=Pair, 9=High Card
    void count_draws(vector<Card>& test_hand, size_t slot, size_t start,
                     array<uint64_t, 10>& counts, uint64_t& total) const {
        if (slot == test_hand.size()) {
            HandAnalysis a = analyze_cards(test_hand);
            if (a.has_royal_flush)     counts[0]++;
            if (a.has_straight_flush)  counts[1]++;
            if (a.has_four_of_a_kind)  counts[2]++;
            if (a.has_full_house)      counts[3]++;
            if (a.has_flush)           counts[4]++;
            if (a.has_straight)        counts[5]++;
            if (a.has_three_of_a_kind) counts[6]++;
            if (a.has_two_pair)        counts[7]++;
            if (a.has_pair)            counts[8]++;
            counts[9]++;
            total++;
            return;
        }
        size_t cards_needed = test_hand.size() - slot;
        for (size_t i = start; i + cards_needed <= deck.size(); ++i) {
            test_hand[slot] = deck[i];
            count_draws(test_hand, slot + 1, i + 1, counts, total);
        }
    }

public:
    CardGame() : rng(random_device{}()) {
        hand.reserve(HAND_SIZE);
        initialize_deck();
    }

    void deal_initial_hand() {
        hand.clear();
        for (int i = 0; i < HAND_SIZE && !deck.empty(); ++i) {
            hand.push_back(deck.back());
            deck.pop_back();
        }
        invalidate_analysis();
        print_hand();
    }

    bool discard_cards(const string& input) {
        if (input.empty()) {
            cout << "No cards specified for discard." << endl;
            return false;
        }

        vector<Card> cards_to_discard;
        if (!parse_card_list(input, cards_to_discard)) return false;

        // Remove from hand
        for (const auto& card : cards_to_discard) {
            auto it = find_in_hand(card);
            if (it != hand.end()) hand.erase(it);
        }

        invalidate_analysis();
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
        invalidate_analysis();
        print_hand();
    }

    // Shows the chance of every hand if you discard these 1-5 cards
    // and draw replacements. Nothing is actually discarded.
    void show_discard_odds(const string& input) const {
        vector<Card> to_discard;
        if (!parse_card_list(input, to_discard)) return;

        if (to_discard.size() > 5) {
            cout << "You can only check discards of 1-5 cards." << endl;
            return;
        }

        // cards you keep
        vector<Card> test_hand;
        for (const auto& card : hand) {
            bool discarding = false;
            for (const auto& d : to_discard) {
                if (d.suit == card.suit && d.value == card.value) discarding = true;
            }
            if (!discarding) test_hand.push_back(card);
        }

        size_t kept = test_hand.size();
        size_t draw = HAND_SIZE - kept;
        if (draw > deck.size()) draw = deck.size();
        test_hand.resize(kept + draw);

        array<uint64_t, 10> counts{};
        uint64_t total = 0;
        count_draws(test_hand, kept, 0, counts, total);

        const array<string_view, 10> names = {
            "Royal Flush", "Straight Flush", "Four of a Kind", "Full House",
            "Flush", "Straight", "Three of a Kind", "Two Pair", "Pair", "High Card"
        };

        cout << "\nIf you discard " << to_discard.size() << " card(s) and draw "
             << draw << " from " << deck.size() << " remaining:" << endl;
        for (int i = 0; i < 10; ++i) {
            double percent = total ? 100.0 * counts[i] / total : 0.0;
            cout << "  " << std::left << std::setw(16) << names[i]
                 << std::right << fixed << setprecision(2) << std::setw(7)
                 << percent << "%" << endl;
        }
        cout << "(" << total << " possible draws checked)" << endl;
    }

    void print_valid_cards() const {
        cout << "Valid card format examples: H2, D10, CK, SA" << endl;
        cout << "Suits: H(Hearts), D(Diamonds), C(Clubs), S(Spades)" << endl;
        cout << "Values: 2-10, J, Q, K, A" << endl;
        cout << "Type ? before cards to see odds without discarding, e.g. ? H2, D5" << endl;
    }

    const vector<Card>& get_deck() const { return deck; }
    const vector<Card>& get_hand() const { return hand; }
    int get_remaining_cards() const { return static_cast<int>(deck.size()); }

    void print_deck() const {
        cout << "Remaining cards in deck (" << deck.size() << "): ";
        for (const auto& card : deck) cout << card << " ";
        cout << endl;
    }

    // Each check reads the cached analysis, so repeated calls are cheap.
    bool has_four_of_a_kind()  const { return analyze_hand().has_four_of_a_kind; }
    bool has_three_of_a_kind() const { return analyze_hand().has_three_of_a_kind; }
    bool has_pair()            const { return analyze_hand().has_pair; }
    bool has_two_pair()        const { return analyze_hand().has_two_pair; }
    bool has_full_house()      const { return analyze_hand().has_full_house; }
    bool has_flush()           const { return analyze_hand().has_flush; }
    bool has_straight()        const { return analyze_hand().has_straight; }
    bool has_straight_flush()  const { return analyze_hand().has_straight_flush; }
    bool has_royal_flush()     const { return analyze_hand().has_royal_flush; }

    // One analysis call drives every line of the output.
    void print_hand_combos() const {
        const auto& a = analyze_hand();

        cout << "\nPossible hands:" << endl;
        if (a.has_royal_flush)      cout << "  Royal Flush" << endl;
        if (a.has_straight_flush)   cout << "  Straight Flush" << endl;
        if (a.has_four_of_a_kind)   cout << "  Four of a Kind" << endl;
        if (a.has_full_house)       cout << "  Full House" << endl;
        if (a.has_flush)            cout << "  Flush" << endl;
        if (a.has_straight)         cout << "  Straight" << endl;
        if (a.has_three_of_a_kind)  cout << "  Three of a Kind" << endl;
        if (a.has_two_pair)         cout << "  Two Pair" << endl;
        if (a.has_pair)             cout << "  Pair" << endl;
        cout << "  High Card" << endl;

        cout << "\nCard Value Breakdown:" << endl;
        for (int v = 0; v < 13; ++v) {
            int count = a.value_counts[v];
            if (count > 0) {
                cout << "  " << Card::VALUE_NAMES[v] << ": " << count;
                cout << (count > 1 ? " cards" : " card") << endl;
            }
        }
    }
};

int main() {
    CardGame game;

    cout << "=== Card Game ===" << endl;
    game.print_valid_cards();
    cout << endl;

    cout << "Dealing initial hand..." << endl;
    game.deal_initial_hand();
    game.print_hand_combos();

    while (true) {
        cout << "\nCards remaining in deck: " << game.get_remaining_cards() << endl;
        cout << "Enter cards to discard, ? cards to check odds, or 'quit': ";

        string input;
        getline(cin, input);

        if (input == "quit" || input == "q") break;
        if (input.empty()) { cout << "No cards to discard." << endl; continue; }

        // "? H2, D5" shows probabilities without discarding
        if (input[0] == '?') {
            game.show_discard_odds(input.substr(1));
            continue;
        }

        if (game.discard_cards(input)) {
            cout << "\nRefilling hand..." << endl;
            game.refill_hand();
            game.print_hand_combos();
        }

        if (game.get_remaining_cards() == 0) {
            cout << "\nDeck is empty! Game over." << endl;
            break;
        }
    }

    cout << "Thanks for playing!" << endl;
    return 0;
}