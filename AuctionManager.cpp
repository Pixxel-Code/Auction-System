#include "AuctionManager.h"
#include <iostream>

AuctionManager::AuctionManager() {
    auctions = nullptr;
    auctionCount = 0;
}

AuctionManager::~AuctionManager() {
    delete[] auctions;
}
void AuctionManager::addAuction(Item item) {

    Auction** temp = new Auction * [auctionCount + 1];

    for (int i = 0; i < auctionCount; i++)
        temp[i] = auctions[i];

    temp[auctionCount] = new Auction(item);

    delete[] auctions;
    auctions = temp;

    auctionCount++;
}
Auction* AuctionManager::getAuctionByItemId(int itemId) {

    for (int i = 0; i < auctionCount; i++) {
        if (auctions[i]->getItem().getId() == itemId)
            return auctions[i];
    }

    return nullptr;
}
void AuctionManager::removeAuction(int itemId) {

    int newCount = 0;

    for (int i = 0; i < auctionCount; i++) {
        if (auctions[i]->getItem().getId() != itemId)
            newCount++;
    }

    Auction** temp = new Auction * [newCount];
    int index = 0;

    for (int i = 0; i < auctionCount; i++) {

        if (auctions[i]->getItem().getId() != itemId)
            temp[index++] = auctions[i];
        else
            delete auctions[i]; // important cleanup
    }

    delete[] auctions;
    auctions = temp;
    auctionCount = newCount;
}
void AuctionManager::checkAllAuctions() {

    for (int i = 0; i < auctionCount; i++) {
        auctions[i]->checkAndClose();
    }
}
void AuctionManager::showAllAuctions(int& count) {
    count = auctionCount;
}