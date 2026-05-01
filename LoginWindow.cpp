#include "LoginWindow.h"
#include "AuthService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFrame>


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

LoginWindow::LoginWindow(QWidget* parent) : QWidget(parent), isAdminMode(false) {
    pages = new QStackedWidget(this);
    pages->addWidget(buildLandingPage());  // index 0
    pages->addWidget(buildLoginPage());    // index 1
    pages->addWidget(buildSignupPage());   // index 2

    QVBoxLayout* main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);
    main->addWidget(pages);

    pages->setCurrentIndex(0);  // start on landing
    applyStyles();
}

// ─── LANDING PAGE ────────────────────────────────────────────────────────────
QWidget* LoginWindow::buildLandingPage() {
    landingPage = new QWidget;
    QVBoxLayout* outer = new QVBoxLayout(landingPage);
    outer->setAlignment(Qt::AlignCenter);

    QFrame* card = new QFrame;
    card->setObjectName("card");
    card->setFixedWidth(460);

    QVBoxLayout* lay = new QVBoxLayout(card);
    lay->setSpacing(20);
    lay->setContentsMargins(48, 48, 48, 48);

    QLabel* logo = makeLogo(64);

    QLabel* title = new QLabel("AuctionHub");
    title->setObjectName("title");
    title->setAlignment(Qt::AlignCenter);

    QLabel* sub = new QLabel("Who are you signing in as?");
    sub->setObjectName("subtitle");
    sub->setAlignment(Qt::AlignCenter);

    // User button
    QPushButton* userBtn = new QPushButton("👤   User Login");
    userBtn->setObjectName("primaryBtn");
    userBtn->setCursor(Qt::PointingHandCursor);
    userBtn->setFixedHeight(52);

    QLabel* userHint = new QLabel("For buyers and sellers");
    userHint->setObjectName("hint");
    userHint->setAlignment(Qt::AlignCenter);

    // Divider
    QFrame* divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setObjectName("divider");

    // Admin button
    QPushButton* adminBtn = new QPushButton("🔐   Admin Login");
    adminBtn->setObjectName("adminBtn");
    adminBtn->setCursor(Qt::PointingHandCursor);
    adminBtn->setFixedHeight(52);

    QLabel* adminHint = new QLabel("For administrators only");
    adminHint->setObjectName("hint");
    adminHint->setAlignment(Qt::AlignCenter);

    lay->addWidget(logo, 0, Qt::AlignCenter);
    lay->addWidget(title);
    lay->addWidget(sub);
    lay->addSpacing(12);
    lay->addWidget(userBtn);
    lay->addWidget(userHint);
    lay->addSpacing(4);
    lay->addWidget(divider);
    lay->addSpacing(4);
    lay->addWidget(adminBtn);
    lay->addWidget(adminHint);

    outer->addWidget(card, 0, Qt::AlignCenter);

    connect(userBtn, &QPushButton::clicked, this, &LoginWindow::goToUserLogin);
    connect(adminBtn, &QPushButton::clicked, this, &LoginWindow::goToAdminLogin);

    return landingPage;
}

