#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include "User.h"

class LoginWindow;
class BuyerDashboard;
class SellerDashboard;
class AdminDashboard;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

public slots:
    void onLoginSuccess(User user);
    void onLogout();

private:
    QStackedWidget* stack;
    LoginWindow* loginWindow;
    BuyerDashboard* buyerDash;
    SellerDashboard* sellerDash;
    AdminDashboard* adminDash;
    User currentUser;
};
