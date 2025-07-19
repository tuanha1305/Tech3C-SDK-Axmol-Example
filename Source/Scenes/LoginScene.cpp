// LoginScene.cpp
#include "LoginScene.h"

USING_NS_AX;

Scene* LoginScene::createScene() {
    return LoginScene::create();
}

bool LoginScene::init() {
    if (!Scene::init()) {
        return false;
    }

    m_isLoggedIn = false;

    setupBackground();
    setupUI();
    setupTech3CCallbacks();
    initializeTech3C();

    return true;
}

void LoginScene::onEnter() {
    Scene::onEnter();

    // Check if already logged in
    auto manager = tech3c::Tech3CManager::getInstance();
    if (manager->isLoggedIn()) {
        updateUserInfo(manager->getCurrentUser());
        updateUIForLoginState(true);
    }
}

void LoginScene::onExit() {
    Scene::onExit();
}

void LoginScene::setupBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // Create gradient background
    m_backgroundLayer = LayerGradient::create(
            Color4B(30, 40, 60, 255),   // Top color (dark blue)
            Color4B(60, 80, 120, 255)   // Bottom color (lighter blue)
    );
    this->addChild(m_backgroundLayer, -1);
}

void LoginScene::setupUI() {
    setupLabels();
    setupButtons();
}

void LoginScene::setupLabels() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Title Label
    m_titleLabel = Label::createWithTTF("Tech3C Login Demo", "fonts/Marker Felt.ttf", TITLE_FONT_SIZE);
    if (m_titleLabel) {
        m_titleLabel->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height - 80));
        m_titleLabel->setColor(Color3B::WHITE);
        this->addChild(m_titleLabel, 1);
    }

    // Status Label
    m_statusLabel = Label::createWithTTF("Ready to login", "fonts/Marker Felt.ttf", LABEL_FONT_SIZE);
    if (m_statusLabel) {
        m_statusLabel->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2 - 80));
        m_statusLabel->setColor(Color3B::YELLOW);
        this->addChild(m_statusLabel, 1);
    }

    // User Info Label
    m_userInfoLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 16);
    if (m_userInfoLabel) {
        m_userInfoLabel->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2 - 140));
        m_userInfoLabel->setColor(Color3B::GREEN);
        m_userInfoLabel->setDimensions(visibleSize.width - 40, 0);
        m_userInfoLabel->setLineBreakWithoutSpace(true);
        m_userInfoLabel->setHorizontalAlignment(TextHAlignment::CENTER);
        this->addChild(m_userInfoLabel, 1);
    }
}

void LoginScene::setupButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    float centerX = origin.x + visibleSize.width/2;
    float centerY = origin.y + visibleSize.height/2;

    // Login Button
    m_loginButton = ui::Button::create();
    if (m_loginButton) {
        m_loginButton->setTitleText("Login");
        m_loginButton->setTitleFontSize(24);
        m_loginButton->setTitleColor(Color3B::WHITE);

        // Set button appearance
        m_loginButton->setScale9Enabled(true);
        m_loginButton->setContentSize(Size(BUTTON_WIDTH, BUTTON_HEIGHT));
        m_loginButton->setColor(Color3B(70, 130, 180)); // Steel blue

        m_loginButton->setPosition(Vec2(centerX, centerY + BUTTON_SPACING));
        m_loginButton->addClickEventListener(AX_CALLBACK_1(LoginScene::onLoginButtonClicked, this));
        this->addChild(m_loginButton, 1);
    }

    // Logout Button
    m_logoutButton = ui::Button::create();
    if (m_logoutButton) {
        m_logoutButton->setTitleText("Logout");
        m_logoutButton->setTitleFontSize(24);
        m_logoutButton->setTitleColor(Color3B::WHITE);

        m_logoutButton->setScale9Enabled(true);
        m_logoutButton->setContentSize(Size(BUTTON_WIDTH, BUTTON_HEIGHT));
        m_logoutButton->setColor(Color3B(220, 20, 60)); // Crimson

        m_logoutButton->setPosition(Vec2(centerX, centerY + BUTTON_SPACING));
        m_logoutButton->addClickEventListener(AX_CALLBACK_1(LoginScene::onLogoutButtonClicked, this));
        m_logoutButton->setVisible(false);
        this->addChild(m_logoutButton, 1);
    }

}

void LoginScene::setupTech3CCallbacks() {
    auto manager = tech3c::Tech3CManager::getInstance();

    // Set callbacks
    manager->setLoginSuccessCallback([this](const tech3c::UserInfo& userInfo) {
        this->onLoginSuccess(userInfo);
    });

    manager->setRegisterSuccessCallback([this](const tech3c::UserInfo& userInfo) {
        this->onRegisterSuccess(userInfo);
    });

    manager->setErrorCallback([this](const tech3c::ErrorInfo& error) {
        this->onLoginError(error);
    });

    manager->setCancelCallback([this]() {
        this->onLoginCancelled();
    });

    manager->setAuthScreenOpenedCallback([this]() {
        this->onAuthScreenOpened();
    });
}

