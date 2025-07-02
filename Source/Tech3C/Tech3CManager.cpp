// Tech3CManager.cpp
#include "Tech3CManager.h"
#include "platform/PlatformConfig.h"

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#include <android/log.h>
#endif

using namespace tech3c;

// Static members
Tech3CManager* Tech3CManager::s_instance = nullptr;
std::mutex Tech3CManager::s_instanceMutex;

static const char* JAVA_CLASS_NAME = "dev/axmol/lib/Tech3CHelper";

namespace tech3c {

    std::string loginTypeToString(LoginType type) {
        switch (type) {
            case LoginType::GUEST: return "guest";
            case LoginType::ACCOUNT: return "account";
            case LoginType::SOCIAL: return "social";
            default: return "guest";
        }
    }

    LoginType stringToLoginType(const std::string& str) {
        if (str == "guest") return LoginType::GUEST;
        if (str == "account") return LoginType::ACCOUNT;
        if (str == "social") return LoginType::SOCIAL;
        return LoginType::GUEST;
    }

    std::string languageToString(Language lang) {
        switch (lang) {
            case Language::ENGLISH: return "English";
            case Language::VIETNAMESE: return "Vietnamese";
            case Language::CHINESE: return "Chinese";
            case Language::KHMER: return "Khmer";
            case Language::LAO: return "Lao";
            case Language::THAI: return "Thai";
            default: return "English";
        }
    }

    std::string uiModeToString(UiMode mode) {
        switch (mode) {
            case UiMode::DIALOG: return "Dialog";
            case UiMode::FULLSCREEN: return "Fullscreen";
            default: return "Dialog";
        }
    }

    std::string orientationModeToString(OrientationMode mode) {
        switch (mode) {
            case OrientationMode::AUTO: return "Auto";
            case OrientationMode::PORTRAIT: return "Portrait";
            case OrientationMode::LANDSCAPE: return "Landscape";
            default: return "Auto";
        }
    }
}

//========================================================================
// Tech3CManager implementation

Tech3CManager::Tech3CManager()
        : m_isInitialized(false) {
    logDebug("Tech3CManager created");
}

Tech3CManager::~Tech3CManager() {
    cleanup();
    logDebug("Tech3CManager destroyed");
}

Tech3CManager* Tech3CManager::getInstance() {
    std::lock_guard<std::mutex> lock(s_instanceMutex);
    if (!s_instance) {
        s_instance = new Tech3CManager();
    }
    return s_instance;
}

void Tech3CManager::destroyInstance() {
    std::lock_guard<std::mutex> lock(s_instanceMutex);
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

bool Tech3CManager::initialize(const std::string& clientId, const std::string& clientSecret) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_isInitialized) {
        logDebug("Tech3CManager already initialized");
        return true;
    }

    if (clientId.empty() || clientSecret.empty()) {
        logError("Client ID or Client Secret is empty");
        return false;
    }

    m_config.clientId = clientId;
    m_config.clientSecret = clientSecret;

    logDebug("Initializing Tech3C SDK with clientId: " + clientId);

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "initialize", "(Ljava/lang/String;Ljava/lang/String;)V")) {

        jstring jClientId = methodInfo.env->NewStringUTF(clientId.c_str());
        jstring jClientSecret = methodInfo.env->NewStringUTF(clientSecret.c_str());

        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, jClientId, jClientSecret);

        // Check for Java exceptions
        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred during initialization");

            methodInfo.env->DeleteLocalRef(jClientId);
            methodInfo.env->DeleteLocalRef(jClientSecret);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            return false;
        }

        methodInfo.env->DeleteLocalRef(jClientId);
        methodInfo.env->DeleteLocalRef(jClientSecret);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);

        m_isInitialized = true;
        logDebug("Tech3C SDK initialized successfully");
        return true;
    } else {
        logError("Failed to find initialize method in Java class");
        return false;
    }
#else
    // For other platforms, just mark as initialized for testing
    m_isInitialized = true;
    logDebug("Tech3C SDK initialized (non-Android platform)");
    return true;
#endif
}

void Tech3CManager::cleanup() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_isInitialized) {
        return;
    }

    // Clear user data
    m_currentUser.clear();

    // Clear callbacks
    m_loginSuccessCallback = nullptr;
    m_registerSuccessCallback = nullptr;
    m_errorCallback = nullptr;
    m_cancelCallback = nullptr;
    m_authScreenOpenedCallback = nullptr;

    m_isInitialized = false;
    logDebug("Tech3CManager cleaned up");
}

