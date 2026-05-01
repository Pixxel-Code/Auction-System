#include "SoundManager.h"

#include "BuyerDashboard.h"
#include "DBManager.h"
#include "AuctionService.h"
#include "AdminService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QFrame>
#include <QScrollArea>
#include <QSplitter>
#include <QDialog>
#include <QTextEdit>


#include <QPainter>

static QPixmap makeLogoPixmap(int size) {
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    double cx = size / 2.0, cy = size / 2.0, r = size / 2.0 - 1;

    // Outer circle
    p.setPen(QPen(QColor("#C8922A"), size * 0.025));
    p.setBrush(QColor("#0D1018"));
    p.drawEllipse(QPointF(cx, cy), r, r);

    // Shield path
    double sw = size * 0.54, sh = size * 0.72;
    double sx = cx - sw / 2, sy = cy - sh / 2 - size * 0.02;
    QPolygonF shield;
    shield << QPointF(cx, sy)
        << QPointF(sx + sw, sy + sh * 0.17)
        << QPointF(sx + sw, sy + sh * 0.52)
        << QPointF(cx, sy + sh)
        << QPointF(sx, sy + sh * 0.52)
        << QPointF(sx, sy + sh * 0.17);
    p.setPen(QPen(QColor("#C8922A"), size * 0.02));
    p.setBrush(QColor("#151E30"));
    p.drawPolygon(shield);

    // A monogram
    p.setPen(Qt::NoPen);
    QFont font("Trebuchet MS", int(size * 0.36), QFont::Bold);
    p.setFont(font);
    p.setPen(QColor("#C8922A"));
    p.drawText(QRectF(cx - size * 0.28, cy - size * 0.22, size * 0.56, size * 0.5),
        Qt::AlignCenter, "A");

    // Crown dots
    double dotR = size * 0.04;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#F0C060"));
    p.drawEllipse(QPointF(cx - size * 0.20, sy + size * 0.02), dotR, dotR);
    p.drawEllipse(QPointF(cx, sy - size * 0.01), dotR, dotR);
    p.drawEllipse(QPointF(cx + size * 0.20, sy + size * 0.02), dotR, dotR);

    p.end();
    return pix;
}

static QLabel* makeLogo(int size) {
    QLabel* logo = new QLabel;
    logo->setAlignment(Qt::AlignCenter);
    logo->setFixedSize(size, size);
    logo->setPixmap(makeLogoPixmap(size));
    return logo;
}

static QLabel* makeDashLogo(int size) {
    QLabel* logo = new QLabel;
    logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    logo->setFixedSize(size, size);
    logo->setPixmap(makeLogoPixmap(size));
    return logo;
}

BuyerDashboard::BuyerDashboard(QWidget* parent) : QWidget(parent) {
    pendingRateTargetId = -1;
    buildUI();
    applyStyles();
}

void BuyerDashboard::setUser(User u) {
    currentUser = u;
    welcomeLabel->setText(QString("Welcome back, <b>%1</b>")
        .arg(QString::fromStdString(u.getUsername())));
    double rep = u.getReputation();
    reputationLabel->setText(QString("⭐ Rating: %1")
        .arg(rep > 0 ? QString::number(rep, 'f', 1) : "No ratings yet"));
    setupAuctions();
    refresh();
}

void BuyerDashboard::setupAuctions() {
    int count = 0;
    Item* items = DBManager::getInstance().getAllItems(count);
    for (int i = 0; i < count; i++)
        auctionMgr.addAuction(items[i]);
}

void BuyerDashboard::buildUI() {
    QVBoxLayout* main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);
    main->setSpacing(0);

    // Header bar
    QFrame* header = new QFrame;
    header->setObjectName("header");
    header->setFixedHeight(64);
    QHBoxLayout* hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(24, 0, 24, 0);

    QLabel* logo = makeDashLogo(40);
    logo->setObjectName("logoIcon");

    welcomeLabel = new QLabel("Welcome");
    welcomeLabel->setObjectName("welcome");

    reputationLabel = new QLabel("");
    reputationLabel->setObjectName("repLabel");

    QPushButton* logoutBtn = new QPushButton("Logout");
    logoutBtn->setObjectName("logoutBtn");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    connect(logoutBtn, &QPushButton::clicked, this, &BuyerDashboard::logoutRequested);

    hlay->addWidget(logo);
    hlay->addStretch();
    hlay->addWidget(welcomeLabel);
    hlay->addSpacing(16);
    hlay->addWidget(reputationLabel);
    hlay->addSpacing(16);
    hlay->addWidget(logoutBtn);

    tabs = new QTabWidget;
    tabs->setObjectName("mainTabs");
    tabs->addTab(buildBrowseTab(), "🔍  Browse Auctions");
    tabs->addTab(buildWatchlistTab(), "👁  Watchlist");
    tabs->addTab(buildNotificationsTab(), "🔔  Notifications");
    tabs->addTab(buildRateTab(), "⭐  Rate Users");   // NEW TAB

    connect(tabs, &QTabWidget::currentChanged, this, &BuyerDashboard::onTabChanged);

    main->addWidget(header);
    main->addWidget(tabs);
}

