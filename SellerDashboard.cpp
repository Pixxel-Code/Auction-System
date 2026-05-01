#include "SoundManager.h"

#include "SellerDashboard.h"
#include "DBManager.h"
#include "ItemService.h"
#include "AdminService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QGroupBox>
#include <QPainter>

static QPixmap makeLogoPixmap(int size) {
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    double cx = size / 2.0, cy = size / 2.0, r = size / 2.0 - 1;

    p.setPen(QPen(QColor("#C8922A"), size * 0.025));
    p.setBrush(QColor("#0D1018"));
    p.drawEllipse(QPointF(cx, cy), r, r);

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

    p.setPen(Qt::NoPen);
    QFont font("Trebuchet MS", int(size * 0.36), QFont::Bold);
    p.setFont(font);
    p.setPen(QColor("#C8922A"));
    p.drawText(QRectF(cx - size * 0.28, cy - size * 0.22, size * 0.56, size * 0.5),
        Qt::AlignCenter, "A");

    double dotR = size * 0.04;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#F0C060"));
    p.drawEllipse(QPointF(cx - size * 0.20, sy + size * 0.02), dotR, dotR);
    p.drawEllipse(QPointF(cx, sy - size * 0.01), dotR, dotR);
    p.drawEllipse(QPointF(cx + size * 0.20, sy + size * 0.02), dotR, dotR);

    p.end();
    return pix;
}

static QLabel* makeDashLogo(int size) {
    QLabel* logo = new QLabel;
    logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    logo->setFixedSize(size, size);
    logo->setPixmap(makeLogoPixmap(size));
    return logo;
}

SellerDashboard::SellerDashboard(QWidget* parent) : QWidget(parent) {
    pendingRateTargetId = -1;
    buildUI();
    applyStyles();
}

void SellerDashboard::setUser(User u) {
    currentUser = u;
    welcomeLabel->setText(QString("Welcome, <b>%1</b>  |  Seller Account")
        .arg(QString::fromStdString(u.getUsername())));
    double rep = u.getReputation();
    reputationLabel->setText(rep > 0
        ? QString("⭐ %1").arg(rep, 0, 'f', 1)
        : QString("⭐ No ratings yet"));
    setupAuctions();
    refresh();
}

void SellerDashboard::setupAuctions() {
    int count = 0;
    Item* items = DBManager::getInstance().getAllItems(count);
    for (int i = 0; i < count; i++)
        if (items[i].getSellerId() == currentUser.getId())
            auctionMgr.addAuction(items[i]);
}

void SellerDashboard::buildUI() {
    QVBoxLayout* main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);
    main->setSpacing(0);

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
    connect(logoutBtn, &QPushButton::clicked, this, &SellerDashboard::logoutRequested);

    hlay->addWidget(logo);
    hlay->addStretch();
    hlay->addWidget(welcomeLabel);
    hlay->addSpacing(16);
    hlay->addWidget(reputationLabel);
    hlay->addSpacing(16);
    hlay->addWidget(logoutBtn);

    tabs = new QTabWidget;
    tabs->setObjectName("mainTabs");
    tabs->addTab(buildListingsTab(), "📦  My Listings");
    tabs->addTab(buildCreateTab(), "➕  Create Listing");
    tabs->addTab(buildNotificationsTab(), "🔔  Notifications");
    tabs->addTab(buildRateTab(), "⭐  Rate Users");

    connect(tabs, &QTabWidget::currentChanged, this, [this](int i) {
        if (i == 0) refreshItems();
        if (i == 2) viewNotifications();
        if (i == 3) loadRateRequests();
        });

    main->addWidget(header);
    main->addWidget(tabs);
}

