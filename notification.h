#pragma once
#include <string>
using namespace std;

class Notification {
private:
    int userId;
    string message;

public:
    Notification();
    Notification(int uid, string msg);
    string getMessage();
    int getUserId();
};
