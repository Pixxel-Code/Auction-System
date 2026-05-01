#include "AdminDashboard.h"
#include "AdminService.h"
#include "DBManager.h"
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

AdminDashboard::AdminDashboard(QWidget* parent) : QWidget(parent) {
    buildUI();
    applyStyles();
}

void AdminDashboard::setUser(User u) {
    currentUser = u;
    welcomeLabel->setText(QString("🛡  Admin Panel  |  <b>%1</b>")
        .arg(QString::fromStdString(u.getUsername())));
    refresh();
}

void AdminDashboard::buildUI() {
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

    welcomeLabel = new QLabel("");
    welcomeLabel->setObjectName("welcome");

    QPushButton* logoutBtn = new QPushButton("Logout");
    logoutBtn->setObjectName("logoutBtn");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    connect(logoutBtn, &QPushButton::clicked, this, &AdminDashboard::logoutRequested);

    hlay->addWidget(logo);
    hlay->addStretch();
    hlay->addWidget(welcomeLabel);
    hlay->addSpacing(16);
    hlay->addWidget(logoutBtn);

    tabs = new QTabWidget;
    tabs->setObjectName("mainTabs");
    tabs->addTab(buildUsersTab(), "👥  Users");
    tabs->addTab(buildItemsTab(), "📦  Items");
    tabs->addTab(buildBidsTab(), "💰  Bids");
    tabs->addTab(buildRateTab(), "⭐  Rate Users");

    connect(tabs, &QTabWidget::currentChanged, this, [this](int i) {
        if (i == 0) loadUsers();
        if (i == 1) loadItems();
        if (i == 2) loadBids();
        });

    main->addWidget(header);
    main->addWidget(tabs);
}

QWidget* AdminDashboard::buildUsersTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    QHBoxLayout* topRow = new QHBoxLayout;
    QLabel* hdr = new QLabel("User Management");
    hdr->setObjectName("sectionHeader");
    userStatsLabel = new QLabel("");
    userStatsLabel->setObjectName("statLabel");
    topRow->addWidget(hdr);
    topRow->addStretch();
    topRow->addWidget(userStatsLabel);

    usersTable = new QTableWidget;
    usersTable->setObjectName("dataTable");
    usersTable->setColumnCount(6);
    usersTable->setHorizontalHeaderLabels({ "ID","Username","Role","Reputation","Ratings","Watching" });
    usersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    usersTable->verticalHeader()->hide();
    usersTable->setAlternatingRowColors(true);

    QHBoxLayout* actRow = new QHBoxLayout;
    deleteUserBtn = new QPushButton("🗑  Delete User");
    deleteUserBtn->setObjectName("dangerBtn");
    deleteUserBtn->setCursor(Qt::PointingHandCursor);
    QPushButton* refreshBtn = new QPushButton("🔄  Refresh");
    refreshBtn->setObjectName("clearBtn");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    connect(deleteUserBtn, &QPushButton::clicked, this, &AdminDashboard::deleteUser);
    connect(refreshBtn, &QPushButton::clicked, this, &AdminDashboard::loadUsers);

    actRow->addStretch();
    actRow->addWidget(refreshBtn);
    actRow->addWidget(deleteUserBtn);

    lay->addLayout(topRow);
    lay->addWidget(usersTable);
    lay->addLayout(actRow);
    return w;
}

QWidget* AdminDashboard::buildItemsTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    QLabel* hdr = new QLabel("Item Management");
    hdr->setObjectName("sectionHeader");

    itemsTable = new QTableWidget;
    itemsTable->setObjectName("dataTable");
    itemsTable->setColumnCount(7);
    itemsTable->setHorizontalHeaderLabels(
        { "ID","Title","Category","Base Price","Current Price","Seller ID","Status" });
    itemsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    itemsTable->verticalHeader()->hide();
    itemsTable->setAlternatingRowColors(true);

    QHBoxLayout* actRow = new QHBoxLayout;
    deleteItemBtn = new QPushButton("🗑  Delete Item");
    deleteItemBtn->setObjectName("dangerBtn");
    deleteItemBtn->setCursor(Qt::PointingHandCursor);
    QPushButton* refreshBtn = new QPushButton("🔄  Refresh");
    refreshBtn->setObjectName("clearBtn");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    connect(deleteItemBtn, &QPushButton::clicked, this, &AdminDashboard::deleteItem);
    connect(refreshBtn, &QPushButton::clicked, this, &AdminDashboard::loadItems);

    actRow->addStretch();
    actRow->addWidget(refreshBtn);
    actRow->addWidget(deleteItemBtn);

    lay->addWidget(hdr);
    lay->addWidget(itemsTable);
    lay->addLayout(actRow);
    return w;
}

