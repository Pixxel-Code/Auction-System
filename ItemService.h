#pragma once
#include <string>
#include"User.h"

class ItemService {
public:
    static void createItem(std::string title, double price, int sellerId, std::string category, User currentUser, int durationSeconds = 3600, std::string description = "");
};