QWidget* BuyerDashboard::buildBrowseTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    // Search bar
    QFrame* searchBar = new QFrame;
    searchBar->setObjectName("searchBar");
    QHBoxLayout* slay = new QHBoxLayout(searchBar);
    slay->setContentsMargins(16, 12, 16, 12);
    slay->setSpacing(12);

    searchField = new QLineEdit;
    searchField->setPlaceholderText("Search auctions...");
    searchField->setObjectName("searchField");

    categoryFilter = new QComboBox;
    categoryFilter->addItem("All Categories");
    categoryFilter->addItem("Electronics");
    categoryFilter->addItem("Phones");
    categoryFilter->addItem("Clothing");
    categoryFilter->addItem("Vehicles");
    categoryFilter->addItem("Furniture");
    categoryFilter->addItem("Other");
    categoryFilter->setObjectName("filterCombo");

    minPrice = new QLineEdit;
    minPrice->setPlaceholderText("Min $");
    minPrice->setFixedWidth(80);
    minPrice->setObjectName("priceField");

    maxPrice = new QLineEdit;
    maxPrice->setPlaceholderText("Max $");
    maxPrice->setFixedWidth(80);
    maxPrice->setObjectName("priceField");

    searchBtn = new QPushButton("Search");
    searchBtn->setObjectName("searchBtn");
    searchBtn->setCursor(Qt::PointingHandCursor);

    QPushButton* clearBtn = new QPushButton("Clear");
    clearBtn->setObjectName("clearBtn");
    clearBtn->setCursor(Qt::PointingHandCursor);

    slay->addWidget(searchField, 2);
    slay->addWidget(categoryFilter, 1);
    slay->addWidget(new QLabel("$"));
    slay->addWidget(minPrice);
    slay->addWidget(new QLabel("–"));
    slay->addWidget(maxPrice);
    slay->addWidget(searchBtn);
    slay->addWidget(clearBtn);

    connect(searchBtn, &QPushButton::clicked, this, &BuyerDashboard::searchItems);
    connect(clearBtn, &QPushButton::clicked, this, [this]() {
        searchField->clear(); minPrice->clear(); maxPrice->clear();
        categoryFilter->setCurrentIndex(0);
        loadAllItems();
        });
    connect(searchField, &QLineEdit::returnPressed, this, &BuyerDashboard::searchItems);

    // Table
    itemsTable = new QTableWidget;
    itemsTable->setObjectName("dataTable");
    itemsTable->setColumnCount(6);
    itemsTable->setHorizontalHeaderLabels({ "ID","Title","Category","Current Price","Time Left","Status" });
    itemsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    itemsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    itemsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    itemsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    itemsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    itemsTable->verticalHeader()->hide();
    itemsTable->setAlternatingRowColors(true);

    // Action buttons
    QHBoxLayout* actRow = new QHBoxLayout;

    descBtn = new QPushButton("📄  View Description");
    descBtn->setObjectName("clearBtn");
    descBtn->setCursor(Qt::PointingHandCursor);

    bidBtn = new QPushButton("💰  Place Bid");
    bidBtn->setObjectName("actionBtn");
    bidBtn->setCursor(Qt::PointingHandCursor);

    watchBtn = new QPushButton("👁  Add to Watchlist");
    watchBtn->setObjectName("watchBtn");
    watchBtn->setCursor(Qt::PointingHandCursor);

    actRow->addStretch();
    actRow->addWidget(descBtn);
    actRow->addWidget(bidBtn);
    actRow->addWidget(watchBtn);

    connect(descBtn, &QPushButton::clicked, this, &BuyerDashboard::viewDescription);
    connect(bidBtn, &QPushButton::clicked, this, &BuyerDashboard::placeBid);
    connect(watchBtn, &QPushButton::clicked, this, &BuyerDashboard::addToWatchlist);

    lay->addWidget(searchBar);
    lay->addWidget(itemsTable);
    lay->addLayout(actRow);

    return w;
}

