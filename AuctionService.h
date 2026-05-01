#pragma once
#include "Auction.h"

class AuctionService {
public:
    static bool placeBid(Auction& auction, int userId, double amount);
};
