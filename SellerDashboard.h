#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTabWidget>
#include <QListWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include "User.h"
#include "AuctionManager.h"

class SellerDashboard : public QWidget {
    Q_OBJECT
public:
    explicit SellerDashboard(QWidget* parent = nullptr);
    void setUser(User u);
    void refresh();

signals:
    void logoutRequested();

private slots:
    void createItem();
    void closeSelectedAuction();
    void refreshItems();
    void viewNotifications();
    void submitRating();
    void onRateRequestSelected();

private:
    User currentUser;
    AuctionManager auctionMgr;

    QTabWidget* tabs;

    // My Listings tab
    QTableWidget* listingsTable;
    QPushButton* closeAuctionBtn;

    // Create Listing tab
    QLineEdit* itemTitle;
    QTextEdit* itemDescription;   // NEW
    QDoubleSpinBox* itemPrice;
    QComboBox* itemCategory;
    QSpinBox* itemDuration;
    QPushButton* createBtn;

    // Notifications tab
    QListWidget* notifList;

    // Rate tab
    QListWidget* rateRequestList;
    QDoubleSpinBox* sellerRatingBox;
    QPushButton* submitRatingBtn;
    int             pendingRateTargetId;

    // Header
    QLabel* welcomeLabel;
    QLabel* reputationLabel;

    void buildUI();
    QWidget* buildListingsTab();
    QWidget* buildCreateTab();
    QWidget* buildNotificationsTab();
    QWidget* buildRateTab();
    void loadRateRequests();
    void applyStyles();
    void setupAuctions();
};