QWidget* BuyerDashboard::buildWatchlistTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    QLabel* hdr = new QLabel("Your Watchlist");
    hdr->setObjectName("sectionHeader");

    watchTable = new QTableWidget;
    watchTable->setObjectName("dataTable");
    watchTable->setColumnCount(5);
    watchTable->setHorizontalHeaderLabels({ "ID","Title","Category","Current Price","Time Left" });
    watchTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    watchTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    watchTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    watchTable->verticalHeader()->hide();
    watchTable->setAlternatingRowColors(true);

    QHBoxLayout* actRow = new QHBoxLayout;
    unwatchBtn = new QPushButton("🗑  Remove from Watchlist");
    unwatchBtn->setObjectName("dangerBtn");
    unwatchBtn->setCursor(Qt::PointingHandCursor);
    actRow->addStretch();
    actRow->addWidget(unwatchBtn);

    connect(unwatchBtn, &QPushButton::clicked, this, &BuyerDashboard::removeFromWatchlist);

    lay->addWidget(hdr);
    lay->addWidget(watchTable);
    lay->addLayout(actRow);

    return w;
}

QWidget* BuyerDashboard::buildNotificationsTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    QLabel* hdr = new QLabel("Notifications");
    hdr->setObjectName("sectionHeader");

    notifList = new QListWidget;
    notifList->setObjectName("notifList");

    QPushButton* refreshBtn = new QPushButton("🔄  Refresh");
    refreshBtn->setObjectName("clearBtn");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    connect(refreshBtn, &QPushButton::clicked, this, &BuyerDashboard::viewNotifications);

    lay->addWidget(hdr);
    lay->addWidget(notifList);
    lay->addWidget(refreshBtn, 0, Qt::AlignRight);

    return w;
}

QWidget* BuyerDashboard::buildRateTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    QLabel* hdr = new QLabel("Rate Users");
    hdr->setObjectName("sectionHeader");

    QLabel* hint = new QLabel("Select a completed auction below, then submit your rating for the seller:");
    hint->setObjectName("hintLabel");

    rateRequestList = new QListWidget;
    rateRequestList->setObjectName("notifList");

    QHBoxLayout* rateRow = new QHBoxLayout;
    QLabel* starLbl = new QLabel("Your Rating (1–5):");
    starLbl->setObjectName("formLabel");

    buyerRatingBox = new QDoubleSpinBox;
    buyerRatingBox->setRange(1.0, 5.0);
    buyerRatingBox->setSingleStep(0.5);
    buyerRatingBox->setValue(5.0);
    buyerRatingBox->setObjectName("formField");
    buyerRatingBox->setFixedWidth(90);

    submitRatingBtn = new QPushButton("⭐  Submit Rating");
    submitRatingBtn->setObjectName("primaryBtn");
    submitRatingBtn->setCursor(Qt::PointingHandCursor);

    rateRow->addWidget(starLbl);
    rateRow->addWidget(buyerRatingBox);
    rateRow->addSpacing(12);
    rateRow->addWidget(submitRatingBtn);
    rateRow->addStretch();

    connect(rateRequestList, &QListWidget::currentRowChanged,
        this, &BuyerDashboard::onRateRequestSelected);
    connect(submitRatingBtn, &QPushButton::clicked,
        this, &BuyerDashboard::submitRating);

    lay->addWidget(hdr);
    lay->addWidget(hint);
    lay->addWidget(rateRequestList);
    lay->addLayout(rateRow);

    return w;
}

void BuyerDashboard::refresh() {
    loadAllItems();
    refreshWatchlist();
    viewNotifications();
}

void BuyerDashboard::loadAllItems() {
    int count = 0;
    Item* items = DBManager::getInstance().getActiveItems(count);
    loadItems(items, count);
    delete[] items;
}

