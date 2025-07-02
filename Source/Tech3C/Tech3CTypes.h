#pragma once

#include <string>
#include <functional>

namespace tech3c {

    // Login Types
    enum class LoginType {
        GUEST = 0,
        ACCOUNT = 1,
        SOCIAL = 2,
    };

    // UI Modes
    enum class UiMode {
        DIALOG = 0,
        FULLSCREEN = 1
    };

    // Languages
    enum class Language {
        ENGLISH = 0,
        VIETNAMESE = 1,
        CHINESE = 2,
        KHMER = 3,
        LAO = 4,
        THAI = 5
    };

    // Orientation Modes
    enum class OrientationMode {
        AUTO = 0,
        PORTRAIT = 1,
        LANDSCAPE = 2
    };

    // User Information
    struct UserInfo {
        std::string userId;
        std::string accessToken;
        std::string refreshToken;
        LoginType loginType;
        long expiryTime;

        UserInfo() : loginType(LoginType::GUEST), expiryTime(0) {}

        bool isValid() const {
            return !userId.empty() && !accessToken.empty();
        }

        void clear() {
            userId.clear();
            accessToken.clear();
            refreshToken.clear();
            loginType = LoginType::GUEST;
            expiryTime = 0;
        }
    };

    // Error Information
    struct ErrorInfo {
        int code;
        std::string message;
        std::string details;

        ErrorInfo() : code(0) {}
        ErrorInfo(int c, const std::string& msg, const std::string& det = "")
            : code(c), message(msg), details(det) {}
    };

    // Callback Types
    using LoginSuccessCallback = std::function<void(const UserInfo& userInfo)>;
    using RegisterSuccessCallback = std::function<void(const UserInfo& userInfo)>;
    using ErrorCallback = std::function<void(const ErrorInfo& error)>;
    using CancelCallback = std::function<void()>;
    using AuthScreenOpenedCallback = std::function<void()>;

    // Configuration
    struct Config {
        std::string clientId;
        std::string clientSecret;
        bool debugMode;
        UiMode uiMode;
        Language language;
        OrientationMode orientation;
        bool enableGuestLogin;
        bool disableExitLogin;
        bool requireOtp;
        bool enableMaintenanceCheck;

        Config()
            : debugMode(false)
            , uiMode(UiMode::DIALOG)
            , language(Language::ENGLISH)
            , orientation(OrientationMode::AUTO)
            , enableGuestLogin(true)
            , disableExitLogin(false)
            , requireOtp(false)
            , enableMaintenanceCheck(true) {}
    };

    // Helper functions
    std::string loginTypeToString(LoginType type);
    LoginType stringToLoginType(const std::string& str);
    std::string languageToString(Language lang);
    std::string uiModeToString(UiMode mode);
    std::string orientationModeToString(OrientationMode mode);
}
