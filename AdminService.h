#pragma once
#include "DBManager.h"
#include "User.h"

class AdminService {
public:

    // VIEW
    static User* getUsers(int& count);
    static Item* getItems(int& count);
    static Bid* getBids(int& count);

    // ADMIN ACTIONS (WITH ROLE CHECK)
    static bool deleteUser(int id, User currentUser);
    static bool deleteItem(int id, User currentUser);
    static bool rateUser(int raterId, int targetId, double rating);
    static double getReputation(int userId);
};