void BuyerDashboard::loadItems(Item* items, int count) {
    itemsTable->setRowCount(0);
    for (int i = 0; i < count; i++) {
        int row = itemsTable->rowCount();
        itemsTable->insertRow(row);
        itemsTable->setItem(row, 0, new QTableWidgetItem(QString::number(items[i].getId())));
        itemsTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(items[i].getTitle())));
        itemsTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(items[i].getCategory())));
        itemsTable->setItem(row, 3, new QTableWidgetItem(
            QString("$%1").arg(items[i].getCurrentPrice(), 0, 'f', 2)));

        int secs = items[i].getSecondsRemaining();
        QString timeStr;
        if (secs <= 0) timeStr = "Expired";
        else if (secs < 60) timeStr = QString("%1s").arg(secs);
        else if (secs < 3600) timeStr = QString("%1m %2s").arg(secs / 60).arg(secs % 60);
        else timeStr = QString("%1h %2m").arg(secs / 3600).arg((secs % 3600) / 60);

        itemsTable->setItem(row, 4, new QTableWidgetItem(timeStr));

        QTableWidgetItem* status = new QTableWidgetItem(items[i].isExpired() ? "Closed" : "Active");
        status->setForeground(items[i].isExpired() ? QColor("#f87171") : QColor("#34d399"));
        itemsTable->setItem(row, 5, status);
    }
}

void BuyerDashboard::viewDescription() {
    int row = itemsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select an item to view its description.");
        return;
    }

    int itemId = itemsTable->item(row, 0)->text().toInt();
    QString title = itemsTable->item(row, 1)->text();

    // Find description from DB
    int count = 0;
    Item* allItems = DBManager::getInstance().getAllItems(count);
    QString desc = "No description provided.";
    for (int i = 0; i < count; i++) {
        if (allItems[i].getId() == itemId) {
            string d = allItems[i].getDescription();
            if (!d.empty()) desc = QString::fromStdString(d);
            break;
        }
    }

    // Show popup dialog
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Item Description");
    dlg->setMinimumSize(480, 300);

    QVBoxLayout* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(14);

    QLabel* titleLbl = new QLabel(QString("<b>%1</b>").arg(title));
    titleLbl->setObjectName("sectionHeader");
    titleLbl->setWordWrap(true);

    QTextEdit* descEdit = new QTextEdit;
    descEdit->setObjectName("descView");
    descEdit->setReadOnly(true);
    descEdit->setPlainText(desc);

    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setObjectName("clearBtn");
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    lay->addWidget(titleLbl);
    lay->addWidget(descEdit);
    lay->addWidget(closeBtn, 0, Qt::AlignRight);

    // Style the dialog to match the app theme
    dlg->setStyleSheet(R"(
        QDialog { background-color: #080B12; color: #E8E0D0; }
        QLabel#sectionHeader { font-size: 16px; font-weight: 700; color: #F0E8D0; background: transparent; }
        QTextEdit#descView {
            background: #0D1018; border: 1px solid #252D40; border-radius: 8px;
            padding: 12px; color: #C8C0B0; font-size: 13px;
            font-family: 'Trebuchet MS', sans-serif;
        }
        QPushButton#clearBtn {
            background: transparent; border: 1px solid #252D40; border-radius: 7px;
            padding: 9px 20px; color: #5A6480; font-size: 13px;
        }
        QPushButton#clearBtn:hover {
            border-color: #3A4560; color: #8A90A0; background: rgba(255,255,255,0.03);
        }
    )");

    dlg->exec();
    delete dlg;
}

void BuyerDashboard::searchItems() {
    playClick();
    QString keyword = searchField->text().trimmed();
    QString cat = categoryFilter->currentIndex() == 0 ? "" : categoryFilter->currentText();
    QString minStr = minPrice->text().trimmed();
    QString maxStr = maxPrice->text().trimmed();

    DBManager& db = DBManager::getInstance();
    int count = 0;

    if (!minStr.isEmpty() || !maxStr.isEmpty()) {
        double mn = minStr.isEmpty() ? 0 : minStr.toDouble();
        double mx = maxStr.isEmpty() ? 1e9 : maxStr.toDouble();
        Item* items = db.searchByPrice(mn, mx, count);
        loadItems(items, count);
        delete[] items;
    }
    else if (!keyword.isEmpty()) {
        Item* items = db.searchByTitle(keyword.toStdString(), count);
        loadItems(items, count);
        delete[] items;
    }
    else if (!cat.isEmpty()) {
        Item* items = db.searchByCategory(cat.toStdString(), count);
        loadItems(items, count);
        delete[] items;
    }
    else {
        loadAllItems();
    }
}

