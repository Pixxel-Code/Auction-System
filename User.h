#pragma once
#include <string>
using namespace std;
//string getPassword();
class User {
private:
private:
    int id;
    string username;
    string password;
    string role;

    string watchlistData;

    double totalRating;
    int ratingCount;

public:
    User();
    User(int id, string u, string p, string r);

    int getId();
    string getUsername();
    string getRole();
    string getPassword();
    double getReputation();
    void addRating(double rating);
    double getTotalRating();
    int getRatingCount();

    string getWatchlistData();
    void setWatchlistData(string data);
    void addToWatchlist(int itemId);
    void removeFromWatchlist(int itemId);
    bool isWatching(int itemId);
    int* getWatchlist(int& count);
};
