#include "DBManager.h"
#include <cstring>
#include <fstream>
#include <iostream>
DBManager& DBManager::getInstance() {
    static DBManager instance;
    return instance;
}

DBManager::DBManager() {
    users = nullptr;
    userCount = 0;

    items = nullptr;
    itemCount = 0;

    bids = nullptr;
    bidCount = 0;

    maxUserId = 0;
    maxItemId = 0;

    notifications = nullptr;
    notificationCount = 0;
}

DBManager::~DBManager() {
    delete[] users;
    delete[] items;
    delete[] bids;
    delete[] notifications;
}
void DBManager::addUser(User user) {

    User* temp = new User[userCount + 1];

    for (int i = 0; i < userCount; i++)
        temp[i] = users[i];

    temp[userCount] = user;

    delete[] users;
    users = temp;

    userCount++;

    saveUser(user);   // ✔ ONLY HERE
}
User DBManager::getUser(const char* username) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].getUsername().c_str(), username) == 0)
            return users[i];
    }
    return User();
}
void DBManager::addItem(Item item) {

    Item* temp = new Item[itemCount + 1];

    for (int i = 0; i < itemCount; i++)
        temp[i] = items[i];

    temp[itemCount] = item;

    delete[] items;
    items = temp;

    itemCount++;

    saveItem(item);   // ✔ ONLY HERE
}
Item* DBManager::getAllItems(int& count) {
    count = itemCount;
    return items;
}
void DBManager::addBid(Bid bid) {

    Bid* temp = new Bid[bidCount + 1];

    for (int i = 0; i < bidCount; i++)
        temp[i] = bids[i];

    temp[bidCount] = bid;

    delete[] bids;
    bids = temp;

    bidCount++;

    saveBid(bid);   // ✔ ONLY HERE
}
Bid* DBManager::getBidsForItem(int itemId, int& count) {

    int c = 0;
    for (int i = 0; i < bidCount; i++) {
        if (bids[i].getItemId() == itemId)
            c++;
    }

    Bid* result = new Bid[c];
    int index = 0;

    for (int i = 0; i < bidCount; i++) {
        if (bids[i].getItemId() == itemId)
            result[index++] = bids[i];
    }

    count = c;
    return result;
}
void DBManager::loadUsers() {
    delete[] users;
    users = nullptr;
    userCount = 0;

    std::ifstream file("users.txt");
    if (!file.is_open()) {
        std::cout << "users.txt not found\n";
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        string fields[7];
        int fi = 0;
        for (int i = 0; i < (int)line.size(); i++) {
            if (line[i] == '|' && fi < 6) fi++;
            else fields[fi] += line[i];
        }

        try {
            int    id = stoi(fields[0]);
            string username = fields[1];
            string password = fields[2];
            string role = fields[3];
            double totalRating = stod(fields[4]);
            int    ratingCount = stoi(fields[5]);

            if (id > maxUserId) maxUserId = id;

            User user(id, username, password, role);
            if (ratingCount > 0)
                user.addRating(totalRating);
            user.setWatchlistData(fields[6]);

            User* temp = new User[userCount + 1];
            for (int i = 0; i < userCount; i++)
                temp[i] = users[i];
            temp[userCount] = user;
            delete[] users;
            users = temp;
            userCount++;
        }
        catch (...) { continue; } // skip malformed lines
    }
    file.close();
}
void DBManager::loadItems() {
    delete[] items;
    items = nullptr;
    itemCount = 0;

    std::ifstream file("items.txt");
    if (!file.is_open()) {
        std::cout << "items.txt not found\n";
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        string fields[9];
        int fi = 0;
        for (int i = 0; i < (int)line.size(); i++) {
            if (line[i] == '|' && fi < 8) fi++;
            else fields[fi] += line[i];
        }

        try {
            int    id = stoi(fields[0]);
            string title = fields[1];
            double base = stod(fields[2]);
            double current = stod(fields[3]);
            int    sellerId = stoi(fields[4]);
            string category = fields[5];
            int    duration = stoi(fields[6]);
            time_t startTime = (time_t)stoll(fields[7]);
            string description = fields[8];

            if (id > maxItemId) maxItemId = id;

            Item item(id, title, base, sellerId, category, duration, description);
            item.updatePrice(current);
            item.setStartTime(startTime);

            Item* temp = new Item[itemCount + 1];
            for (int i = 0; i < itemCount; i++)
                temp[i] = items[i];
            temp[itemCount] = item;
            delete[] items;
            items = temp;
            itemCount++;
        }
        catch (...) { continue; } // skip malformed lines
    }

    file.close();
}
void DBManager::loadBids() {

    delete[] bids;
    bids = nullptr;
    bidCount = 0;
    std::ifstream file("bids.txt");

    if (!file.is_open()) {
        std::cout << "bids.txt not found\n";
        return;
    }

    int userId, itemId;
    double amount;

    while (file >> userId >> itemId >> amount) {

        Bid bid(userId, itemId, amount);

        Bid* temp = new Bid[bidCount + 1];

        for (int i = 0; i < bidCount; i++)
            temp[i] = bids[i];

        temp[bidCount] = bid;

        delete[] bids;
        bids = temp;

        bidCount++;
    }

    file.close();
}
void DBManager::saveUser(User user) {
    std::ofstream file("users.txt", std::ios::app);
    file << user.getId() << "|"
        << user.getUsername() << "|"
        << user.getPassword() << "|"
        << user.getRole() << "|"
        << user.getTotalRating() << "|"
        << user.getRatingCount() << "|"
        << user.getWatchlistData() << "\n";
    file.close();
}
void DBManager::saveItem(Item item) {
    std::ofstream file("items.txt", std::ios::app);
    file << item.getId() << "|"
        << item.getTitle() << "|"
        << item.getBasePrice() << "|"
        << item.getCurrentPrice() << "|"
        << item.getSellerId() << "|"
        << item.getCategory() << "|"
        << item.getDuration() << "|"
        << (long long)item.getStartTime() << "|"
        << item.getDescription() << "\n";
    file.close();
}
void DBManager::saveBid(Bid bid) {

    std::ofstream file("bids.txt", std::ios::app);

    file << bid.getUserId() << " "
        << bid.getItemId() << " "
        << bid.getAmount() << "\n";

    file.close();
}

