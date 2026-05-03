#include "Notification.h"

Notification::Notification() {
    userId = 0;
    message = "";
}

Notification::Notification(int uid, string msg) {
    userId = uid;
    message = msg;
}
//message getter
string Notification::getMessage() {
    return message;
}
//id gtter
int Notification::getUserId() {
    return userId;
}
