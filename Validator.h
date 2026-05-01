#pragma once
#include <string>
using namespace std;

class Validator {
public:
    static bool isEmpty(const string& s) {
        return s.empty() || s.find_first_not_of(" \t\n") == string::npos;
    }

    static bool isValidPrice(double price) {
        return price > 0.0;
    }

    static bool isValidRole(const string& role) {
        return role == "buyer" || role == "seller" || role == "admin";
    }
};