QWidget* AdminDashboard::buildBidsTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    QLabel* hdr = new QLabel("All Bids");
    hdr->setObjectName("sectionHeader");

    bidsTable = new QTableWidget;
    bidsTable->setObjectName("dataTable");
    bidsTable->setColumnCount(3);
    bidsTable->setHorizontalHeaderLabels({ "User ID","Item ID","Bid Amount" });
    bidsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bidsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    bidsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bidsTable->verticalHeader()->hide();
    bidsTable->setAlternatingRowColors(true);

    QPushButton* refreshBtn = new QPushButton("🔄  Refresh");
    refreshBtn->setObjectName("clearBtn");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    connect(refreshBtn, &QPushButton::clicked, this, &AdminDashboard::loadBids);

    lay->addWidget(hdr);
    lay->addWidget(bidsTable);
    lay->addWidget(refreshBtn, 0, Qt::AlignRight);
    return w;
}

QWidget* AdminDashboard::buildRateTab() {
    QWidget* w = new QWidget;
    QVBoxLayout* outer = new QVBoxLayout(w);
    outer->setContentsMargins(20, 20, 20, 20);
    outer->setAlignment(Qt::AlignTop);

    QLabel* hdr = new QLabel("Rate a User");
    hdr->setObjectName("sectionHeader");

    QFrame* card = new QFrame;
    card->setObjectName("formCard");
    card->setMaximumWidth(480);

    QVBoxLayout* cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(28, 28, 28, 28);
    cardLay->setSpacing(18);

    auto makeRow = [](const QString& lbl, QWidget* fw) {
        QHBoxLayout* h = new QHBoxLayout;
        QLabel* l = new QLabel(lbl);
        l->setFixedWidth(130);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        l->setObjectName("formLabel");
        h->addWidget(l);
        h->addWidget(fw, 1);
        return h;
        };

    raterIdBox = new QSpinBox;
    raterIdBox->setRange(1, 99999);
    raterIdBox->setObjectName("formField");

    targetIdBox = new QSpinBox;
    targetIdBox->setRange(1, 99999);
    targetIdBox->setObjectName("formField");

    ratingBox = new QDoubleSpinBox;
    ratingBox->setRange(1.0, 5.0);
    ratingBox->setSingleStep(0.5);
    ratingBox->setValue(5.0);
    ratingBox->setObjectName("formField");

    rateBtn = new QPushButton("⭐  Submit Rating");
    rateBtn->setObjectName("primaryBtn");
    rateBtn->setCursor(Qt::PointingHandCursor);
    rateBtn->setFixedHeight(44);
    connect(rateBtn, &QPushButton::clicked, this, &AdminDashboard::rateUser);

    rateResult = new QLabel("");
    rateResult->setObjectName("rateResult");
    rateResult->setAlignment(Qt::AlignCenter);

    // Star rating hint
    QLabel* hint = new QLabel("Scale: 1.0 (Poor) → 5.0 (Excellent)");
    hint->setObjectName("hintLabel");

    cardLay->addLayout(makeRow("Rater User ID:", raterIdBox));
    cardLay->addLayout(makeRow("Target User ID:", targetIdBox));
    cardLay->addLayout(makeRow("Rating:", ratingBox));
    cardLay->addWidget(hint);
    cardLay->addWidget(rateBtn);
    cardLay->addWidget(rateResult);

    // Show users for reference
    QLabel* refHdr = new QLabel("User ID Reference");
    refHdr->setObjectName("sectionHeader");
    refHdr->setStyleSheet("font-size:15px;");

    QTableWidget* refTable = new QTableWidget;
    refTable->setObjectName("dataTable");
    refTable->setColumnCount(3);
    refTable->setHorizontalHeaderLabels({ "ID","Username","Current Rep" });
    refTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    refTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    refTable->verticalHeader()->hide();
    refTable->setMaximumHeight(200);
    refTable->setAlternatingRowColors(true);

    // populate ref table
    int count = 0;
    User* users = AdminService::getUsers(count);
    for (int i = 0; i < count; i++) {
        int row = refTable->rowCount();
        refTable->insertRow(row);
        refTable->setItem(row, 0, new QTableWidgetItem(QString::number(users[i].getId())));
        refTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(users[i].getUsername())));
        double rep = users[i].getReputation();
        refTable->setItem(row, 2, new QTableWidgetItem(
            rep > 0 ? QString::number(rep, 'f', 2) : "None"));
    }

    outer->addWidget(hdr);
    outer->addSpacing(16);
    outer->addWidget(card);
    outer->addSpacing(20);
    outer->addWidget(refHdr);
    outer->addSpacing(8);
    outer->addWidget(refTable);
    return w;
}

void AdminDashboard::loadUsers() {
    usersTable->setRowCount(0);
    int count = 0;
    User* users = AdminService::getUsers(count);
    userStatsLabel->setText(QString("Total Users: %1").arg(count));

    for (int i = 0; i < count; i++) {
        int row = usersTable->rowCount();
        usersTable->insertRow(row);
        usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(users[i].getId())));
        usersTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(users[i].getUsername())));

        QTableWidgetItem* roleItem = new QTableWidgetItem(QString::fromStdString(users[i].getRole()));
        QString role = QString::fromStdString(users[i].getRole());
        if (role == "admin") roleItem->setForeground(QColor("#f59e0b"));
        else if (role == "seller") roleItem->setForeground(QColor("#34d399"));
        else roleItem->setForeground(QColor("#60a5fa"));
        usersTable->setItem(row, 2, roleItem);

        double rep = users[i].getReputation();
        usersTable->setItem(row, 3, new QTableWidgetItem(
            rep > 0 ? QString::number(rep, 'f', 2) : "—"));
        usersTable->setItem(row, 4, new QTableWidgetItem(
            QString::number(users[i].getRatingCount())));

        int wCount = 0;
        int* wl = users[i].getWatchlist(wCount);
        usersTable->setItem(row, 5, new QTableWidgetItem(QString::number(wCount)));
        delete[] wl;
    }
}