void Tech3CManager::setDebugMode(bool debug) {
    m_config.debugMode = debug;
    logDebug("Debug mode set to: " + std::string(debug ? "true" : "false"));

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "setDebugMode", "(Z)V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, debug);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while setting debug mode");

            if (m_errorCallback) {
                ErrorInfo error(1010, "Failed to set debug mode");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find setDebugMode method");
        if (m_errorCallback) {
            ErrorInfo error(1011, "Failed to find setDebugMode method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::setUiMode(UiMode mode) {
    m_config.uiMode = mode;
    logDebug("UI mode set to: " + uiModeToString(mode));

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "setUiMode", "(I)V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, (int)mode);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while setting UI mode");

            if (m_errorCallback) {
                ErrorInfo error(1008, "Failed to set UI mode");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find setUiMode method");
        if (m_errorCallback) {
            ErrorInfo error(1009, "Failed to find setUiMode method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::setLanguage(Language language) {
    m_config.language = language;
    logDebug("Language set to: " + languageToString(language));

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "setLanguage", "(I)V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, (int)language);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while setting language");

            if (m_errorCallback) {
                ErrorInfo error(1006, "Failed to set language");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find setLanguage method");
        if (m_errorCallback) {
            ErrorInfo error(1007, "Failed to find setLanguage method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::setOrientation(OrientationMode orientation) {
    m_config.orientation = orientation;
    logDebug("Orientation set to: " + orientationModeToString(orientation));

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "setOrientation", "(I)V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, (int)orientation);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while setting orientation");

            if (m_errorCallback) {
                ErrorInfo error(1004, "Failed to set orientation");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find setOrientation method");
        if (m_errorCallback) {
            ErrorInfo error(1005, "Failed to find setOrientation method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif

}

void Tech3CManager::setIpMaintenanceCheck(const std::string& ip) {
    logDebug("Setting IP maintenance check to: " + (ip.empty() ? "empty" : ip));

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "setIpMaintenanceCheck", "(Ljava/lang/String;)V")) {
        jstring jIp = methodInfo.env->NewStringUTF(ip.c_str());
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, jIp);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while setting IP maintenance check");

            if (m_errorCallback) {
                ErrorInfo error(1022, "Failed to set IP maintenance check");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(jIp);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find setIpMaintenanceCheck method");
        if (m_errorCallback) {
            ErrorInfo error(1023, "Failed to find setIpMaintenanceCheck method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::setEnableGuestLogin(bool enable) {
    m_config.enableGuestLogin = enable;
    logDebug("Guest login enabled: " + std::string(enable ? "true" : "false"));

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "setEnableGuestLogin", "(Z)V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, enable);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while setting guest login");

            if (m_errorCallback) {
                ErrorInfo error(1012, "Failed to set guest login");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find setEnableGuestLogin method");
        if (m_errorCallback) {
            ErrorInfo error(1013, "Failed to find setEnableGuestLogin method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::setDisableExitLogin(bool disable) {
    m_config.disableExitLogin = disable;
    logDebug("Exit login disabled: " + std::string(disable ? "true" : "false"));

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "setDisableExitLogin", "(Z)V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, disable);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while setting disable exit login");

            if (m_errorCallback) {
                ErrorInfo error(1014, "Failed to set disable exit login");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find setDisableExitLogin method");
        if (m_errorCallback) {
            ErrorInfo error(1015, "Failed to find setDisableExitLogin method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::setRequireOtp(bool require) {
    m_config.requireOtp = require;
    logDebug("Require OTP: " + std::string(require ? "true" : "false"));

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "setRequireOtp", "(Z)V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, require);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while setting require OTP");

            if (m_errorCallback) {
                ErrorInfo error(1016, "Failed to set require OTP");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find setRequireOtp method");
        if (m_errorCallback) {
            ErrorInfo error(1017, "Failed to find setRequireOtp method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::setEnableMaintenanceCheck(bool enable) {
    m_config.enableMaintenanceCheck = enable;
    logDebug("Maintenance check enabled: " + std::string(enable ? "true" : "false"));

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "setEnableMaintenanceCheck", "(Z)V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, enable);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while setting maintenance check");

            if (m_errorCallback) {
                ErrorInfo error(1018, "Failed to set maintenance check");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find setEnableMaintenanceCheck method");
        if (m_errorCallback) {
            ErrorInfo error(1019, "Failed to find setEnableMaintenanceCheck method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::showAuth() {
    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

    logDebug("Showing authentication screen");

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "showAuth", "()V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while showing auth");

            if (m_errorCallback) {
                ErrorInfo error(1002, "Failed to show authentication screen");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find showAuth method");
        if (m_errorCallback) {
            ErrorInfo error(1003, "Failed to find showAuth method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::logout() {
    std::lock_guard<std::mutex> lock(m_mutex);

    logDebug("User logged out");
    m_currentUser.clear();

    if (!m_isInitialized) {
        logError("SDK not initialized");
        if (m_errorCallback) {
            ErrorInfo error(1001, "SDK not initialized");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
        return;
    }

#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, JAVA_CLASS_NAME, "logout", "()V")) {
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID);

        if (methodInfo.env->ExceptionCheck()) {
            methodInfo.env->ExceptionDescribe();
            methodInfo.env->ExceptionClear();
            logError("Java exception occurred while logging out");

            if (m_errorCallback) {
                ErrorInfo error(1020, "Failed to logout");
                executeOnMainThread([this, error]() { m_errorCallback(error); });
            }
        }

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    } else {
        logError("Failed to find logout method");
        if (m_errorCallback) {
            ErrorInfo error(1021, "Failed to find logout method");
            executeOnMainThread([this, error]() { m_errorCallback(error); });
        }
    }
#endif
}

void Tech3CManager::onLoginSuccess(const std::string& userId, const std::string& accessToken,
                                   const std::string& refreshToken, int loginType, long expiryTime) {
    std::lock_guard<std::mutex> lock(m_mutex);

    logDebug("Login success for user: " + userId);

    m_currentUser.userId = userId;
    m_currentUser.accessToken = accessToken;
    m_currentUser.refreshToken = refreshToken;
    m_currentUser.loginType = static_cast<LoginType>(loginType);
    m_currentUser.expiryTime = expiryTime;

    if (m_loginSuccessCallback) {
        UserInfo userCopy = m_currentUser;
        executeOnMainThread([this, userCopy]() { m_loginSuccessCallback(userCopy); });
    }
}

void Tech3CManager::onRegisterSuccess(const std::string& userId, const std::string& accessToken,
                                      const std::string& refreshToken, long expiryTime) {
    std::lock_guard<std::mutex> lock(m_mutex);

    logDebug("Register success for user: " + userId);

    m_currentUser.userId = userId;
    m_currentUser.accessToken = accessToken;
    m_currentUser.refreshToken = refreshToken;
    m_currentUser.loginType = LoginType::ACCOUNT; // Default for registration
    m_currentUser.expiryTime = expiryTime;

    if (m_registerSuccessCallback) {
        UserInfo userCopy = m_currentUser;
        executeOnMainThread([this, userCopy]() { m_registerSuccessCallback(userCopy); });
    }
}

void Tech3CManager::onError(const std::string& error) {
    logError("Auth error: " + error);

    if (m_errorCallback) {
        ErrorInfo errorInfo(2000, error);
        executeOnMainThread([this, errorInfo]() { m_errorCallback(errorInfo); });
    }
}

void Tech3CManager::onAuthCancelled() {
    logDebug("Auth cancelled by user");

    if (m_cancelCallback) {
        executeOnMainThread([this]() { m_cancelCallback(); });
    }
}

void Tech3CManager::onAuthScreenOpened() {
    logDebug("Auth screen opened");

    if (m_authScreenOpenedCallback) {
        executeOnMainThread([this]() { m_authScreenOpenedCallback(); });
    }
}

void Tech3CManager::logDebug(const std::string& message) {
    if (m_config.debugMode) {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
        __android_log_print(ANDROID_LOG_DEBUG, "Tech3CManager", "%s", message.c_str());
#endif
        AXLOG("[Tech3C] %s", message.c_str());
    }
}

void Tech3CManager::logError(const std::string& message) {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    __android_log_print(ANDROID_LOG_ERROR, "Tech3CManager", "%s", message.c_str());
#endif
    AXLOGWARN("[Tech3C ERROR] %s", message.c_str());
}

void Tech3CManager::executeOnMainThread(std::function<void()> callback) {
    // Execute callback on main thread
    Director::getInstance()->getScheduler()->performFunctionInCocosThread([callback]() {
        callback();
    });
}

//========================================================================
// JNI Callback Functions
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID

extern "C" {
    JNIEXPORT void JNICALL Java_dev_axmol_lib_Tech3CHelper_nativeOnLoginSuccess
            (JNIEnv *env, jclass clazz, jstring userId, jstring accessToken, jstring refreshToken, jint loginType, jlong expiryTime) {

        std::string userIdStr = JniHelper::jstring2string(userId);
        std::string accessTokenStr = JniHelper::jstring2string(accessToken);
        std::string refreshTokenStr = JniHelper::jstring2string(refreshToken);

        Tech3CManager::getInstance()->onLoginSuccess(userIdStr, accessTokenStr, refreshTokenStr, loginType, expiryTime);
    }

    JNIEXPORT void JNICALL Java_dev_axmol_lib_Tech3CHelper_nativeOnRegisterSuccess
            (JNIEnv *env, jclass clazz, jstring userId, jstring accessToken, jstring refreshToken, jlong expiryTime) {

        std::string userIdStr = JniHelper::jstring2string(userId);
        std::string accessTokenStr = JniHelper::jstring2string(accessToken);
        std::string refreshTokenStr = JniHelper::jstring2string(refreshToken);

        Tech3CManager::getInstance()->onRegisterSuccess(userIdStr, accessTokenStr, refreshTokenStr, expiryTime);
    }

    JNIEXPORT void JNICALL Java_dev_axmol_lib_Tech3CHelper_nativeOnError
            (JNIEnv *env, jclass clazz, jstring error) {

        std::string errorStr = JniHelper::jstring2string(error);
        Tech3CManager::getInstance()->onError(errorStr);
    }

    JNIEXPORT void JNICALL Java_dev_axmol_lib_Tech3CHelper_nativeOnAuthCancelled
            (JNIEnv *env, jclass clazz) {

        Tech3CManager::getInstance()->onAuthCancelled();
    }

    JNIEXPORT void JNICALL Java_dev_axmol_lib_Tech3CHelper_nativeOnAuthScreenOpened
            (JNIEnv *env, jclass clazz) {

        Tech3CManager::getInstance()->onAuthScreenOpened();
    }
}

#endif
