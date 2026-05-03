#include "AuthService.h"
#include "DBManager.h"
#include"Validator.h"
#include<iostream>
#include <cstdlib>

bool AuthService::signup(string username, string password, string role)
{
//validations
    if (Validator::isEmpty(username)) {
        cout << "Username cannot be empty\n";
        return false;
    }

    if (Validator::isEmpty(password)) {
        cout << "Password cannot be empty\n";
        return false;
    }

    if (!Validator::isValidRole(role)) {
        cout << "Invalid role - defaulting to buyer\n";
        role = "buyer";
    }
//check admin 
    if (username == "admin") {
        role = "admin";
    }

    // Check BEFORE adding
    User existing = DBManager::getInstance().getUser(username.c_str());
    if (!existing.getUsername().empty()) {
        cout << "Username already taken\n";
        return false;
    }

    User user(DBManager::getInstance().generateUserId(), username, password, role);
    DBManager::getInstance().addUser(user);

    return true;
}

User AuthService::login(string username, string password) {
    User user = DBManager::getInstance().getUser(username.c_str());

    if (user.getUsername() == username && user.getPassword() == password) {
        return user;
    }

    return User();
}
