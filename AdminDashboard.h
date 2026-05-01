#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QListWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include "User.h"

class AdminDashboard : public QWidget {
    Q_OBJECT
public:
    explicit AdminDashboard(QWidget* parent = nullptr);
    void setUser(User u);
    void refresh();

signals:
    void logoutRequested();

private slots:
    void deleteUser();
    void deleteItem();
    void rateUser();

private:
    User currentUser;

    QTabWidget* tabs;

    // Users tab
    QTableWidget* usersTable;
    QPushButton* deleteUserBtn;
    QLabel* userStatsLabel;

    // Items tab
    QTableWidget* itemsTable;
    QPushButton* deleteItemBtn;

    // Bids tab
    QTableWidget* bidsTable;

    // Rate tab
    QSpinBox* raterIdBox;
    QSpinBox* targetIdBox;
    QDoubleSpinBox* ratingBox;
    QPushButton* rateBtn;
    QLabel* rateResult;

    // Header
    QLabel* welcomeLabel;

    void buildUI();
    QWidget* buildUsersTab();
    QWidget* buildItemsTab();
    QWidget* buildBidsTab();
    QWidget* buildRateTab();
    void loadUsers();
    void loadItems();
    void loadBids();
    void applyStyles();
};