QWidget* SellerDashboard::buildListingsTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    QLabel* hdr = new QLabel("My Active Listings");
    hdr->setObjectName("sectionHeader");

    listingsTable = new QTableWidget;
    listingsTable->setObjectName("dataTable");
    listingsTable->setColumnCount(7);
    listingsTable->setHorizontalHeaderLabels(
        { "ID","Title","Category","Base Price","Current Price","Time Left","Bids" });
    listingsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    listingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    listingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listingsTable->verticalHeader()->hide();
    listingsTable->setAlternatingRowColors(true);

    QHBoxLayout* actRow = new QHBoxLayout;
    closeAuctionBtn = new QPushButton("🔒  Close Auction");
    closeAuctionBtn->setObjectName("dangerBtn");
    closeAuctionBtn->setCursor(Qt::PointingHandCursor);
    QPushButton* refreshBtn = new QPushButton("🔄  Refresh");
    refreshBtn->setObjectName("clearBtn");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    connect(closeAuctionBtn, &QPushButton::clicked, this, &SellerDashboard::closeSelectedAuction);
    connect(refreshBtn, &QPushButton::clicked, this, &SellerDashboard::refreshItems);

    actRow->addStretch();
    actRow->addWidget(refreshBtn);
    actRow->addWidget(closeAuctionBtn);

    lay->addWidget(hdr);
    lay->addWidget(listingsTable);
    lay->addLayout(actRow);
    return w;
}

QWidget* SellerDashboard::buildCreateTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* outer = new QVBoxLayout(w);
    outer->setContentsMargins(20, 20, 20, 20);
    outer->setAlignment(Qt::AlignTop);

    QLabel* hdr = new QLabel("Create New Listing");
    hdr->setObjectName("sectionHeader");

    QFrame* card = new QFrame;
    card->setObjectName("formCard");
    card->setMaximumWidth(600);

    itemTitle = new QLineEdit;
    itemTitle->setPlaceholderText("e.g. Vintage Camera");
    itemTitle->setObjectName("formField");

    // ── Description (NEW) ──────────────────────────────────────
    itemDescription = new QTextEdit;
    itemDescription->setPlaceholderText(
        "Describe your item — condition, features, included accessories...");
    itemDescription->setObjectName("descField");
    itemDescription->setFixedHeight(100);
    // ──────────────────────────────────────────────────────────

    itemPrice = new QDoubleSpinBox;
    itemPrice->setPrefix("$ ");
    itemPrice->setRange(0.01, 1000000);
    itemPrice->setDecimals(2);
    itemPrice->setValue(100.0);
    itemPrice->setObjectName("formField");

    itemCategory = new QComboBox;
    itemCategory->addItems({ "Electronics","Phones","Clothing","Vehicles","Furniture","Other" });
    itemCategory->setObjectName("formField");

    itemDuration = new QSpinBox;
    itemDuration->setRange(60, 604800);
    itemDuration->setValue(3600);
    itemDuration->setSuffix(" seconds");
    itemDuration->setSingleStep(3600);
    itemDuration->setObjectName("formField");

    QHBoxLayout* durRow = new QHBoxLayout;
    auto addPreset = [this, &durRow](const QString& label, int secs) {
        QPushButton* btn = new QPushButton(label);
        btn->setObjectName("presetBtn");
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, [this, secs]() { itemDuration->setValue(secs); });
        durRow->addWidget(btn);
        };
    addPreset("1h", 3600);
    addPreset("6h", 21600);
    addPreset("12h", 43200);
    addPreset("24h", 86400);
    addPreset("3d", 259200);
    addPreset("7d", 604800);
    durRow->addStretch();

    createBtn = new QPushButton("🚀  Publish Listing");
    createBtn->setObjectName("primaryBtn");
    createBtn->setCursor(Qt::PointingHandCursor);
    createBtn->setFixedHeight(46);
    connect(createBtn, &QPushButton::clicked, this, &SellerDashboard::createItem);

    QVBoxLayout* cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(28, 28, 28, 28);
    cardLay->setSpacing(18);

    auto makeRow = [](const QString& lbl, QWidget* widget, bool top = false) {
        QHBoxLayout* h = new QHBoxLayout;
        QLabel* l = new QLabel(lbl);
        l->setFixedWidth(130);
        l->setAlignment(top
            ? (Qt::AlignRight | Qt::AlignTop)
            : (Qt::AlignRight | Qt::AlignVCenter));
        if (top) l->setContentsMargins(0, 8, 0, 0);
        l->setObjectName("formLabel");
        h->addWidget(l);
        h->addWidget(widget, 1);
        return h;
        };

    cardLay->addLayout(makeRow("Item Title:", itemTitle));
    cardLay->addLayout(makeRow("Description:", itemDescription, true));  // NEW
    cardLay->addLayout(makeRow("Starting Price:", itemPrice));
    cardLay->addLayout(makeRow("Category:", itemCategory));
    cardLay->addLayout(makeRow("Duration:", itemDuration));
    cardLay->addLayout(durRow);
    cardLay->addSpacing(8);
    cardLay->addWidget(createBtn);

    outer->addWidget(hdr);
    outer->addSpacing(16);
    outer->addWidget(card);

    return w;
}

