#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTabWidget>
#include <QListWidget>
#include <QDoubleSpinBox>
#include "User.h"
#include "AuctionManager.h"

class BuyerDashboard : public QWidget {
    Q_OBJECT
public:
    explicit BuyerDashboard(QWidget* parent = nullptr);
    void setUser(User u);
    void refresh();

signals:
    void logoutRequested();

private slots:
    void searchItems();
    void placeBid();
    void addToWatchlist();
    void removeFromWatchlist();
    void refreshWatchlist();
    void viewNotifications();
    void viewDescription();        // NEW
    void onTabChanged(int index);
    void submitRating();
    void onRateRequestSelected();

private:
    User currentUser;
    AuctionManager auctionMgr;

    QTabWidget* tabs;

    // Browse tab
    QTableWidget* itemsTable;
    QLineEdit* searchField;
    QComboBox* categoryFilter;
    QLineEdit* minPrice;
    QLineEdit* maxPrice;
    QPushButton* searchBtn;
    QPushButton* bidBtn;
    QPushButton* watchBtn;
    QPushButton* descBtn;         // NEW

    // Watchlist tab
    QTableWidget* watchTable;
    QPushButton* unwatchBtn;

    // Notifications tab
    QListWidget* notifList;

    // Rate tab
    QListWidget* rateRequestList;
    QDoubleSpinBox* buyerRatingBox;
    QPushButton* submitRatingBtn;
    int             pendingRateTargetId;

    // Header
    QLabel* welcomeLabel;
    QLabel* reputationLabel;

    void buildUI();
    QWidget* buildBrowseTab();
    QWidget* buildWatchlistTab();
    QWidget* buildNotificationsTab();
    QWidget* buildRateTab();
    void loadItems(Item* items, int count);
    void loadAllItems();
    void loadRateRequests();
    void applyStyles();
    void setupAuctions();
};