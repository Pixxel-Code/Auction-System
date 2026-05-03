#include "User.h"
//constructor
User::User() {
    id = 0;
    username = "";
    password = "";
    role = "";
    totalRating = 0;
    ratingCount = 0;
    watchlistData = "";
}
//parameterized
User::User(int id, string u, string p, string r) {
    this->id = id;
    username = u;
    password = p;
    role = r;

    totalRating = 0;
    ratingCount = 0;
    watchlistData = "";
}
//rating check
void User::addRating(double rating) {
    if (rating < 1.0) rating = 1.0;
    if (rating > 5.0) rating = 5.0;
    totalRating += rating;
    ratingCount++;
}
double User::getReputation() {
    if (ratingCount == 0)
        return 0.0;
    double avg = totalRating / ratingCount;
    if (avg > 5.0) avg = 5.0;
    if (avg < 1.0) avg = 1.0;
    return avg;
}

int User::getId() { return id; }
string User::getUsername() { return username; }
string User::getRole() { return role; }
string User::getPassword() { return password; }
double User::getTotalRating() { return totalRating; }
int User::getRatingCount() { return ratingCount; }

string User::getWatchlistData() { return watchlistData; }
void User::setWatchlistData(string data) { watchlistData = data; }
void User::addToWatchlist(int itemId) {
    if (isWatching(itemId)) return;
    if (!watchlistData.empty()) watchlistData += ",";
    watchlistData += to_string(itemId);
}

void User::removeFromWatchlist(int itemId) {
    string target = to_string(itemId);
    string result = "";
    string token = "";
    bool first = true;
    for (int i = 0; i <= (int)watchlistData.size(); i++) {
        if (i == (int)watchlistData.size() || watchlistData[i] == ',') {
            if (token != target) {
                if (!first) result += ",";
                result += token;
                first = false;
            }
            token = "";
        }
        else {
            token += watchlistData[i];
        }
    }
    watchlistData = result;
}

bool User::isWatching(int itemId) {
    string target = to_string(itemId);
    string token = "";
    for (int i = 0; i <= (int)watchlistData.size(); i++) {
        if (i == (int)watchlistData.size() || watchlistData[i] == ',') {
            if (token == target) return true;
            token = "";
        }
        else {
            token += watchlistData[i];
        }
    }
    return false;
}
int* User::getWatchlist(int& count) {
    count = 0;
    if (watchlistData.empty()) return nullptr;
    int c = 1;
    for (char ch : watchlistData) if (ch == ',') c++;
    int* ids = new int[c];
    string token = "";
    int idx = 0;
    for (int i = 0; i <= (int)watchlistData.size(); i++) {
        if (i == (int)watchlistData.size() || watchlistData[i] == ',') {
            if (!token.empty()) ids[idx++] = stoi(token);
            token = "";
        }
        else {
            token += watchlistData[i];
        }
    }
    count = idx;
    return ids;
}