void BuyerDashboard::placeBid() {
    int row = itemsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select an item to bid on.");
        return;
    }

    int itemId = itemsTable->item(row, 0)->text().toInt();
    QString title = itemsTable->item(row, 1)->text();
    double currentPrice = itemsTable->item(row, 3)->text().mid(1).toDouble();

    bool ok;
    double amount = QInputDialog::getDouble(this,
        "Place Bid",
        QString("Bidding on: <b>%1</b><br>Current price: $%2<br><br>Your bid amount:")
        .arg(title).arg(currentPrice, 0, 'f', 2),
        currentPrice + 1.0, 0.01, 1e9, 2, &ok);

    if (!ok) return;

    // Get or create auction
    auctionMgr.checkAllAuctions();
    Auction* auction = auctionMgr.getAuctionByItemId(itemId);

    if (!auction) {
        // Reload auctions
        int cnt = 0;
        Item* allItems = DBManager::getInstance().getAllItems(cnt);
        for (int i = 0; i < cnt; i++) {
            if (allItems[i].getId() == itemId) {
                auctionMgr.addAuction(allItems[i]);
                break;
            }
        }
        auction = auctionMgr.getAuctionByItemId(itemId);
    }

    if (!auction) {
        QMessageBox::critical(this, "Error", "Auction not found.");
        return;
    }

    bool success = AuctionService::placeBid(*auction, currentUser.getId(), amount);
    if (success) {
        playBidDing();
        QMessageBox::information(this, "Bid Placed",
            QString("Your bid of $%1 was accepted!").arg(amount, 0, 'f', 2));
        loadAllItems();
    }
    else {
        QMessageBox::warning(this, "Bid Failed",
            "Your bid must be higher than the current price, and the auction must be active.");
    }
}

void BuyerDashboard::addToWatchlist() {
    int row = itemsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select an item to watch.");
        return;
    }
    int itemId = itemsTable->item(row, 0)->text().toInt();
    if (currentUser.isWatching(itemId)) {
        QMessageBox::information(this, "Already Watching", "You are already watching this item.");
        return;
    }
    currentUser.addToWatchlist(itemId);
    DBManager::getInstance().updateUser(currentUser);
    QMessageBox::information(this, "Added", "Item added to your watchlist.");
    refreshWatchlist();
}

void BuyerDashboard::removeFromWatchlist() {
    int row = watchTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select an item to remove.");
        return;
    }
    int itemId = watchTable->item(row, 0)->text().toInt();
    currentUser.removeFromWatchlist(itemId);
    DBManager::getInstance().updateUser(currentUser);
    refreshWatchlist();
}

void BuyerDashboard::refreshWatchlist() {
    if (currentUser.getId() <= 0) return;
    watchTable->setRowCount(0);
    int wCount = 0;
    int* watched = currentUser.getWatchlist(wCount);
    if (!watched) return;

    DBManager& db = DBManager::getInstance();
    int iCount = 0;
    Item* allItems = db.getAllItems(iCount);

    for (int w = 0; w < wCount; w++) {
        for (int i = 0; i < iCount; i++) {
            if (allItems[i].getId() == watched[w]) {
                int row = watchTable->rowCount();
                watchTable->insertRow(row);
                watchTable->setItem(row, 0, new QTableWidgetItem(QString::number(allItems[i].getId())));
                watchTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(allItems[i].getTitle())));
                watchTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(allItems[i].getCategory())));
                watchTable->setItem(row, 3, new QTableWidgetItem(
                    QString("$%1").arg(allItems[i].getCurrentPrice(), 0, 'f', 2)));
                int secs = allItems[i].getSecondsRemaining();
                QString t = secs <= 0 ? "Expired" : QString("%1m %2s").arg(secs / 60).arg(secs % 60);
                watchTable->setItem(row, 4, new QTableWidgetItem(t));
                break;
            }
        }
    }
    delete[] watched;
}

void BuyerDashboard::viewNotifications() {
    if (currentUser.getId() <= 0) return;
    notifList->clear();
    int count = 0;
    Notification* notifs = DBManager::getInstance()
        .getNotificationsForUser(currentUser.getId(), count);
    if (count == 0) {
        notifList->addItem("No notifications yet.");
        return;
    }
    playNotification();
    for (int i = count - 1; i >= 0; i--) {
        // Skip internal RATE_REQUEST messages from the notifications tab
        string msg = notifs[i].getMessage();
        if (msg.substr(0, 12) == "RATE_REQUEST") continue;

        QListWidgetItem* item = new QListWidgetItem(
            "🔔  " + QString::fromStdString(msg));
        item->setSizeHint(QSize(0, 44));
        notifList->addItem(item);
    }
    if (notifList->count() == 0)
        notifList->addItem("No notifications yet.");
    delete[] notifs;
}

