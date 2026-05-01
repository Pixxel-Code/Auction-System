#pragma once
#include "User.h"
#include "Item.h"
#include "Bid.h"
#include"notification.h"

class DBManager {
private:
    User* users;
    int userCount;

    Item* items;
    int itemCount;

    Bid* bids;
    int bidCount;

    int maxUserId;
    int maxItemId;

    Notification* notifications;
    int notificationCount;

public:
    static DBManager& getInstance();

    DBManager();
    ~DBManager();

    // ADMIN SUPPORT (ADD THESE)
    User* getAllUsers(int& count);
    void removeUserById(int id);

    void removeItemById(int id);

    Bid* getAllBids(int& count);
    // USER
    void addUser(User user);
    User getUser(const char* username);

    // ITEM
    void addItem(Item item);
    Item* getAllItems(int& count);

    // BID
    void addBid(Bid bid);
    Bid* getBidsForItem(int itemId, int& count);

    // LOAD/SAVE
    void loadUsers();
    void loadItems();
    void loadBids();

    void saveUser(User user);
    void saveItem(Item item);
    void saveBid(Bid bid);

    // ✅ THESE MUST EXIST HERE
    int generateUserId();
    int generateItemId();
    Item* searchByPrice(double min, double max, int& count);

    // FILE REWRITE
    void rewriteUsersFile();
    void rewriteItemsFile();
    void rewriteBidsFile();

    Item* searchByTitle(string title, int& count);
    Item* searchByCategory(string category, int& count);
    Item* getActiveItems(int& count);
    Item* getExpiredItems(int& count);

    void addNotification(Notification n);
    Notification* getNotificationsForUser(int userId, int& count);

    // CASCADE DELETE
    void removeBidsByItemId(int itemId);

    void loadNotifications();
    void rewriteNotificationsFile();
    void saveNotification(Notification n);

    void updateUser(User updatedUser);
    void updateItemPrice(int itemId, double newPrice);
    void expireItem(int itemId);
};