int DBManager::generateUserId() {
    return ++maxUserId;
}

int DBManager::generateItemId() {
    return ++maxItemId;
}


Item* DBManager::searchByPrice(double min, double max, int& count) {

    int c = 0;

    for (int i = 0; i < itemCount; i++) {
        if (items[i].getCurrentPrice() >= min &&
            items[i].getCurrentPrice() <= max)
            c++;
    }

    Item* result = new Item[c];
    int index = 0;

    for (int i = 0; i < itemCount; i++) {
        if (items[i].getCurrentPrice() >= min &&
            items[i].getCurrentPrice() <= max)
            result[index++] = items[i];
    }

    count = c;
    return result;
}
User* DBManager::getAllUsers(int& count) {
    count = userCount;
    return users;
}
void DBManager::removeUserById(int id) {

    int newCount = 0;

    // STEP 1: calculate new size (excluding target user)
    for (int i = 0; i < userCount; i++) {
        if (users[i].getId() != id)
            newCount++;
    }

    // STEP 2: create new array
    User* temp = new User[newCount];
    int index = 0;

    // STEP 3: copy all except the deleted user
    for (int i = 0; i < userCount; i++) {
        if (users[i].getId() != id) {
            temp[index] = users[i];
            index++;
        }
    }

    // STEP 4: replace old memory
    delete[] users;
    users = temp;
    userCount = newCount;
}
void DBManager::removeItemById(int id) {

    int newCount = 0;

    for (int i = 0; i < itemCount; i++) {
        if (items[i].getId() != id)
            newCount++;
    }

    Item* temp = new Item[newCount];
    int index = 0;

    for (int i = 0; i < itemCount; i++) {
        if (items[i].getId() != id) {
            temp[index] = items[i];
            index++;
        }
    }

    delete[] items;
    items = temp;
    itemCount = newCount;
}
Bid* DBManager::getAllBids(int& count) {
    count = bidCount;
    return bids;
}
void DBManager::rewriteUsersFile() {
    std::ofstream file("users.txt");
    for (int i = 0; i < userCount; i++) {
        file << users[i].getId() << "|"
            << users[i].getUsername() << "|"
            << users[i].getPassword() << "|"
            << users[i].getRole() << "|"
            << users[i].getTotalRating() << "|"
            << users[i].getRatingCount() << "|"
            << users[i].getWatchlistData() << "\n";
    }
    file.close();
}
void DBManager::rewriteItemsFile() {
    std::ofstream file("items.txt");
    for (int i = 0; i < itemCount; i++) {
        file << items[i].getId() << "|"
            << items[i].getTitle() << "|"
            << items[i].getBasePrice() << "|"
            << items[i].getCurrentPrice() << "|"
            << items[i].getSellerId() << "|"
            << items[i].getCategory() << "|"
            << items[i].getDuration() << "|"
            << (long long)items[i].getStartTime() << "|"
            << items[i].getDescription() << "\n";
    }
    file.close();
}
void DBManager::rewriteBidsFile() {

    std::ofstream file("bids.txt");

    for (int i = 0; i < bidCount; i++) {
        file << bids[i].getUserId() << " "
            << bids[i].getItemId() << " "
            << bids[i].getAmount() << "\n";
    }

    file.close();
}
void DBManager::removeBidsByItemId(int itemId) {

    int newCount = 0;

    for (int i = 0; i < bidCount; i++) {
        if (bids[i].getItemId() != itemId)
            newCount++;
    }

    Bid* temp = new Bid[newCount];
    int index = 0;

    for (int i = 0; i < bidCount; i++) {
        if (bids[i].getItemId() != itemId)
            temp[index++] = bids[i];
    }

    delete[] bids;
    bids = temp;
    bidCount = newCount;
}
Item* DBManager::searchByTitle(string title, int& count) {
    int c = 0;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].getTitle().find(title) != string::npos)
            c++;
    }
    Item* result = new Item[c];
    int index = 0;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].getTitle().find(title) != string::npos)
            result[index++] = items[i];
    }
    count = c;
    return result;
}
Item* DBManager::searchByCategory(string category, int& count) {
    int c = 0;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].getCategory() == category)
            c++;
    }
    Item* result = new Item[c];
    int index = 0;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].getCategory() == category)
            result[index++] = items[i];
    }
    count = c;
    return result;
}
Item* DBManager::getActiveItems(int& count) {
    int c = 0;
    for (int i = 0; i < itemCount; i++) {
        if (!items[i].isExpired())
            c++;
    }
    Item* result = new Item[c];
    int index = 0;
    for (int i = 0; i < itemCount; i++) {
        if (!items[i].isExpired())
            result[index++] = items[i];
    }
    count = c;
    return result;
}
Item* DBManager::getExpiredItems(int& count) {
    int c = 0;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].isExpired())
            c++;
    }
    Item* result = new Item[c];
    int index = 0;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].isExpired())
            result[index++] = items[i];
    }
    count = c;
    return result;
}
void DBManager::addNotification(Notification n) {
    Notification* temp = new Notification[notificationCount + 1];
    for (int i = 0; i < notificationCount; i++)
        temp[i] = notifications[i];
    temp[notificationCount] = n;
    delete[] notifications;
    notifications = temp;
    notificationCount++;
    saveNotification(n);
}
Notification* DBManager::getNotificationsForUser(int userId, int& count) {
    int c = 0;
    for (int i = 0; i < notificationCount; i++) {
        if (notifications[i].getUserId() == userId)
            c++;
    }
    Notification* result = new Notification[c];
    int index = 0;
    for (int i = 0; i < notificationCount; i++) {
        if (notifications[i].getUserId() == userId)
            result[index++] = notifications[i];
    }
    count = c;
    return result;
}
void DBManager::saveNotification(Notification n) {
    std::ofstream file("notifications.txt", std::ios::app);
    file << n.getUserId() << "|" << n.getMessage() << "\n";
    file.close();
}