void AdminDashboard::loadItems() {
    itemsTable->setRowCount(0);
    int count = 0;
    Item* items = AdminService::getItems(count);

    for (int i = 0; i < count; i++) {
        int row = itemsTable->rowCount();
        itemsTable->insertRow(row);
        itemsTable->setItem(row, 0, new QTableWidgetItem(QString::number(items[i].getId())));
        itemsTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(items[i].getTitle())));
        itemsTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(items[i].getCategory())));
        itemsTable->setItem(row, 3, new QTableWidgetItem(QString("$%1").arg(items[i].getBasePrice(), 0, 'f', 2)));
        itemsTable->setItem(row, 4, new QTableWidgetItem(QString("$%1").arg(items[i].getCurrentPrice(), 0, 'f', 2)));
        itemsTable->setItem(row, 5, new QTableWidgetItem(QString::number(items[i].getSellerId())));

        QTableWidgetItem* st = new QTableWidgetItem(items[i].isExpired() ? "Expired" : "Active");
        st->setForeground(items[i].isExpired() ? QColor("#f87171") : QColor("#34d399"));
        itemsTable->setItem(row, 6, st);
    }
}

void AdminDashboard::loadBids() {
    bidsTable->setRowCount(0);
    int count = 0;
    Bid* bids = AdminService::getBids(count);

    for (int i = 0; i < count; i++) {
        int row = bidsTable->rowCount();
        bidsTable->insertRow(row);
        bidsTable->setItem(row, 0, new QTableWidgetItem(QString::number(bids[i].getUserId())));
        bidsTable->setItem(row, 1, new QTableWidgetItem(QString::number(bids[i].getItemId())));
        bidsTable->setItem(row, 2, new QTableWidgetItem(
            QString("$%1").arg(bids[i].getAmount(), 0, 'f', 2)));
    }
}

void AdminDashboard::deleteUser() {
    int row = usersTable->currentRow();
    if (row < 0) { QMessageBox::warning(this, "", "Please select a user."); return; }

    int id = usersTable->item(row, 0)->text().toInt();
    QString name = usersTable->item(row, 1)->text();
    QString role = usersTable->item(row, 2)->text();

    if (role == "admin") {
        QMessageBox::warning(this, "Cannot Delete", "You cannot delete the admin account.");
        return;
    }

    auto reply = QMessageBox::question(this, "Confirm Delete",
        QString("Delete user '%1' (ID: %2)?\nThis action cannot be undone.").arg(name).arg(id));
    if (reply != QMessageBox::Yes) return;

    bool ok = AdminService::deleteUser(id, currentUser);
    if (ok) {
        QMessageBox::information(this, "Deleted",
            QString("User '%1' deleted.").arg(name));
        loadUsers();
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to delete user.");
    }
}

void AdminDashboard::deleteItem() {
    int row = itemsTable->currentRow();
    if (row < 0) { QMessageBox::warning(this, "", "Please select an item."); return; }

    int id = itemsTable->item(row, 0)->text().toInt();
    QString title = itemsTable->item(row, 1)->text();

    auto reply = QMessageBox::question(this, "Confirm Delete",
        QString("Delete item '%1' (ID: %2)?\nAll bids for this item will also be deleted.").arg(title).arg(id));
    if (reply != QMessageBox::Yes) return;

    bool ok = AdminService::deleteItem(id, currentUser);
    if (ok) {
        QMessageBox::information(this, "Deleted",
            QString("Item '%1' and its bids deleted.").arg(title));
        loadItems();
        loadBids();
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to delete item.");
    }
}

void AdminDashboard::rateUser() {
    int rater = raterIdBox->value();
    int target = targetIdBox->value();
    double rat = ratingBox->value();

    bool ok = AdminService::rateUser(rater, target, rat);
    if (ok) {
        rateResult->setStyleSheet("color:#34d399;");
        rateResult->setText(QString("✓ Rating of %1/5 submitted for user ID %2")
            .arg(rat, '0', 'f', 1).arg(target));
        double rep = AdminService::getReputation(target);
        rateResult->setText(rateResult->text() +
            QString("\n  New reputation: %1").arg(rep, '0', 'f', 2));
    }
    else {
        rateResult->setStyleSheet("color:#f87171;");
        rateResult->setText("✗ Rating failed. Check user IDs and rating range (1-5).");
    }
}

void AdminDashboard::refresh() {
    loadUsers();
    loadItems();
    loadBids();
}

void AdminDashboard::applyStyles() {
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