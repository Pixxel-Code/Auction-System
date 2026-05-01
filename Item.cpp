#include "Item.h"

Item::Item() {
    id = 0;
    title = "";
    basePrice = 0;
    currentPrice = 0;
    sellerId = 0;
    duration = 0;
    startTime = 0;
    category = "";
    description = "";
    isActive = false;
}
Item::Item(int id, string title, double base, int seller, string category, int durationSeconds, string desc) {
    this->id = id;
    this->title = title;
    this->basePrice = base;
    this->currentPrice = base;
    this->sellerId = seller;
    this->category = category;
    this->description = desc;
    this->duration = durationSeconds;
    startTime = time(0);
    isActive = true;
}
void Item::updatePrice(double newPrice) {
    currentPrice = newPrice;
}
bool Item::isExpired() const {
    time_t now = time(0);
    return difftime(now, startTime) >= duration;
}
double Item::getCurrentPrice() { return currentPrice; }
int Item::getId() { return id; }
string Item::getTitle() { return title; }
double Item::getBasePrice() { return basePrice; }
int Item::getSellerId() { return sellerId; }
string Item::getCategory() { return category; }
void Item::setCategory(string cat) { category = cat; }
int    Item::getDuration() const { return duration; }
time_t Item::getStartTime() const { return startTime; }
void   Item::setStartTime(time_t t) { startTime = t; }
void   Item::setDuration(int s) { duration = s; }
int    Item::getSecondsRemaining() const {
    if (isExpired()) return 0;
    int elapsed = (int)difftime(time(0), startTime);
    return duration - elapsed;
}string Item::getDescription() { return description; }
void   Item::setDescription(string d) { description = d; }