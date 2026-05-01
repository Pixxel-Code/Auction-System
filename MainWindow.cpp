#include "MainWindow.h"
#include "LoginWindow.h"
#include "BuyerDashboard.h"
#include "SellerDashboard.h"
#include "AdminDashboard.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("AuctionHub");
    setMinimumSize(1100, 700);
    resize(1280, 800);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    loginWindow  = new LoginWindow(this);
    buyerDash    = new BuyerDashboard(this);
    sellerDash   = new SellerDashboard(this);
    adminDash    = new AdminDashboard(this);

    stack->addWidget(loginWindow);   // index 0
    stack->addWidget(buyerDash);     // index 1
    stack->addWidget(sellerDash);    // index 2
    stack->addWidget(adminDash);     // index 3

    stack->setCurrentIndex(0);

    connect(loginWindow, &LoginWindow::loginSuccess,
            this,        &MainWindow::onLoginSuccess);

    connect(buyerDash,  &BuyerDashboard::logoutRequested,
            this,       &MainWindow::onLogout);
    connect(sellerDash, &SellerDashboard::logoutRequested,
            this,       &MainWindow::onLogout);
    connect(adminDash,  &AdminDashboard::logoutRequested,
            this,       &MainWindow::onLogout);
}

void MainWindow::onLoginSuccess(User user) {
    currentUser = user;
    QString role = QString::fromStdString(user.getRole());

    if (role == "admin") {
        adminDash->setUser(user);
        stack->setCurrentIndex(3);
    } else if (role == "seller") {
        sellerDash->setUser(user);
        stack->setCurrentIndex(2);
    } else {
        buyerDash->setUser(user);
        stack->setCurrentIndex(1);
    }
}

void MainWindow::onLogout() {
    currentUser = User();
    stack->setCurrentIndex(0);
}
