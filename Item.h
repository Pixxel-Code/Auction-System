#pragma once
#include <string>
#include <ctime>
using namespace std;

class Item {
private:
    int id;
    string title;
    double basePrice;
    double currentPrice;
    int sellerId;
    string category;

    int duration;
    time_t startTime;
    bool isActive;
    string description;

public:
    Item();
    Item(int id, string t, double b, int seller, string category, int durationSeconds = 3600, string description = "");

    void updatePrice(double newPrice);
    double getCurrentPrice();
    int getId();
    string getTitle();
    double getBasePrice();
    int getSellerId();
    string getCategory();
    void setCategory(string cat);
    string getDescription();
    void setDescription(string d);
    int    getDuration() const;
    time_t getStartTime() const;
    void   setStartTime(time_t t);
    void   setDuration(int secs);
    int    getSecondsRemaining() const;

    bool isExpired() const;
};