#include "ItemService.h"
#include "DBManager.h"
#include "Item.h"
#include"Validator.h"
#include<iostream>
#include <cstdlib>

void ItemService::createItem(string title, double price, int sellerId, string category, User currentUser, int durationSeconds, string description) {
    {
        if (currentUser.getRole() != "seller") {
            cout << "Only sellers can list items\n";
            return;
        }

        if (Validator::isEmpty(title)) {
            cout << "Title cannot be empty" << endl;
            return;
        }

        if (!Validator::isValidPrice(price)) {
            cout << "Price must be greater than 0" << endl;
            return;
        }
        int id = DBManager::getInstance().generateItemId();
        Item item(id, title, price, sellerId, category, durationSeconds, description);
        DBManager::getInstance().addItem(item);
    }
}