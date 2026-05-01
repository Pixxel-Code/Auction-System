#include "Notification.h"

Notification::Notification() {
    userId = 0;
    message = "";
}

Notification::Notification(int uid, string msg) {
    userId = uid;
    message = msg;
}

string Notification::getMessage() {
    return message;
}

int Notification::getUserId() {
    return userId;
}