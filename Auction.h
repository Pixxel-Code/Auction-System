#pragma once
#include "Item.h"
#include "Bid.h"
#include "DBManager.h"
#include"notification.h"

class Auction {
private:
    Item item;
    bool isActive;

public:
    Auction(Item item);

    bool placeBid(Bid bid);
    Bid getHighestBid();

    void closeAuction();
    void checkAndClose();

    Item getItem();
    bool isActiveAuction();
};