void BuyerDashboard::loadRateRequests() {
    if (currentUser.getId() <= 0) return;
    rateRequestList->clear();
    pendingRateTargetId = -1;

    int count = 0;
    Notification* notifs = DBManager::getInstance()
        .getNotificationsForUser(currentUser.getId(), count);

    for (int i = 0; i < count; i++) {
        string msg = notifs[i].getMessage();
        if (msg.size() < 12 || msg.substr(0, 12) != "RATE_REQUEST") continue;

        // Format: RATE_REQUEST|seller|<targetId>|<itemTitle>
        string fields[4];
        int fi = 0;
        for (char c : msg) {
            if (c == '|' && fi < 3) fi++;
            else fields[fi] += c;
        }

        // Validate: must have all 4 fields and fields[2] must be numeric
        if (fi < 3 || fields[2].empty()) continue;
        bool isNum = true;
        for (char c : fields[2]) if (!isdigit(c)) { isNum = false; break; }
        if (!isNum) continue;

        int targetId = stoi(fields[2]);
        string role = fields[1];   // "seller" or "buyer"
        string itemTitle = fields[3];

        QString label = QString("Rate the %1 for auction: \"%2\"")
            .arg(QString::fromStdString(role))
            .arg(QString::fromStdString(itemTitle));

        QListWidgetItem* listItem = new QListWidgetItem("⭐  " + label);
        listItem->setData(Qt::UserRole, targetId);
        listItem->setSizeHint(QSize(0, 48));
        rateRequestList->addItem(listItem);
    }
    delete[] notifs;

    if (rateRequestList->count() == 0)
        rateRequestList->addItem("No pending ratings.");
}

void BuyerDashboard::onRateRequestSelected() {
    int row = rateRequestList->currentRow();
    if (row < 0) {
        pendingRateTargetId = -1;
        return;
    }
    QListWidgetItem* it = rateRequestList->item(row);
    QVariant v = it->data(Qt::UserRole);
    if (!v.isValid()) {
        pendingRateTargetId = -1;
        return;
    }
    pendingRateTargetId = v.toInt();
}

void BuyerDashboard::submitRating() {
    if (pendingRateTargetId < 0) {
        QMessageBox::warning(this, "No Selection",
            "Please select an auction from the list to rate.");
        return;
    }

    double rating = buyerRatingBox->value();
    bool ok = AdminService::rateUser(currentUser.getId(), pendingRateTargetId, rating);

    if (ok) {
        QMessageBox::information(this, "Rating Submitted",
            QString("You gave a rating of %1 / 5 ⭐").arg(rating, 0, 'f', 1));
        pendingRateTargetId = -1;
        loadRateRequests();
    }
    else {
        QMessageBox::warning(this, "Error",
            "Could not submit rating. You may have already rated this user.");
    }
}

void BuyerDashboard::onTabChanged(int index) {
    if (index == 1) refreshWatchlist();
    if (index == 2) viewNotifications();
    if (index == 3) loadRateRequests();
}