QWidget* SellerDashboard::buildNotificationsTab() {
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
    connect(refreshBtn, &QPushButton::clicked, this, &SellerDashboard::viewNotifications);

    lay->addWidget(hdr);
    lay->addWidget(notifList);
    lay->addWidget(refreshBtn, 0, Qt::AlignRight);

    return w;
}

QWidget* SellerDashboard::buildRateTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    QLabel* hdr = new QLabel("Rate Users");
    hdr->setObjectName("sectionHeader");

    QLabel* hint = new QLabel("Select a completed auction below, then submit your rating for the buyer:");
    hint->setObjectName("hintLabel");

    rateRequestList = new QListWidget;
    rateRequestList->setObjectName("notifList");

    QHBoxLayout* rateRow = new QHBoxLayout;
    QLabel* starLbl = new QLabel("Your Rating (1–5):");
    starLbl->setObjectName("formLabel");

    sellerRatingBox = new QDoubleSpinBox;
    sellerRatingBox->setRange(1.0, 5.0);
    sellerRatingBox->setSingleStep(0.5);
    sellerRatingBox->setValue(5.0);
    sellerRatingBox->setObjectName("formField");
    sellerRatingBox->setFixedWidth(90);

    submitRatingBtn = new QPushButton("⭐  Submit Rating");
    submitRatingBtn->setObjectName("primaryBtn");
    submitRatingBtn->setCursor(Qt::PointingHandCursor);

    rateRow->addWidget(starLbl);
    rateRow->addWidget(sellerRatingBox);
    rateRow->addSpacing(12);
    rateRow->addWidget(submitRatingBtn);
    rateRow->addStretch();

    connect(rateRequestList, &QListWidget::currentRowChanged,
        this, &SellerDashboard::onRateRequestSelected);
    connect(submitRatingBtn, &QPushButton::clicked,
        this, &SellerDashboard::submitRating);

    lay->addWidget(hdr);
    lay->addWidget(hint);
    lay->addWidget(rateRequestList);
    lay->addLayout(rateRow);

    return w;
}