// ─── LOGIN PAGE ──────────────────────────────────────────────────────────────
QWidget* LoginWindow::buildLoginPage() {
    loginPage = new QWidget;
    QVBoxLayout* outer = new QVBoxLayout(loginPage);
    outer->setAlignment(Qt::AlignCenter);

    QFrame* card = new QFrame;
    card->setObjectName("card");
    card->setFixedWidth(420);

    QVBoxLayout* lay = new QVBoxLayout(card);
    lay->setSpacing(18);
    lay->setContentsMargins(40, 40, 40, 40);

    QLabel* logo = makeLogo(56);

    QLabel* titleLbl = new QLabel("AuctionHub");
    titleLbl->setObjectName("title");
    titleLbl->setAlignment(Qt::AlignCenter);

    // Dynamic mode label — updated by goToUserLogin / goToAdminLogin
    loginModeLabel = new QLabel("Sign in to your account");
    loginModeLabel->setObjectName("subtitle");
    loginModeLabel->setAlignment(Qt::AlignCenter);

    loginUsername = new QLineEdit;
    loginUsername->setPlaceholderText("Username");
    loginUsername->setObjectName("field");

    loginPassword = new QLineEdit;
    loginPassword->setPlaceholderText("Password");
    loginPassword->setEchoMode(QLineEdit::Password);
    loginPassword->setObjectName("field");

    loginBtn = new QPushButton("Sign In");
    loginBtn->setObjectName("primaryBtn");
    loginBtn->setCursor(Qt::PointingHandCursor);

    loginError = new QLabel("");
    loginError->setObjectName("error");
    loginError->setAlignment(Qt::AlignCenter);
    loginError->hide();

    // Bottom row: signup link (hidden in admin mode) + back button
    QHBoxLayout* swRow = new QHBoxLayout;
    QLabel* swLabel = new QLabel("Don't have an account?");
    swLabel->setObjectName("signupHintLabel");
    signupHintLabel = swLabel;
    swLabel->setObjectName("muted");
    goSignupBtn = new QPushButton("Sign Up");
    goSignupBtn->setObjectName("linkBtn");
    goSignupBtn->setCursor(Qt::PointingHandCursor);
    swRow->addStretch();
    swRow->addWidget(swLabel);
    swRow->addWidget(goSignupBtn);
    swRow->addStretch();

    backToLandingBtn = new QPushButton("← Back");
    backToLandingBtn->setObjectName("linkBtn");
    backToLandingBtn->setCursor(Qt::PointingHandCursor);
    backToLandingBtn->setToolTip("Return to role selection");

    lay->addWidget(logo, 0, Qt::AlignCenter);
    lay->addWidget(titleLbl);
    lay->addWidget(loginModeLabel);
    lay->addSpacing(8);
    lay->addWidget(loginUsername);
    lay->addWidget(loginPassword);
    lay->addWidget(loginError);
    lay->addWidget(loginBtn);
    lay->addLayout(swRow);
    lay->addWidget(backToLandingBtn, 0, Qt::AlignLeft);

    outer->addWidget(card, 0, Qt::AlignCenter);

    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::handleLogin);
    connect(goSignupBtn, &QPushButton::clicked, this, &LoginWindow::switchToSignup);
    connect(backToLandingBtn, &QPushButton::clicked, this, &LoginWindow::goToLanding);
    connect(loginPassword, &QLineEdit::returnPressed, this, &LoginWindow::handleLogin);

    return loginPage;
}

// ─── SIGNUP PAGE ─────────────────────────────────────────────────────────────
QWidget* LoginWindow::buildSignupPage() {
    signupPage = new QWidget;
    QVBoxLayout* outer = new QVBoxLayout(signupPage);
    outer->setAlignment(Qt::AlignCenter);

    QFrame* card = new QFrame;
    card->setObjectName("card");
    card->setFixedWidth(420);

    QVBoxLayout* lay = new QVBoxLayout(card);
    lay->setSpacing(18);
    lay->setContentsMargins(40, 40, 40, 40);

    QLabel* logo = makeLogo(56);

    QLabel* title = new QLabel("Create Account");
    title->setObjectName("title");
    title->setAlignment(Qt::AlignCenter);

    QLabel* sub = new QLabel("Join AuctionHub today");
    sub->setObjectName("subtitle");
    sub->setAlignment(Qt::AlignCenter);

    signupUsername = new QLineEdit;
    signupUsername->setPlaceholderText("Choose a username");
    signupUsername->setObjectName("field");

    signupPassword = new QLineEdit;
    signupPassword->setPlaceholderText("Choose a password");
    signupPassword->setEchoMode(QLineEdit::Password);
    signupPassword->setObjectName("field");

    roleCombo = new QComboBox;
    roleCombo->addItem("Buyer", "buyer");
    roleCombo->addItem("Seller", "seller");
    roleCombo->setObjectName("field");

    signupBtn = new QPushButton("Create Account");
    signupBtn->setObjectName("primaryBtn");
    signupBtn->setCursor(Qt::PointingHandCursor);

    signupError = new QLabel("");
    signupError->setObjectName("error");
    signupError->setAlignment(Qt::AlignCenter);
    signupError->hide();

    QHBoxLayout* swRow = new QHBoxLayout;
    QLabel* swLabel = new QLabel("Already have an account?");
    swLabel->setObjectName("muted");
    goLoginBtn = new QPushButton("Sign In");
    goLoginBtn->setObjectName("linkBtn");
    goLoginBtn->setCursor(Qt::PointingHandCursor);
    swRow->addStretch();
    swRow->addWidget(swLabel);
    swRow->addWidget(goLoginBtn);
    swRow->addStretch();

    lay->addWidget(logo, 0, Qt::AlignCenter);
    lay->addWidget(title);
    lay->addWidget(sub);
    lay->addSpacing(8);
    lay->addWidget(signupUsername);
    lay->addWidget(signupPassword);
    lay->addWidget(roleCombo);
    lay->addWidget(signupError);
    lay->addWidget(signupBtn);
    lay->addLayout(swRow);

    outer->addWidget(card, 0, Qt::AlignCenter);

    connect(signupBtn, &QPushButton::clicked, this, &LoginWindow::handleSignup);
    connect(goLoginBtn, &QPushButton::clicked, this, &LoginWindow::switchToLogin);

    return signupPage;
}