void LoginScene::initializeTech3C() {
    auto manager = tech3c::Tech3CManager::getInstance();

    // Initialize with your credentials
    bool success = manager->initialize("3cgame", "2NPDoFBSarmjgvK25OKSi9tEttaX1s4Y");

    if (success) {
        // Configure SDK
        manager->setDebugMode(true);
        manager->setOrientation(tech3c::OrientationMode::LANDSCAPE);
        manager->setUiMode(tech3c::UiMode::DIALOG);
        manager->setLanguage(tech3c::Language::VIETNAMESE);
        manager->setEnableGuestLogin(true);
        manager->setDisableExitLogin(false);
        manager->setRequireOtp(true);
        // manager->setEnableMaintenanceCheck(true);
        // manager->setIpMaintenanceCheck("103.51.120.202");

        updateStatusMessage("Tech3C SDK initialized successfully", Color3B::GREEN);
    } else {
        updateStatusMessage("Failed to initialize Tech3C SDK", Color3B::RED);
    }
}

void LoginScene::onLoginButtonClicked(Object* sender) {
    auto manager = tech3c::Tech3CManager::getInstance();

    if (!manager->isInitialized()) {
        showErrorDialog("Error", "SDK not initialized");
        return;
    }

    updateStatusMessage("Showing login screen...", Color3B::YELLOW);
    manager->setIpMaintenanceCheck("103.51.120.202");
    manager->showAuth();
}

void LoginScene::onLogoutButtonClicked(Object* sender) {
    auto manager = tech3c::Tech3CManager::getInstance();
    manager->logout();

    updateUIForLoginState(false);
    updateStatusMessage("Logged out successfully", Color3B::ORANGE);

    if (m_userInfoLabel) {
        m_userInfoLabel->setString("");
    }
}

void LoginScene::onLoginSuccess(const tech3c::UserInfo& userInfo) {
    AXLOGD("Login success: UserID=%s, LoginType=%s",
           userInfo.userId.c_str(),
           tech3c::loginTypeToString(userInfo.loginType).c_str());

    updateUserInfo(userInfo);
    updateUIForLoginState(true);
    updateStatusMessage("Login successful!", Color3B::GREEN);
}

void LoginScene::onRegisterSuccess(const tech3c::UserInfo& userInfo) {
    AXLOGD("Register success: UserID=%s", userInfo.userId.c_str());

    updateUserInfo(userInfo);
    updateUIForLoginState(true);
    updateStatusMessage("Registration successful!", Color3B::GREEN);
}

void LoginScene::onLoginError(const tech3c::ErrorInfo& error) {
    AXLOGE("Login error: Code=%d, Message=%s", error.code, error.message.c_str());

    std::string errorMsg = "Error " + std::to_string(error.code) + ": " + error.message;
    updateStatusMessage(errorMsg, Color3B::RED);

    showErrorDialog("Login Error", error.message);
}

void LoginScene::onLoginCancelled() {
    AXLOGD("Login cancelled by user");
    updateStatusMessage("Login cancelled", Color3B::ORANGE);
}

void LoginScene::onAuthScreenOpened() {
    AXLOGD("Auth screen opened");
    updateStatusMessage("Auth screen opened", Color3B::BLUE);
}

void LoginScene::updateUIForLoginState(bool isLoggedIn) {
    m_isLoggedIn = isLoggedIn;

    if (m_loginButton) {
        m_loginButton->setVisible(!isLoggedIn);
    }

    if (m_logoutButton) {
        m_logoutButton->setVisible(isLoggedIn);
    }

    if (m_changePasswordButton) {
        m_changePasswordButton->setVisible(isLoggedIn);
    }
}

void LoginScene::updateUserInfo(const tech3c::UserInfo& userInfo) {
    if (!m_userInfoLabel) return;

    std::string infoText = formatUserInfo(userInfo);
    m_userInfoLabel->setString(infoText);
}

void LoginScene::updateStatusMessage(const std::string& message, const Color3B& color) {
    if (!m_statusLabel) return;

    m_statusLabel->setString(message);
    m_statusLabel->setColor(color);

    AXLOGD("Status: %s", message.c_str());
}

void LoginScene::showErrorDialog(const std::string& title, const std::string& message) {
    // Simple error display using label
    updateStatusMessage("ERROR: " + message, Color3B::RED);

    // You can implement a proper dialog here if needed
    AXLOGE("%s: %s", title.c_str(), message.c_str());
}

std::string LoginScene::formatUserInfo(const tech3c::UserInfo& userInfo) {
    if (!userInfo.isValid()) {
        return "";
    }

    std::string info = "User Info:\n";
    info += "ID: " + userInfo.userId + "\n";
    info += "Type: " + tech3c::loginTypeToString(userInfo.loginType) + "\n";
    info += "Token: " + truncateString(userInfo.accessToken, 20) + "...\n";

    if (userInfo.expiryTime > 0) {
        info += "Expires: " + std::to_string(userInfo.expiryTime);
    }

    return info;
}

std::string LoginScene::truncateString(const std::string& str, size_t maxLength) {
    if (str.length() <= maxLength) {
        return str;
    }
    return str.substr(0, maxLength) + "...";
}