void SellerDashboard::loadRateRequests() {
    if (currentUser.getId() <= 0) return;
    rateRequestList->clear();
    pendingRateTargetId = -1;

    int count = 0;
    Notification* notifs = DBManager::getInstance()
        .getNotificationsForUser(currentUser.getId(), count);

    for (int i = 0; i < count; i++) {
        string msg = notifs[i].getMessage();
        if (msg.size() < 12 || msg.substr(0, 12) != "RATE_REQUEST") continue;

        string fields[4];
        int fi = 0;
        for (char c : msg) {
            if (c == '|' && fi < 3) fi++;
            else fields[fi] += c;
        }

        if (fi < 3 || fields[2].empty()) continue;
        bool isNum = true;
        for (char c : fields[2]) if (!isdigit(c)) { isNum = false; break; }
        if (!isNum) continue;

        int    targetId = stoi(fields[2]);
        string role = fields[1];
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

void SellerDashboard::onRateRequestSelected() {
    int row = rateRequestList->currentRow();
    if (row < 0) { pendingRateTargetId = -1; return; }
    QVariant v = rateRequestList->item(row)->data(Qt::UserRole);
    if (!v.isValid()) { pendingRateTargetId = -1; return; }
    pendingRateTargetId = v.toInt();
}

void SellerDashboard::submitRating() {
    if (pendingRateTargetId < 0) {
        QMessageBox::warning(this, "No Selection",
            "Please select an auction from the list to rate.");
        return;
    }

    double rating = sellerRatingBox->value();
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

void SellerDashboard::createItem() {
    QString title = itemTitle->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter an item title.");
        return;
    }
    playClick();

    double  price = itemPrice->value();
    QString category = itemCategory->currentText();
    int     duration = itemDuration->value();
    QString description = itemDescription->toPlainText().trimmed();   // NEW

    ItemService::createItem(
        title.toStdString(), price,
        currentUser.getId(),
        category.toStdString(),
        currentUser, duration,
        description.toStdString());   // NEW

    QMessageBox::information(this, "Success",
        QString("'%1' has been listed successfully!").arg(title));

    itemTitle->clear();
    itemDescription->clear();   // NEW
    itemPrice->setValue(100.0);

    int count = 0;
    Item* all = DBManager::getInstance().getAllItems(count);
    for (int i = 0; i < count; i++) {
        if (all[i].getSellerId() == currentUser.getId() &&
            !auctionMgr.getAuctionByItemId(all[i].getId()))
            auctionMgr.addAuction(all[i]);
    }

    tabs->setCurrentIndex(0);
    refreshItems();
}

void SellerDashboard::refreshItems() {
    if (currentUser.getId() <= 0) return;
    listingsTable->setRowCount(0);
    int count = 0;
    Item* all = DBManager::getInstance().getAllItems(count);

    for (int i = 0; i < count; i++) {
        if (all[i].getSellerId() != currentUser.getId()) continue;
        if (all[i].isExpired()) continue;

        int bCount = 0;
        Bid* bids = DBManager::getInstance().getBidsForItem(all[i].getId(), bCount);
        delete[] bids;

        int row = listingsTable->rowCount();
        listingsTable->insertRow(row);
        listingsTable->setItem(row, 0, new QTableWidgetItem(QString::number(all[i].getId())));
        listingsTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(all[i].getTitle())));
        listingsTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(all[i].getCategory())));
        listingsTable->setItem(row, 3, new QTableWidgetItem(QString("$%1").arg(all[i].getBasePrice(), 0, 'f', 2)));
        listingsTable->setItem(row, 4, new QTableWidgetItem(QString("$%1").arg(all[i].getCurrentPrice(), 0, 'f', 2)));

        int secs = all[i].getSecondsRemaining();
        QString t = secs <= 0 ? "Expired" :
            secs < 3600 ? QString("%1m").arg(secs / 60) :
            QString("%1h %2m").arg(secs / 3600).arg((secs % 3600) / 60);
        listingsTable->setItem(row, 5, new QTableWidgetItem(t));
        listingsTable->setItem(row, 6, new QTableWidgetItem(QString::number(bCount)));
    }
}

void SellerDashboard::closeSelectedAuction() {
    int row = listingsTable->currentRow();
    if (row < 0) { QMessageBox::warning(this, "", "Please select a listing."); return; }

    int itemId = listingsTable->item(row, 0)->text().toInt();
    QString title = listingsTable->item(row, 1)->text();

    auto reply = QMessageBox::question(this, "Close Auction",
        QString("Close auction for '%1'? This will finalize the winner.").arg(title));
    if (reply != QMessageBox::Yes) return;

    Auction* a = auctionMgr.getAuctionByItemId(itemId);
    if (!a) { QMessageBox::warning(this, "Error", "Auction not found."); return; }

    a->closeAuction();
    auctionMgr.removeAuction(itemId);
    DBManager::getInstance().expireItem(itemId);

    playBidDing();
    QMessageBox::information(this, "Closed", "Auction closed successfully.");
    refreshItems();
}

