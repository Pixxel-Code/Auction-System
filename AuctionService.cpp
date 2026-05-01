#include "AuctionService.h"

bool AuctionService::placeBid(Auction& auction, int userId, double amount) {

    Bid bid(userId, auction.getItem().getId(), amount);

    return auction.placeBid(bid);
}