#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include "User.h"
#include "DBManager.h"
#include "AuthService.h"

class LoginWindow : public QWidget {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget* parent = nullptr);

signals:
    void loginSuccess(User user);

private slots:
    void handleLogin();
    void handleSignup();
    void switchToSignup();
    void switchToLogin();
    void goToUserLogin();
    void goToAdminLogin();
    void goToLanding();

private:
    // Pages
    QWidget* landingPage;   // index 0 — role selection
    QWidget* loginPage;     // index 1 — login form
    QWidget* signupPage;    // index 2 — signup form

    // Landing page
    bool        isAdminMode;   // tracks which mode was selected

    // Login page
    QLabel* loginModeLabel;
    QLabel* signupHintLabel;
    QLineEdit* loginUsername;
    QLineEdit* loginPassword;
    QPushButton* loginBtn;
    QPushButton* goSignupBtn;
    QPushButton* backToLandingBtn;
    QLabel* loginError;

    // Signup page
    QLineEdit* signupUsername;
    QLineEdit* signupPassword;
    QComboBox* roleCombo;
    QPushButton* signupBtn;
    QPushButton* goLoginBtn;
    QLabel* signupError;

    QStackedWidget* pages;

    QWidget* buildLandingPage();
    QWidget* buildLoginPage();
    QWidget* buildSignupPage();
    void applyStyles();
};