void BuyerDashboard::applyStyles() {
    setStyleSheet(R"(
    /* ── BASE ── */
    QWidget {
        background-color: #080B12;
        color: #E8E0D0;
        font-family: 'Trebuchet MS', sans-serif;
        font-size: 14px;
    }
    /* ── HEADER BAR ── */
    QFrame#header {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #0D1018, stop:0.5 #111520, stop:1 #0D1018);
        border-bottom: 1px solid #C8922A;
        min-height: 68px;
    }
    QLabel#logo {
        font-size: 20px;
        font-weight: 700;
        color: #F0C060;
        letter-spacing: 1px;
        background: transparent;
    }
    QLabel#welcome {
        color: #C8A870;
        font-size: 13px;
        background: transparent;
        letter-spacing: 0.3px;
    }
    QLabel#repLabel {
        color: #F0C060;
        font-size: 13px;
        font-weight: 600;
        background: transparent;
    }
    /* ── TABS ── */
    QTabWidget#mainTabs::pane {
        border: none;
        background: #080B12;
    }
    QTabWidget#mainTabs QTabBar {
        background: #0D1018;
        border-bottom: 1px solid #1E2535;
    }
    QTabWidget#mainTabs QTabBar::tab {
        background: transparent;
        color: #5A6480;
        padding: 16px 28px;
        border: none;
        border-bottom: 2px solid transparent;
        font-size: 13px;
        font-weight: 600;
        letter-spacing: 0.5px;
        min-width: 120px;
    }
    QTabWidget#mainTabs QTabBar::tab:selected {
        color: #F0C060;
        border-bottom: 2px solid #C8922A;
        background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
            stop:0 transparent, stop:1 rgba(200,146,42,0.06));
    }
    QTabWidget#mainTabs QTabBar::tab:hover:!selected {
        color: #A89060;
        border-bottom: 2px solid #4A3820;
    }
    /* ── SECTION HEADERS ── */
    QLabel#sectionHeader {
        font-size: 20px;
        font-weight: 700;
        color: #F0E8D0;
        letter-spacing: 0.5px;
        background: transparent;
    }
    QLabel#formLabel {
        color: #7A8090;
        font-size: 13px;
        background: transparent;
        font-weight: 600;
        letter-spacing: 0.3px;
    }
    QLabel#statLabel {
        color: #5A6480;
        font-size: 12px;
        background: transparent;
    }
    QLabel#hintLabel {
        color: #404858;
        font-size: 11px;
        background: transparent;
    }
    QLabel#rateResult {
        color: #50C878;
        background: transparent;
        font-weight: 600;
    }
    QLabel { background: transparent; }
    /* ── CARDS / FRAMES ── */
    QFrame#formCard {
        background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
            stop:0 #111520, stop:1 #0D1018);
        border: 1px solid #252D40;
        border-top: 1px solid #2E3850;
        border-radius: 14px;
    }
    QFrame#searchBar {
        background: #0D1018;
        border: 1px solid #1E2535;
        border-radius: 12px;
    }
    /* ── INPUTS ── */
    QLineEdit#field, QLineEdit#formField, QLineEdit#searchField, QLineEdit#priceField,
    QDoubleSpinBox#formField, QSpinBox#formField,
    QDoubleSpinBox#field, QSpinBox#field,
    QComboBox#field, QComboBox#formField, QComboBox#filterCombo {
        background: #0A0D14;
        border: 1px solid #252D40;
        border-radius: 8px;
        padding: 11px 16px;
        color: #E8E0D0;
        font-family: 'Trebuchet MS', sans-serif;
        font-size: 14px;
        selection-background-color: #C8922A;
        selection-color: #080B12;
        min-height: 22px;
    }
    QLineEdit#field:focus, QLineEdit#formField:focus, QLineEdit#searchField:focus,
    QDoubleSpinBox#formField:focus, QSpinBox#formField:focus {
        border: 1px solid #C8922A;
        background: #0C1018;
    }
    QLineEdit#priceField:focus { border: 1px solid #C8922A; }
    QComboBox#field::drop-down, QComboBox#formField::drop-down,
    QComboBox#filterCombo::drop-down { border: none; width: 24px; }
    QComboBox#field QAbstractItemView, QComboBox#formField QAbstractItemView,
    QComboBox#filterCombo QAbstractItemView {
        background: #111520;
        color: #E8E0D0;
        border: 1px solid #252D40;
        selection-background-color: #C8922A;
        selection-color: #080B12;
        outline: none;
    }
    QSpinBox#formField::up-button, QSpinBox#formField::down-button,
    QDoubleSpinBox#formField::up-button, QDoubleSpinBox#formField::down-button {
        background: #1A2030;
        border: none;
        width: 20px;
    }
    /* ── BUTTONS ── */
    QPushButton#primaryBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #C8922A, stop:1 #E0A830);
        color: #080B12;
        border: none;
        border-radius: 9px;
        padding: 13px 28px;
        font-size: 14px;
        font-weight: 700;
        letter-spacing: 0.5px;
        min-height: 22px;
    }
    QPushButton#primaryBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #E0A830, stop:1 #F0C050);
    }
    QPushButton#primaryBtn:pressed {
        background: #A07020;
    }
    QPushButton#adminBtn {
        background: transparent;
        border: 1.5px solid #C8922A;
        border-radius: 9px;
        padding: 13px 28px;
        font-size: 14px;
        font-weight: 600;
        color: #C8A870;
        letter-spacing: 0.3px;
        min-height: 22px;
    }
    QPushButton#adminBtn:hover {
        background: rgba(200,146,42,0.1);
        border-color: #E0A830;
        color: #F0C060;
    }
    QPushButton#linkBtn {
        background: transparent;
        border: none;
        color: #C8922A;
        font-size: 13px;
        font-weight: 600;
        padding: 0 4px;
        text-decoration: underline;
    }
    QPushButton#linkBtn:hover { color: #F0C060; }
    QPushButton#logoutBtn {
        background: transparent;
        border: 1px solid #252D40;
        border-radius: 7px;
        padding: 7px 18px;
        color: #5A6480;
        font-size: 13px;
    }
    QPushButton#logoutBtn:hover {
        border-color: #C8402A;
        color: #E06050;
        background: rgba(200,64,42,0.08);
    }
    QPushButton#actionBtn, QPushButton#searchBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #C8922A, stop:1 #D8A030);
        color: #080B12;
        border: none;
        border-radius: 8px;
        padding: 10px 22px;
        font-weight: 700;
        font-size: 13px;
        letter-spacing: 0.3px;
    }
    QPushButton#actionBtn:hover, QPushButton#searchBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #D8A030, stop:1 #F0B840);
    }
    QPushButton#watchBtn {
        background: transparent;
        border: 1.5px solid #C8922A;
        border-radius: 8px;
        padding: 10px 22px;
        color: #C8922A;
        font-weight: 600;
        font-size: 13px;
    }
    QPushButton#watchBtn:hover {
        background: rgba(200,146,42,0.1);
        color: #F0C060;
        border-color: #E0A830;
    }
    QPushButton#clearBtn {
        background: transparent;
        border: 1px solid #252D40;
        border-radius: 7px;
        padding: 9px 18px;
        color: #5A6480;
        font-size: 13px;
    }
    QPushButton#clearBtn:hover {
        border-color: #3A4560;
        color: #8A90A0;
        background: rgba(255,255,255,0.03);
    }
    QPushButton#dangerBtn {
        background: transparent;
        border: 1.5px solid #C84030;
        border-radius: 8px;
        padding: 10px 22px;
        color: #C84030;
        font-weight: 600;
        font-size: 13px;
    }
    QPushButton#dangerBtn:hover {
        background: rgba(200,64,48,0.12);
        border-color: #E05040;
        color: #E05040;
    }
    QPushButton#presetBtn {
        background: #111828;
        border: 1px solid #2A3448;
        border-radius: 6px;
        padding: 5px 14px;
        color: #8A9AB8;
        font-size: 12px;
        font-weight: 600;
    }
    QPushButton#presetBtn:hover {
        background: #1A2438;
        border-color: #C8922A;
        color: #C8A870;
    }
    /* ── TABLES ── */
    QTableWidget#dataTable {
        background: #0D1018;
        border: 1px solid #1E2535;
        border-radius: 12px;
        gridline-color: #161C28;
        font-family: 'Consolas', monospace;
        font-size: 13px;
    }
    QTableWidget#dataTable::item {
        padding: 11px 14px;
        border: none;
        color: #C8C0B0;
    }
    QTableWidget#dataTable::item:selected {
        background: rgba(200,146,42,0.18);
        color: #F0E0B0;
    }
    QTableWidget#dataTable::item:alternate {
        background: #0A0E16;
    }
    QHeaderView::section {
        background: #0D1018;
        color: #5A6480;
        padding: 11px 14px;
        border: none;
        border-bottom: 1px solid #1E2535;
        font-family: 'Trebuchet MS', sans-serif;
        font-weight: 700;
        font-size: 11px;
        letter-spacing: 1px;
        text-transform: uppercase;
    }
    /* ── LISTS ── */
    QListWidget#notifList {
        background: #0D1018;
        border: 1px solid #1E2535;
        border-radius: 12px;
        font-size: 13px;
        color: #A8A090;
    }
    QListWidget#notifList::item {
        padding: 13px 18px;
        border-bottom: 1px solid #141A24;
    }
    QListWidget#notifList::item:selected {
        background: rgba(200,146,42,0.15);
        color: #F0D090;
    }
    QListWidget#notifList::item:hover:!selected {
        background: rgba(255,255,255,0.03);
    }
    /* ── SCROLL BARS ── */
    QScrollBar:vertical {
        background: #0A0D14;
        width: 6px;
        border-radius: 3px;
    }
    QScrollBar::handle:vertical {
        background: #252D40;
        border-radius: 3px;
        min-height: 30px;
    }
    QScrollBar::handle:vertical:hover { background: #C8922A; }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    QScrollBar:horizontal {
        background: #0A0D14;
        height: 6px;
        border-radius: 3px;
    }
    QScrollBar::handle:horizontal {
        background: #252D40;
        border-radius: 3px;
    }
    QScrollBar::handle:horizontal:hover { background: #C8922A; }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }
    )");
}