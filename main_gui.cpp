#include <QApplication>
#include "MainWindow.h"
#include "DBManager.h"
#include "AuthService.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AuctionHub");
    app.setApplicationVersion("1.0");

    // Load persisted data
    DBManager& db = DBManager::getInstance();
    db.loadUsers();
    db.loadItems();
    db.loadBids();
    db.loadNotifications();

    // Create default admin if no users exist
    int count = 0;
    db.getAllUsers(count);
    if (count == 0) {
        AuthService::signup("admin", "admin123", "admin");
    }

    MainWindow window;
    window.show();

    return app.exec();
}
