#pragma once

#include "axmol.h"
#include "Tech3CTypes.h"
#include <memory>
#include <mutex>

USING_NS_AX;

namespace tech3c {

    class Tech3CManager {
    public:
        // Singleton
        static Tech3CManager* getInstance();
        static void destroyInstance();

        // Initialize SDK
        bool initialize(const std::string& clientId, const std::string& clientSecret);
        void cleanup();

        // Configuration
        void setDebugMode(bool debug);
        void setUiMode(UiMode mode);
        void setLanguage(Language language);
        void setOrientation(OrientationMode orientation);
        void setEnableGuestLogin(bool enable);
        void setDisableExitLogin(bool disable);
        void setRequireOtp(bool require);
        void setEnableMaintenanceCheck(bool enable);
        void setIpMaintenanceCheck(const std::string& ip);

        // Authentication
        void showAuth();
        void logout();

        // User Info
        const UserInfo& getCurrentUser() const { return m_currentUser; }
        bool isLoggedIn() const { return m_currentUser.isValid(); }

        // Callbacks
        void setLoginSuccessCallback(LoginSuccessCallback callback) { m_loginSuccessCallback = callback; }
        void setRegisterSuccessCallback(RegisterSuccessCallback callback) { m_registerSuccessCallback = callback; }
        void setErrorCallback(ErrorCallback callback) { m_errorCallback = callback; }
        void setCancelCallback(CancelCallback callback) { m_cancelCallback = callback; }
        void setAuthScreenOpenedCallback(AuthScreenOpenedCallback callback) { m_authScreenOpenedCallback = callback; }

        // JNI Callbacks (called from Java)
        void onLoginSuccess(const std::string& userId, const std::string& accessToken,
                          const std::string& refreshToken, int loginType, long expiryTime);
        void onRegisterSuccess(const std::string& userId, const std::string& accessToken,
                             const std::string& refreshToken, long expiryTime);
        void onError(const std::string& error);
        void onAuthCancelled();
        void onAuthScreenOpened();

        // Utility
        bool isInitialized() const { return m_isInitialized; }
        const Config& getConfig() const { return m_config; }

    private:
        Tech3CManager();
        ~Tech3CManager();

        // Prevent copy
        Tech3CManager(const Tech3CManager&) = delete;
        Tech3CManager& operator=(const Tech3CManager&) = delete;

        // Internal methods
        void callJavaMethod(const std::string& methodName, const std::string& signature, ...);
        void logDebug(const std::string& message);
        void logError(const std::string& message);

        // Thread-safe callback execution
        void executeOnMainThread(std::function<void()> callback);

        static Tech3CManager* s_instance;
        static std::mutex s_instanceMutex;

        // Configuration
        Config m_config;
        bool m_isInitialized;

        // User data
        UserInfo m_currentUser;

        // Callbacks
        LoginSuccessCallback m_loginSuccessCallback;
        RegisterSuccessCallback m_registerSuccessCallback;
        ErrorCallback m_errorCallback;
        CancelCallback m_cancelCallback;
        AuthScreenOpenedCallback m_authScreenOpenedCallback;

        // Thread safety
        mutable std::mutex m_mutex;
    };
}
