#include "AdminService.h"
#include"notification.h"
#include<iostream>

// VIEW
User* AdminService::getUsers(int& count) {
    return DBManager::getInstance().getAllUsers(count);
}

Item* AdminService::getItems(int& count) {
    return DBManager::getInstance().getAllItems(count);
}

Bid* AdminService::getBids(int& count) {
    return DBManager::getInstance().getAllBids(count);
}

// DELETE USER
bool AdminService::deleteUser(int id, User currentUser) {

    if (currentUser.getRole() != "admin") {
        cout << "Access denied\n";
        return false;
    }

    DBManager& db = DBManager::getInstance();

    db.removeUserById(id);
    db.rewriteUsersFile();

    cout << "User deleted successfully\n";

    return true;
}
// DELETE ITEM
bool AdminService::deleteItem(int id, User currentUser) {

    if (currentUser.getRole() != "admin") {
        cout << "Access denied\n";
        return false;
    }

    DBManager& db = DBManager::getInstance();

    db.removeItemById(id);
    db.removeBidsByItemId(id);

    db.rewriteItemsFile();
    db.rewriteBidsFile();
    cout << "Item deleted successfully\n";

    return true;
}
bool AdminService::rateUser(int raterId, int targetId, double rating) {
    if (raterId == targetId) {
        cout << "Cannot rate yourself\n";
        return false;
    }
    if (rating < 1.0 || rating > 5.0) {
        cout << "Rating must be between 1 and 5\n";
        return false;
    }

    DBManager& db = DBManager::getInstance();
    int count = 0;
    User* users = db.getAllUsers(count);

    // Check both rater and target exist
    bool raterFound = false, targetFound = false;
    for (int i = 0; i < count; i++) {
        if (users[i].getId() == raterId) raterFound = true;
        if (users[i].getId() == targetId) targetFound = true;
    }
    if (!raterFound || !targetFound) {
        cout << "User not found\n";
        return false;
    }

    for (int i = 0; i < count; i++) {
        if (users[i].getId() == targetId) {
            users[i].addRating(rating);
            db.updateUser(users[i]);

            // Format rating nicely e.g. "4.5"
            string ratingStr = to_string(rating);
            ratingStr = ratingStr.substr(0, ratingStr.find('.') + 2);

            db.addNotification(Notification(targetId,
                "You received a new rating: " + ratingStr + "/5"));
            cout << "Rating submitted successfully\n";
            return true;
        }
    }
    cout << "User not found\n";
    return false;
}

double AdminService::getReputation(int userId) {
    int count = 0;
    User* users = DBManager::getInstance().getAllUsers(count);
    for (int i = 0; i < count; i++)
        if (users[i].getId() == userId)
            return users[i].getReputation();
    return 0.0;
}