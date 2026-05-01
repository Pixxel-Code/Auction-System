#pragma once

class Bid {
private:
    int userId;
    int itemId;
    double amount;

public:
    Bid();
    Bid(int u, int i, double a);

    int getUserId();
    int getItemId();
    double getAmount();
};
