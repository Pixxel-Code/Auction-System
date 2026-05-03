#include <QApplication>
#include "MainWindow.h"
#include "DBManager.h"
#include "AuthService.h"

//Main starting
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AuctionHub");
    app.setApplicationVersion("1.0");

    // Loading persisted data
    DBManager& db = DBManager::getInstance();
    db.loadUsers();
    db.loadItems();
    db.loadBids();
    db.loadNotifications();

    // Create default admin and users if no users exist
    int count = 0;
    db.getAllUsers(count);
    if (count == 0) {
        AuthService::signup("admin", "admin123", "admin");
    }
//show window
    MainWindow window;
    window.show();

    return app.exec();
}
