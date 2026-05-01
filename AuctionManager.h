#pragma once
#include "Auction.h"

class AuctionManager {

private:
    Auction** auctions;
    int auctionCount;

public:
    AuctionManager();
    ~AuctionManager();

    void addAuction(Item item);
    Auction* getAuctionByItemId(int itemId);

    void removeAuction(int itemId);

    void checkAllAuctions();

    void showAllAuctions(int& count);
};