void SellerDashboard::viewNotifications() {
    if (currentUser.getId() <= 0) return;
    notifList->clear();
    int count = 0;
    Notification* notifs = DBManager::getInstance()
        .getNotificationsForUser(currentUser.getId(), count);
    if (count == 0) { notifList->addItem("No notifications yet."); return; }
    playNotification();
    for (int i = count - 1; i >= 0; i--) {
        string msg = notifs[i].getMessage();
        if (msg.size() >= 12 && msg.substr(0, 12) == "RATE_REQUEST") continue;
        QListWidgetItem* it = new QListWidgetItem(
            "🔔  " + QString::fromStdString(msg));
        it->setSizeHint(QSize(0, 44));
        notifList->addItem(it);
    }
    if (notifList->count() == 0)
        notifList->addItem("No notifications yet.");
    delete[] notifs;
}

void SellerDashboard::refresh() {
    refreshItems();
    viewNotifications();
}

void SellerDashboard::applyStyles() {
    setStyleSheet(R"(
    QWidget {
        background-color: #080B12; color: #E8E0D0;
        font-family: 'Trebuchet MS', sans-serif; font-size: 14px;
    }
    QFrame#header {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #0D1018, stop:0.5 #111520, stop:1 #0D1018);
        border-bottom: 1px solid #C8922A; min-height: 68px;
    }
    QLabel#welcome { color: #C8A870; font-size: 13px; background: transparent; }
    QLabel#repLabel { color: #F0C060; font-size: 13px; font-weight: 600; background: transparent; }
    QTabWidget#mainTabs::pane { border: none; background: #080B12; }
    QTabWidget#mainTabs QTabBar { background: #0D1018; border-bottom: 1px solid #1E2535; }
    QTabWidget#mainTabs QTabBar::tab {
        background: transparent; color: #5A6480; padding: 16px 28px;
        border: none; border-bottom: 2px solid transparent;
        font-size: 13px; font-weight: 600; min-width: 120px;
    }
    QTabWidget#mainTabs QTabBar::tab:selected {
        color: #F0C060; border-bottom: 2px solid #C8922A;
        background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
            stop:0 transparent, stop:1 rgba(200,146,42,0.06));
    }
    QTabWidget#mainTabs QTabBar::tab:hover:!selected {
        color: #A89060; border-bottom: 2px solid #4A3820;
    }
    QLabel#sectionHeader {
        font-size: 20px; font-weight: 700; color: #F0E8D0;
        letter-spacing: 0.5px; background: transparent;
    }
    QLabel#formLabel {
        color: #7A8090; font-size: 13px; background: transparent;
        font-weight: 600; letter-spacing: 0.3px;
    }
    QLabel#hintLabel { color: #404858; font-size: 11px; background: transparent; }
    QLabel { background: transparent; }
    QFrame#formCard {
        background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
            stop:0 #111520, stop:1 #0D1018);
        border: 1px solid #252D40; border-top: 1px solid #2E3850; border-radius: 14px;
    }
    QLineEdit#formField, QDoubleSpinBox#formField, QSpinBox#formField, QComboBox#formField {
        background: #0A0D14; border: 1px solid #252D40; border-radius: 8px;
        padding: 11px 16px; color: #E8E0D0; font-size: 14px; min-height: 22px;
    }
    QLineEdit#formField:focus, QDoubleSpinBox#formField:focus,
    QSpinBox#formField:focus { border: 1px solid #C8922A; background: #0C1018; }
    QComboBox#formField::drop-down { border: none; width: 24px; }
    QComboBox#formField QAbstractItemView {
        background: #111520; color: #E8E0D0; border: 1px solid #252D40;
        selection-background-color: #C8922A; selection-color: #080B12;
    }
    QSpinBox#formField::up-button, QSpinBox#formField::down-button,
    QDoubleSpinBox#formField::up-button, QDoubleSpinBox#formField::down-button {
        background: #1A2030; border: none; width: 20px;
    }
    QTextEdit#descField {
        background: #0A0D14; border: 1px solid #252D40; border-radius: 8px;
        padding: 10px 14px; color: #E8E0D0; font-size: 13px;
        font-family: 'Trebuchet MS', sans-serif;
    }
    QTextEdit#descField:focus { border: 1px solid #C8922A; background: #0C1018; }
    QPushButton#primaryBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #C8922A, stop:1 #E0A830);
        color: #080B12; border: none; border-radius: 9px;
        padding: 13px 28px; font-size: 14px; font-weight: 700; min-height: 22px;
    }
    QPushButton#primaryBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #E0A830, stop:1 #F0C050);
    }
    QPushButton#primaryBtn:pressed { background: #A07020; }
    QPushButton#logoutBtn {
        background: transparent; border: 1px solid #252D40;
        border-radius: 7px; padding: 7px 18px; color: #5A6480; font-size: 13px;
    }
    QPushButton#logoutBtn:hover {
        border-color: #C8402A; color: #E06050; background: rgba(200,64,42,0.08);
    }
    QPushButton#clearBtn {
        background: transparent; border: 1px solid #252D40;
        border-radius: 7px; padding: 9px 18px; color: #5A6480; font-size: 13px;
    }
    QPushButton#clearBtn:hover {
        border-color: #3A4560; color: #8A90A0; background: rgba(255,255,255,0.03);
    }
    QPushButton#dangerBtn {
        background: transparent; border: 1.5px solid #C84030;
        border-radius: 8px; padding: 10px 22px; color: #C84030; font-weight: 600;
    }
    QPushButton#dangerBtn:hover {
        background: rgba(200,64,48,0.12); border-color: #E05040; color: #E05040;
    }
    QPushButton#presetBtn {
        background: #111828; border: 1px solid #2A3448; border-radius: 6px;
        padding: 5px 14px; color: #8A9AB8; font-size: 12px; font-weight: 600;
    }
    QPushButton#presetBtn:hover {
        background: #1A2438; border-color: #C8922A; color: #C8A870;
    }
    QTableWidget#dataTable {
        background: #0D1018; border: 1px solid #1E2535; border-radius: 12px;
        gridline-color: #161C28; font-family: 'Consolas', monospace; font-size: 13px;
    }
    QTableWidget#dataTable::item { padding: 11px 14px; border: none; color: #C8C0B0; }
    QTableWidget#dataTable::item:selected { background: rgba(200,146,42,0.18); color: #F0E0B0; }
    QTableWidget#dataTable::item:alternate { background: #0A0E16; }
    QHeaderView::section {
        background: #0D1018; color: #5A6480; padding: 11px 14px;
        border: none; border-bottom: 1px solid #1E2535;
        font-weight: 700; font-size: 11px; letter-spacing: 1px;
    }
    QListWidget#notifList {
        background: #0D1018; border: 1px solid #1E2535;
        border-radius: 12px; font-size: 13px; color: #A8A090;
    }
    QListWidget#notifList::item { padding: 13px 18px; border-bottom: 1px solid #141A24; }
    QListWidget#notifList::item:selected { background: rgba(200,146,42,0.15); color: #F0D090; }
    QListWidget#notifList::item:hover:!selected { background: rgba(255,255,255,0.03); }
    QScrollBar:vertical { background: #0A0D14; width: 6px; border-radius: 3px; }
    QScrollBar::handle:vertical { background: #252D40; border-radius: 3px; min-height: 30px; }
    QScrollBar::handle:vertical:hover { background: #C8922A; }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    QScrollBar:horizontal { background: #0A0D14; height: 6px; border-radius: 3px; }
    QScrollBar::handle:horizontal { background: #252D40; border-radius: 3px; }
    QScrollBar::handle:horizontal:hover { background: #C8922A; }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }
    )");
}