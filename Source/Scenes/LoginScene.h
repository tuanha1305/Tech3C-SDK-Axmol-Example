// LoginScene.h
#pragma once

#include "axmol.h"
#include "ui/UIButton.h"
#include "ui/UIWidget.h"
#include "Tech3C/Tech3CManager.h"

USING_NS_AX;

class LoginScene : public Scene {
public:
    static Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(LoginScene);

protected:
    void onEnter() override;
    void onExit() override;

private:
    // UI Setup
    void setupUI();
    void setupButtons();
    void setupLabels();
    void setupBackground();

    // Button Events
    void onLoginButtonClicked(Object* sender);
    void onLogoutButtonClicked(Object* sender);

    // Tech3C Callbacks
    void setupTech3CCallbacks();
    void onLoginSuccess(const tech3c::UserInfo& userInfo);
    void onRegisterSuccess(const tech3c::UserInfo& userInfo);
    void onLoginError(const tech3c::ErrorInfo& error);
    void onLoginCancelled();
    void onAuthScreenOpened();

    // UI Updates
    void updateUIForLoginState(bool isLoggedIn);
    void updateUserInfo(const tech3c::UserInfo& userInfo);
    void updateStatusMessage(const std::string& message, const Color3B& color = Color3B::WHITE);
    void showErrorDialog(const std::string& title, const std::string& message);

    // Utility
    void initializeTech3C();
    std::string formatUserInfo(const tech3c::UserInfo& userInfo);
    std::string truncateString(const std::string& str, size_t maxLength);

    // UI Elements
    ui::Button* m_loginButton;
    ui::Button* m_logoutButton;
    ui::Button* m_changePasswordButton;

    Label* m_titleLabel;
    Label* m_statusLabel;
    Label* m_userInfoLabel;

    LayerColor* m_backgroundLayer;

    // Data
    bool m_isLoggedIn;

    // Constants
    static constexpr float BUTTON_WIDTH = 200.0f;
    static constexpr float BUTTON_HEIGHT = 50.0f;
    static constexpr float BUTTON_SPACING = 60.0f;
    static constexpr float LABEL_FONT_SIZE = 20.0f;
    static constexpr float TITLE_FONT_SIZE = 28.0f;
};