// ─── SLOTS ───────────────────────────────────────────────────────────────────
void LoginWindow::goToUserLogin() {
    isAdminMode = false;
    loginModeLabel->setText("Sign in as Buyer / Seller");
    goSignupBtn->setVisible(true);
    signupHintLabel->setVisible(true);
    loginError->hide();
    loginUsername->clear();
    loginPassword->clear();
    pages->setCurrentIndex(1);
}

void LoginWindow::goToAdminLogin() {
    isAdminMode = true;
    loginModeLabel->setText("Sign in as Administrator");
    goSignupBtn->setVisible(false);
    signupHintLabel->setVisible(false);
    loginError->hide();
    loginUsername->clear();
    loginPassword->clear();
    pages->setCurrentIndex(1);
}

void LoginWindow::goToLanding() {
    loginError->hide();
    pages->setCurrentIndex(0);
}

void LoginWindow::handleLogin() {
    QString u = loginUsername->text().trimmed();
    QString p = loginPassword->text();

    if (u.isEmpty() || p.isEmpty()) {
        loginError->setText("Please fill in all fields.");
        loginError->show();
        return;
    }

    User user = AuthService::login(u.toStdString(), p.toStdString());

    if (user.getUsername().empty()) {
        loginError->setText("Invalid username or password.");
        loginError->show();
        return;
    }

    // Enforce role match with the button the user clicked
    if (isAdminMode && user.getRole() != "admin") {
        loginError->setText("This account is not an admin account.");
        loginError->show();
        return;
    }
    if (!isAdminMode && user.getRole() == "admin") {
        loginError->setText("Please use Admin Login for admin accounts.");
        loginError->show();
        return;
    }

    loginError->hide();
    loginUsername->clear();
    loginPassword->clear();
    emit loginSuccess(user);
}

void LoginWindow::handleSignup() {
    QString u = signupUsername->text().trimmed();
    QString p = signupPassword->text();
    QString r = roleCombo->currentData().toString();

    if (u.isEmpty() || p.isEmpty()) {
        signupError->setText("Please fill in all fields.");
        signupError->show();
        return;
    }

    User existing = DBManager::getInstance().getUser(u.toStdString().c_str());
    if (!existing.getUsername().empty()) {
        signupError->setText("Username already taken.");
        signupError->show();
        return;
    }

    bool ok = AuthService::signup(u.toStdString(), p.toStdString(), r.toStdString());
    if (!ok) {
        signupError->setText("Signup failed. Try again.");
        signupError->show();
        return;
    }

    signupError->hide();
    signupUsername->clear();
    signupPassword->clear();
    QMessageBox::information(this, "Success", "Account created! Please sign in.");
    switchToLogin();
}

void LoginWindow::switchToSignup() { pages->setCurrentIndex(2); }
void LoginWindow::switchToLogin() { pages->setCurrentIndex(1); }

// ─── STYLES ──────────────────────────────────────────────────────────────────
void LoginWindow::applyStyles() {
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

    QFrame#card {
        background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
            stop:0 #111828, stop:1 #0A0E18);
        border: 1px solid #252D40;
        border-top: 1px solid #3A4560;
        border-radius: 18px;
    }
    QLabel#title {
        font-size: 28px;
        font-weight: 700;
        color: #F0E8D0;
        letter-spacing: 1px;
        background: transparent;
    }
    QLabel#subtitle {
        font-size: 13px;
        color: #5A6480;
        background: transparent;
        letter-spacing: 0.3px;
    }
    QLabel#hint {
        font-size: 12px;
        color: #3A4050;
        background: transparent;
    }
    QLabel#muted {
        font-size: 13px;
        color: #4A5268;
        background: transparent;
    }
    QLabel#error {
        font-size: 13px;
        color: #E06050;
        background: transparent;
        font-weight: 600;
    }
    QFrame#divider {
        color: #1E2535;
        background-color: #1E2535;
        max-height: 1px;
    }

    )");
}