void DBManager::loadNotifications() {
    delete[] notifications;
    notifications = nullptr;
    notificationCount = 0;

    std::ifstream file("notifications.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        string fields[2];
        int fi = 0;
        for (int i = 0; i < (int)line.size(); i++) {
            if (line[i] == '|' && fi < 1) fi++;
            else fields[fi] += line[i];
        }

        try {
            int    userId = stoi(fields[0]);
            string msg = fields[1];

            Notification n(userId, msg);
            Notification* temp = new Notification[notificationCount + 1];
            for (int i = 0; i < notificationCount; i++)
                temp[i] = notifications[i];
            temp[notificationCount] = n;
            delete[] notifications;
            notifications = temp;
            notificationCount++;
        }
        catch (...) { continue; } // skip malformed lines
    }
    file.close();
}

void DBManager::rewriteNotificationsFile() {
    std::ofstream file("notifications.txt");
    for (int i = 0; i < notificationCount; i++)
        file << notifications[i].getUserId() << "|"
        << notifications[i].getMessage() << "\n";
    file.close();
}
void DBManager::updateUser(User updatedUser) {
    for (int i = 0; i < userCount; i++) {
        if (users[i].getId() == updatedUser.getId()) {
            users[i] = updatedUser;
            break;
        }
    }
    rewriteUsersFile();
}

void DBManager::updateItemPrice(int itemId, double newPrice) {
    for (int i = 0; i < itemCount; i++) {
        if (items[i].getId() == itemId) {
            items[i].updatePrice(newPrice);
            break;
        }
    }
    rewriteItemsFile();
}

void DBManager::expireItem(int itemId) {
    for (int i = 0; i < itemCount; i++) {
        if (items[i].getId() == itemId) {
            items[i].setStartTime(0);
            items[i].setDuration(1);
            break;
        }
    }
    rewriteItemsFile();
}