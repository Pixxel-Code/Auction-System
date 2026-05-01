#include "Bid.h"

Bid::Bid() {
    userId = 0;
    itemId = 0;
    amount = 0;
}

Bid::Bid(int u, int i, double a) {
    userId = u;
    itemId = i;
    amount = a;
}

int Bid::getUserId() { return userId; }
int Bid::getItemId() { return itemId; }
double Bid::getAmount() { return amount; }