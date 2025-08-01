# Hướng dẫn tích hợp Tech3C SDK cho Axmol

## 1. Android

### 1. Cấu hình

#### 1.1 Chỉnh `minSdkVersion`
Hiện tại Tech3C SDK chỉ hỗ trợ `minSdkVersion` 21 trở lên. Vào `proj.android/app/build.gradle` và tìm dòng `minSdkVersion` và thay đổi giá trị thành 21 trở lên.

```gradle
android {
    defaultConfig {
        minSdkVersion 21  // Thay đổi từ 16 thành 21
        targetSdkVersion 33
    }
}
```

#### 1.2 Thêm `Tech3C Maven Repository`
Vào `proj.android/build.gradle`

```gradle
allprojects {
    repositories {
        google()
        mavenCentral()
        maven { url 'https://dl.3cgame.vn/repository/android/' }
    }
}
```

#### 1.3 Thêm thư viện Tech3C SDK Auth
Vào `proj.android/app/build.gradle`

```gradle
dependencies {
    implementation project(':libaxmol')
    
    // Thêm Tech3C SDK
    implementation "vn.tech3c.sdk:auth:1.0.0"
}
```

### 2. Tích hợp Code

#### 2.1 Tạo cấu trúc thư mục
```
Classes/
├── Tech3C/
│   ├── Tech3CManager.h
│   ├── Tech3CManager.cpp
│   └── Tech3CTypes.h
├── Scenes/
│   ├── LoginScene.h
│   └── LoginScene.cpp
└── AppDelegate.cpp
```

#### 2.2 Tạo Java Bridge
Tạo file `proj.android/app/src/main/java/dev/axmol/lib/Tech3CHelper.java`:

```java
package dev.axmol.lib;

import android.app.Activity;
import android.content.Context;
import android.util.Log;

import vn.tech3c.sdk.auth.callback.OnAuthCallback;
import vn.tech3c.sdk.auth.controller.Tech3CIdController;
import vn.tech3c.sdk.auth.entities.enums.Language;
import vn.tech3c.sdk.auth.entities.enums.LoginType;
import vn.tech3c.sdk.auth.entities.enums.UiMode;
import vn.tech3c.sdk.auth.entities.enums.OrientationMode;
import vn.tech3c.sdk.auth.exceptions.Tech3CIdException;

public class Tech3CHelper {
    private static final String TAG = "Tech3CHelper";
    private static Context sAppContext;
    private static Activity sCurrentActivity;
    private static boolean sIsInitialized = false;

    // Native callback methods
    public static native void nativeOnLoginSuccess(String userId, String accessToken, String refreshToken, int loginType, long expiryTime);
    public static native void nativeOnRegisterSuccess(String userId, String accessToken, String refreshToken, long expiryTime);
    public static native void nativeOnError(String error);
    public static native void nativeOnAuthCancelled();
    public static native void nativeOnAuthScreenOpened();

    /**
     * Set current activity and get application context
     * @param activity Current activity
     */
    public static void setActivity(Activity activity) {
        sCurrentActivity = activity;
        if (activity != null && sAppContext == null) {
            sAppContext = activity.getApplicationContext();
            Log.d(TAG, "Application context set from activity: " + activity.getClass().getSimpleName());
        }
    }

    /**
     * Initialize Tech3C SDK
     * @param clientId Client ID
     * @param clientSecret Client Secret
     */
    public static void initialize(String clientId, String clientSecret) {
        if (sAppContext == null) {
            Log.e(TAG, "Application context is null, cannot initialize Tech3C SDK");
            nativeOnError("Application context is null");
            return;
        }

        if (clientId == null || clientId.trim().isEmpty()) {
            Log.e(TAG, "Client ID is null or empty");
            nativeOnError("Client ID is null or empty");
            return;
        }

        if (clientSecret == null || clientSecret.trim().isEmpty()) {
            Log.e(TAG, "Client Secret is null or empty");
            nativeOnError("Client Secret is null or empty");
            return;
        }

        try {
            Log.d(TAG, "Initializing Tech3C SDK with clientId: " + clientId);
            Tech3CIdController.initialize(sAppContext, clientId, clientSecret)
                .setDebug(true)
                .setUiMode(UiMode.DIALOG)
                .setLanguageDisplay(Language.VIETNAMESE)
                .setDisableExitLogin(false)
                .setEnableGuestLogin(true)
                .setEnableMaintenanceCheck(true)
                .setOnAuthCallback(new OnAuthCallback() {
                    @Override
                    public void onLoginSuccess(String userId, String accessToken, String refreshToken, LoginType loginType, long expiryTime) {
                        Log.d(TAG, "Login success: " + userId + ", loginType: " + loginType);
                        nativeOnLoginSuccess(userId, accessToken, refreshToken, loginType.ordinal(), expiryTime);
                    }

                    @Override
                    public void onRegisterSuccess(String accessToken, String refreshToken, String userId, long expiryTime) {
                        Log.d(TAG, "Register success: " + userId);
                        nativeOnRegisterSuccess(userId, accessToken, refreshToken, expiryTime);
                    }

                    @Override
                    public void onAuthCancelled() {
                        Log.d(TAG, "Auth cancelled by user");
                        nativeOnAuthCancelled();
                    }

                    @Override
                    public void onAuthScreenOpened() {
                        Log.d(TAG, "Auth screen opened");
                        nativeOnAuthScreenOpened();
                    }

                    @Override
                    public void onError(Tech3CIdException exception) {
                        Log.e(TAG, "Auth error: " + exception.getMessage(), exception);
                        nativeOnError(exception.getMessage() != null ? exception.getMessage() : "Unknown error");
                    }
                });

            sIsInitialized = true;
            Log.d(TAG, "Tech3C SDK initialized successfully");

        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize Tech3C SDK", e);
            nativeOnError("Failed to initialize: " + (e.getMessage() != null ? e.getMessage() : "Unknown error"));
            sIsInitialized = false;
        }
    }

    /**
     * Show authentication screen
     */
    public static void showAuth() {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized, cannot show auth");
            nativeOnError("SDK not initialized");
            return;
        }

        if (sCurrentActivity == null) {
            Log.e(TAG, "Current activity is null, cannot show auth");
            nativeOnError("Current activity is null");
            return;
        }

        try {
            Log.d(TAG, "Showing authentication screen");
            Tech3CIdController.shared().showAuth();
        } catch (Exception e) {
            Log.e(TAG, "Failed to show auth", e);
            nativeOnError("Failed to show auth: " + (e.getMessage() != null ? e.getMessage() : "Unknown error"));
        }
    }

    /**
     * Set enable maintenance check
     * @param enable Enable maintenance check
     */
    public static void setEnableMaintenanceCheck(boolean enable) {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized");
            return;
        }

        try {
            Tech3CIdController.shared().setEnableMaintenanceCheck(enable);
            Log.d(TAG, "Maintenance check enabled: " + enable);
        } catch (Exception e) {
            Log.e(TAG, "Failed to set maintenance check", e);
        }
    }

    /**
     * Set debug mode
     * @param debug Debug mode enabled
     */
    public static void setDebugMode(boolean debug) {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized");
            return;
        }

        try {
            Tech3CIdController.shared().setDebug(debug);
            Log.d(TAG, "Debug mode set to: " + debug);
        } catch (Exception e) {
            Log.e(TAG, "Failed to set debug mode", e);
        }
    }

    /**
     * Set IP maintenance check
     * @param ip IP address for maintenance check
     */
    public static void setIpMaintenanceCheck(String ip) {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized");
            return;
        }

        try {
            Tech3CIdController.shared().setIpMaintenanceCheck(ip != null ? ip : "");
            Log.d(TAG, "IP maintenance check set to: " + (ip != null ? ip : "empty"));
        } catch (Exception e) {
            Log.e(TAG, "Failed to set IP maintenance check", e);
        }
    }

    /**
     * Set UI mode
     * @param uiMode UI mode (0 = Dialog, 1 = Fullscreen)
     */
    public static void setUiMode(int uiMode) {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized");
            return;
        }

        try {
            UiMode mode = uiMode == 1 ? UiMode.FULLSCREEN : UiMode.DIALOG;
            Tech3CIdController.shared().setUiMode(mode);
            Log.d(TAG, "UI mode set to: " + mode);
        } catch (Exception e) {
            Log.e(TAG, "Failed to set UI mode", e);
        }
    }

    /**
     * Set orientation mode
     * @param orientation Orientation mode (0 = Auto, 1 = Landscape, 2 = Portrait)
     */
    public static void setOrientation(int orientation) {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized");
            return;
        }

        try {
            OrientationMode orientationMode;
            switch (orientation) {
                case 1: orientationMode = OrientationMode.PORTRAIT; break;
                case 2: orientationMode = OrientationMode.LANDSCAPE; break;
                default: orientationMode = OrientationMode.AUTO; break;
            }

            Tech3CIdController.shared().setOrientation(orientationMode);
            Log.d(TAG, "Orientation set to: " + orientationMode);
        } catch (Exception e) {
            Log.e(TAG, "Failed to set orientation", e);
        }
    }

    /**
     * Set language
     * @param language Language code (0=EN, 1=VI, 2=CN, 3=KH, 4=LA, 5=TH)
     */
    public static void setLanguage(int language) {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized");
            return;
        }

        try {
            Language lang;
            switch (language) {
                case 1: lang = Language.VIETNAMESE; break;
                case 2: lang = Language.CHINESE; break;
                case 3: lang = Language.KHMER; break;
                case 4: lang = Language.LAO; break;
                case 5: lang = Language.THAI; break;
                default: lang = Language.ENGLISH; break;
            }

            Tech3CIdController.shared().setLanguageDisplay(lang);
            Log.d(TAG, "Language set to: " + lang);
        } catch (Exception e) {
            Log.e(TAG, "Failed to set language", e);
        }
    }

    /**
     * Set enable guest login
     * @param enable Enable guest login
     */
    public static void setEnableGuestLogin(boolean enable) {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized");
            return;
        }

        try {
            Tech3CIdController.shared().setEnableGuestLogin(enable);
            Log.d(TAG, "Guest login enabled: " + enable);
        } catch (Exception e) {
            Log.e(TAG, "Failed to set guest login", e);
        }
    }

    /**
     * Set disable exit login
     * @param disable Disable exit login
     */
    public static void setDisableExitLogin(boolean disable) {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized");
            return;
        }

        try {
            Tech3CIdController.shared().setDisableExitLogin(disable);
            Log.d(TAG, "Exit login disabled: " + disable);
        } catch (Exception e) {
            Log.e(TAG, "Failed to set exit login", e);
        }
    }

    /**
     * Set require OTP
     * @param require Require OTP
     */
    public static void setRequireOtp(boolean require) {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized");
            return;
        }

        try {
            Tech3CIdController.shared().setIsRequireOtp(require);
            Log.d(TAG, "Require OTP: " + require);
        } catch (Exception e) {
            Log.e(TAG, "Failed to set require OTP", e);
        }
    }

    /**
     * Logout user
     */
    public static void logout() {
        if (!sIsInitialized) {
            Log.e(TAG, "SDK not initialized, cannot logout");
            nativeOnError("SDK not initialized");
            return;
        }

        try {
            Log.d(TAG, "Logging out user");
             Tech3CIdController.shared().logout();

            Log.d(TAG, "User logged out successfully");

        } catch (Exception e) {
            Log.e(TAG, "Failed to logout", e);
            nativeOnError("Failed to logout: " + (e.getMessage() != null ? e.getMessage() : "Unknown error"));
        }
    }

    /**
     * Check if SDK is initialized
     * @return true if initialized
     */
    public static boolean isInitialized() {
        return sIsInitialized;
    }

    /**
     * Cleanup resources
     */
    public static void cleanup() {
        Log.d(TAG, "Cleaning up Tech3CHelper");
        sCurrentActivity = null;
        sIsInitialized = false;
    }

    /**
     * Called when activity resumes
     * @param activity Current activity
     */
    public static void onActivityResumed(Activity activity) {
        sCurrentActivity = activity;
        Log.d(TAG, "Activity resumed: " + activity.getClass().getSimpleName());
    }

    /**
     * Called when activity pauses
     */
    public static void onActivityPaused() {
        Log.d(TAG, "Activity paused");
    }

    /**
     * Called when activity destroys
     */
    public static void onActivityDestroyed() {
        Log.d(TAG, "Activity destroyed");
        sCurrentActivity = null;
    }
}
```

#### 2.3 Cập nhật AppActivity
Sửa file `proj.android/app/src/main/java/.../AppActivity.java`:

```java
import dev.axmol.lib.Tech3CHelper;

public class AppActivity extends AxmolActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Tech3CHelper.setActivity(this);
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        Tech3CHelper.onActivityResumed(this);
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        Tech3CHelper.onActivityDestroyed();
        Tech3CHelper.cleanup();
    }
}
```

Hoặc tham khảo bản đầy đủ tại [Tech3CHelper.java](https://github.com/tuanha1305/Tech3C-SDK-Axmol-Example/blob/main/proj.android/app/src/dev/axmol/lib/Tech3CHelper.java)

### 3. Sử dụng

#### 3.1 Khởi tạo SDK
Trong `AppDelegate.cpp` hoặc scene đầu tiên:

```cpp
#include "Tech3C/Tech3CManager.h"

// Initialize SDK
auto manager = tech3c::Tech3CManager::getInstance();
bool success = manager->initialize("your_client_id", "your_client_secret");

if (success) {
    // Configure SDK
    manager->setDebugMode(true);
    manager->setUiMode(tech3c::UiMode::DIALOG);
    manager->setLanguage(tech3c::Language::VIETNAMESE);
    manager->setOrientation(tech3c::OrientationMode::AUTO);
}
```

#### 3.2 Setup Callbacks
```cpp
// Set callbacks
manager->setLoginSuccessCallback([](const tech3c::UserInfo& userInfo) {
    // Handle login success
    AXLOGD("Login success: %s", userInfo.userId.c_str());
});

manager->setErrorCallback([](const tech3c::ErrorInfo& error) {
    // Handle error
    AXLOGE("Error: %s", error.message.c_str());
});

manager->setCancelCallback([]() {
    // Handle auth cancelled
    AXLOGD("Auth cancelled");
});
```

#### 3.3 Hiển thị Login
```cpp
// Show login dialog
manager->showAuth();
```

#### 3.4 Logout
```cpp
// Logout user
manager->logout();
```

#### 3.5 Check User Status
```cpp
// Check if user is logged in
if (manager->isLoggedIn()) {
    auto userInfo = manager->getCurrentUser();
    AXLOGD("Current user: %s", userInfo.userId.c_str());
}
```

### 4. API Reference

#### 4.1 Tech3CManager Methods
- `initialize(clientId, clientSecret)` - Khởi tạo SDK
- `showAuth()` - Hiển thị màn hình đăng nhập
- `logout()` - Đăng xuất
- `isLoggedIn()` - Kiểm tra trạng thái đăng nhập
- `getCurrentUser()` - Lấy thông tin user hiện tại

#### 4.2 Configuration Methods
- `setDebugMode(bool)` - Bật/tắt debug mode
- `setUiMode(UiMode)` - Đặt chế độ UI (Dialog/Fullscreen)
- `setLanguage(Language)` - Đặt ngôn ngữ
- `setOrientation(OrientationMode)` - Đặt hướng màn hình

#### 4.3 Callback Types
- `LoginSuccessCallback` - Callback khi đăng nhập thành công
- `ErrorCallback` - Callback khi có lỗi
- `CancelCallback` - Callback khi user hủy
- `AuthScreenOpenedCallback` - Callback khi mở màn hình auth

### 5. Troubleshooting

#### 5.1 Lỗi compilation
- Kiểm tra `minSdkVersion >= 21`
- Đảm bảo đã thêm repository và dependency đúng cách
- Clean và rebuild project

Dưới đây là phiên bản **đã chỉnh sửa chính xác** phần tài liệu về cấu hình iOS với Axmol để tránh lỗi **"không có Podfile"** nếu chưa chạy build trước:

---

## 2. iOS

### 1. Tích hợp Code

#### 2.1 Tạo cấu trúc thư mục
```
Source/
├── Tech3C/
│   ├── Tech3CManager.h
│   ├── Tech3CManager.cpp
│   ├── Tech3CTypes.h
│   ├── Tech3C-iOS.m
│   └── Tech3C-iOS.h
├── Scenes/
│   ├── LoginScene.h
│   └── LoginScene.cpp
└── AppDelegate.cpp
```

#### 2.2 Tạo iOS Bridge Header
Tạo file `Source/Tech3C/Tech3C-iOS.h`: Tham khảo tại [đây]()

```objc
#ifndef Tech3C_iOS_h
#define Tech3C_iOS_h

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for C++ types
typedef struct UserInfo UserInfo;
typedef struct ErrorInfo ErrorInfo;

// Function pointer types for callbacks
typedef void (*LoginSuccessCallback)(const char* userId, const char* accessToken, const char* refreshToken, int loginType, long expiryTime);
typedef void (*RegisterSuccessCallback)(const char* userId, const char* accessToken, const char* refreshToken, long expiryTime);
typedef void (*ErrorCallback)(const char* error);
typedef void (*CancelCallback)(void);
typedef void (*AuthScreenOpenedCallback)(void);

// iOS Bridge Functions
bool tech3c_ios_initialize(const char* appKey, const char* appSecret);
void tech3c_ios_cleanup(void);

// Configuration Methods
void tech3c_ios_setDebugMode(bool debug);
void tech3c_ios_setUiMode(int mode); // 0=Dialog, 1=Fullscreen
void tech3c_ios_setLanguage(int language); // 0=English, 1=Vietnamese, etc.
void tech3c_ios_setOrientation(int orientation); // 0=Auto, 1=Portrait, 2=Landscape
void tech3c_ios_setEnableGuestLogin(bool enable);
void tech3c_ios_setDisableExitLogin(bool disable);
void tech3c_ios_setRequireOtp(bool require);
void tech3c_ios_setEnableMaintenanceCheck(bool enable);
void tech3c_ios_setIpMaintenanceCheck(const char* ip);
void tech3c_ios_setTimeout(int timeoutSeconds);
void tech3c_ios_setEnvironment(int environment); // 0=Production, 1=Development
void tech3c_ios_setDialogSize(float width, float height);

// Authentication Methods
void tech3c_ios_showAuth(void);
void tech3c_ios_logout(void);
void tech3c_ios_changePassword(void);
void tech3c_ios_getUserInfo(void);

// Status Methods
bool tech3c_ios_isLogin(void);
const char* tech3c_ios_getAccessToken(void);
const char* tech3c_ios_getRefreshToken(void);
const char* tech3c_ios_getUserId(void);
const char* tech3c_ios_getDeviceId(void);
long tech3c_ios_getLoginTime(void);
long tech3c_ios_getTokenExpiry(void);
bool tech3c_ios_isTokenExpired(void);

// Callback Registration
void tech3c_ios_setLoginSuccessCallback(LoginSuccessCallback callback);
void tech3c_ios_setRegisterSuccessCallback(RegisterSuccessCallback callback);
void tech3c_ios_setErrorCallback(ErrorCallback callback);
void tech3c_ios_setCancelCallback(CancelCallback callback);
void tech3c_ios_setAuthScreenOpenedCallback(AuthScreenOpenedCallback callback);

#ifdef __cplusplus
}
#endif

#endif

```

#### 2.3 Tạo iOS Implementation
Tạo file `Source/Tech3C/Tech3C-iOS.m`:

```objc
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "Tech3C-iOS.h"

// Import Login3C framework
@import Login3C;

// Bridge class to handle callbacks
@interface Tech3CBridge : NSObject <OnAuthCallback>

@property (nonatomic, assign) LoginSuccessCallback loginSuccessCallback;
@property (nonatomic, assign) RegisterSuccessCallback registerSuccessCallback;
@property (nonatomic, assign) ErrorCallback errorCallback;
@property (nonatomic, assign) CancelCallback cancelCallback;
@property (nonatomic, assign) AuthScreenOpenedCallback authScreenOpenedCallback;

+ (instancetype)shared;

@end

@implementation Tech3CBridge

+ (instancetype)shared {
    static Tech3CBridge *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[Tech3CBridge alloc] init];
    });
    return instance;
}

#pragma mark - OnAuthCallback Implementation

- (void)onLoginSuccessWithUserId:(NSString *)userId 
                     accessToken:(NSString *)accessToken 
                    refreshToken:(NSString *)refreshToken 
                       loginType:(LoginType)loginType 
                      expiryTime:(int64_t)expiryTime {
    
    NSLog(@"[Tech3C Bridge] Login success: %@", userId);
    
    if (self.loginSuccessCallback) {
        self.loginSuccessCallback(
            userId.UTF8String,
            accessToken.UTF8String,
            refreshToken.UTF8String,
            (int)loginType,
            (long)expiryTime
        );
    }
}

- (void)onRegisterSuccessWithAccessToken:(NSString *)accessToken 
                            refreshToken:(NSString *)refreshToken 
                                  userId:(NSString *)userId 
                              expiryTime:(int64_t)expiryTime {
    
    NSLog(@"[Tech3C Bridge] Register success: %@", userId);
    
    if (self.registerSuccessCallback) {
        self.registerSuccessCallback(
            userId.UTF8String,
            accessToken.UTF8String,
            refreshToken.UTF8String,
            (long)expiryTime
        );
    }
}

- (void)onAuthCancelled {
    NSLog(@"[Tech3C Bridge] Auth cancelled");
    
    if (self.cancelCallback) {
        self.cancelCallback();
    }
}

- (void)onAuthScreenOpened {
    NSLog(@"[Tech3C Bridge] Auth screen opened");
    
    if (self.authScreenOpenedCallback) {
        self.authScreenOpenedCallback();
    }
}

- (void)onError:(Tech3CIdError *)error {
    NSLog(@"[Tech3C Bridge] Auth error: %@", error.message);
    
    if (self.errorCallback) {
        self.errorCallback(error.message.UTF8String);
    }
}

@end

#pragma mark - C Bridge Functions

bool tech3c_ios_initialize(const char* appKey, const char* appSecret) {
    if (!appKey || !appSecret) {
        NSLog(@"[Tech3C Bridge] ERROR: appKey or appSecret is null");
        return false;
    }
    
    NSString *nsAppKey = [NSString stringWithUTF8String:appKey];
    NSString *nsAppSecret = [NSString stringWithUTF8String:appSecret];
    
    NSLog(@"[Tech3C Bridge] Initializing with appKey: %@", nsAppKey);
    
    @try {
        NSError *error = nil;
        Tech3CIdController *controller = [Tech3CIdController initializeWithAppKey:nsAppKey 
                                                                         appSecret:nsAppSecret 
                                                                             error:&error];
        
        if (error) {
            NSLog(@"[Tech3C Bridge] ERROR: Failed to initialize SDK: %@", error.localizedDescription);
            return false;
        }
        
        // Set callback
        [controller setOnAuthCallback:[Tech3CBridge shared]];
        
        NSLog(@"[Tech3C Bridge] SDK initialized successfully");
        return true;
        
    } @catch (NSException *exception) {
        NSLog(@"[Tech3C Bridge] ERROR: Exception during initialization: %@", exception.reason);
        return false;
    }
}

void tech3c_ios_cleanup(void) {
    NSLog(@"[Tech3C Bridge] Cleaning up");
    
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        [controller cleanup];
    }
    
    // Clear callbacks
    Tech3CBridge *bridge = [Tech3CBridge shared];
    bridge.loginSuccessCallback = nil;
    bridge.registerSuccessCallback = nil;
    bridge.errorCallback = nil;
    bridge.cancelCallback = nil;
    bridge.authScreenOpenedCallback = nil;
}

#pragma mark - Configuration Methods

void tech3c_ios_setDebugMode(bool debug) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        [controller setDebug:debug];
        NSLog(@"[Tech3C Bridge] Debug mode set to: %s", debug ? "true" : "false");
    }
}

void tech3c_ios_setUiMode(int mode) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        UiMode uiMode = (mode == 0) ? UiModeDialog : UiModeFullscreen;
        [controller setUiMode:uiMode];
        NSLog(@"[Tech3C Bridge] UI mode set to: %s", (mode == 0) ? "Dialog" : "Fullscreen");
    }
}

void tech3c_ios_setLanguage(int language) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        Language lang = (Language)language;
        [controller setLanguageDisplay:lang];
        NSLog(@"[Tech3C Bridge] Language set to: %d", language);
    }
}

void tech3c_ios_setOrientation(int orientation) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        OrientationMode orientMode = (OrientationMode)orientation;
        [controller setOrientation:orientMode];
        NSLog(@"[Tech3C Bridge] Orientation set to: %d", orientation);
    }
}

void tech3c_ios_setEnableGuestLogin(bool enable) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        [controller setEnableGuestLogin:enable];
        NSLog(@"[Tech3C Bridge] Guest login enabled: %s", enable ? "true" : "false");
    }
}

void tech3c_ios_setDisableExitLogin(bool disable) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        [controller setDisableExitLogin:disable];
        NSLog(@"[Tech3C Bridge] Exit login disabled: %s", disable ? "true" : "false");
    }
}

void tech3c_ios_setRequireOtp(bool require) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        [controller setRequireOtp:require];
        NSLog(@"[Tech3C Bridge] Require OTP: %s", require ? "true" : "false");
    }
}

void tech3c_ios_setEnableMaintenanceCheck(bool enable) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        [controller setEnableMaintenanceCheck:enable];
        NSLog(@"[Tech3C Bridge] Maintenance check enabled: %s", enable ? "true" : "false");
    }
}

void tech3c_ios_setIpMaintenanceCheck(const char* ip) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller && ip) {
        NSString *nsIp = [NSString stringWithUTF8String:ip];
        [controller setServerIp:nsIp];
        NSLog(@"[Tech3C Bridge] Server IP set to: %@", nsIp);
    }
}

void tech3c_ios_setTimeout(int timeoutSeconds) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        [controller setTimeout:timeoutSeconds];
        NSLog(@"[Tech3C Bridge] Timeout set to: %d seconds", timeoutSeconds);
    }
}

void tech3c_ios_setEnvironment(int environment) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        Environment env = (environment == 0) ? EnvironmentProduction : EnvironmentDevelopment;
        [controller setEnvironment:env];
        NSLog(@"[Tech3C Bridge] Environment set to: %s", (environment == 0) ? "Production" : "Development");
    }
}

void tech3c_ios_setDialogSize(float width, float height) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        [controller setDialogSizeWithWidth:width height:height];
        NSLog(@"[Tech3C Bridge] Dialog size set to: %.1f x %.1f", width, height);
    }
}

#pragma mark - Authentication Methods

void tech3c_ios_showAuth(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        NSLog(@"[Tech3C Bridge] Showing auth screen");
        [controller showAuth];
    } else {
        NSLog(@"[Tech3C Bridge] ERROR: Controller not initialized");
    }
}

void tech3c_ios_logout(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        NSLog(@"[Tech3C Bridge] Logging out");
        [controller logout];
    }
}

void tech3c_ios_changePassword(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        NSLog(@"[Tech3C Bridge] Showing change password screen");
        [controller changePassword];
    }
}

void tech3c_ios_getUserInfo(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        NSLog(@"[Tech3C Bridge] Getting user info");
        [controller getUserInfo];
    }
}

#pragma mark - Status Methods

bool tech3c_ios_isLogin(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        return [controller isLogin];
    }
    return false;
}

const char* tech3c_ios_getAccessToken(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        NSString *token = [controller getAccessToken];
        return token ? strdup(token.UTF8String) : NULL;
    }
    return NULL;
}

const char* tech3c_ios_getRefreshToken(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        NSString *token = [controller getRefreshToken];
        return token ? strdup(token.UTF8String) : NULL;
    }
    return NULL;
}

const char* tech3c_ios_getUserId(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        NSString *userId = [controller getUserId];
        return userId ? strdup(userId.UTF8String) : NULL;
    }
    return NULL;
}

const char* tech3c_ios_getDeviceId(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        NSString *deviceId = [controller getDeviceId];
        return deviceId ? strdup(deviceId.UTF8String) : NULL;
    }
    return NULL;
}

long tech3c_ios_getLoginTime(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        return [controller getLoginTime];
    }
    return 0;
}

long tech3c_ios_getTokenExpiry(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        return [controller getTokenExpiry];
    }
    return 0;
}

bool tech3c_ios_isTokenExpired(void) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        return [controller isTokenExpired];
    }
    return false;
}

#pragma mark - Callback Registration

void tech3c_ios_setLoginSuccessCallback(LoginSuccessCallback callback) {
    [Tech3CBridge shared].loginSuccessCallback = callback;
}

void tech3c_ios_setRegisterSuccessCallback(RegisterSuccessCallback callback) {
    [Tech3CBridge shared].registerSuccessCallback = callback;
}

void tech3c_ios_setErrorCallback(ErrorCallback callback) {
    [Tech3CBridge shared].errorCallback = callback;
}

void tech3c_ios_setCancelCallback(CancelCallback callback) {
    [Tech3CBridge shared].cancelCallback = callback;
}

void tech3c_ios_setAuthScreenOpenedCallback(AuthScreenOpenedCallback callback) {
    [Tech3CBridge shared].authScreenOpenedCallback = callback;
}
```

#### 2.4 Cập nhật Tech3CManager để hỗ trợ iOS
Trong `Source/Tech3C/Tech3CManager.cpp`, thêm iOS implementation:

```cpp
#include "Tech3CManager.h"
#include "platform/PlatformConfig.h"

#if AX_TARGET_PLATFORM == AX_PLATFORM_IOS
#include "Tech3C-iOS.h"
#endif

namespace tech3c {

bool Tech3CManager::initializePlatform(const std::string& clientId, const std::string& clientSecret) {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    return initializeAndroid(clientId, clientSecret);
#elif AX_TARGET_PLATFORM == AX_PLATFORM_IOS
    return tech3c_ios_initialize(clientId.c_str(), clientSecret.c_str());
#else
    return false;
#endif
}

void Tech3CManager::showAuthPlatform() {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    showAuthAndroid();
#elif AX_TARGET_PLATFORM == AX_PLATFORM_IOS
    tech3c_ios_showAuth();
#endif
}

void Tech3CManager::logoutPlatform() {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    logoutAndroid();
#elif AX_TARGET_PLATFORM == AX_PLATFORM_IOS
    tech3c_ios_logout();
#endif
}

bool Tech3CManager::isLoggedInPlatform() {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    return isLoggedInAndroid();
#elif AX_TARGET_PLATFORM == AX_PLATFORM_IOS
    return tech3c_ios_isLoggedIn();
#else
    return false;
#endif
}

void Tech3CManager::setupCallbacks() {
#if AX_TARGET_PLATFORM == AX_PLATFORM_IOS
    tech3c_ios_setLoginSuccessCallback([](const char* userId, const char* accessToken, const char* refreshToken, int loginType, long expiryTime) {
        auto instance = Tech3CManager::getInstance();
        if (instance->_loginSuccessCallback) {
            UserInfo userInfo;
            userInfo.userId = userId;
            userInfo.accessToken = accessToken;
            userInfo.refreshToken = refreshToken;
            userInfo.loginType = static_cast<LoginType>(loginType);
            userInfo.expiryTime = expiryTime;
            instance->_loginSuccessCallback(userInfo);
        }
    });
    
    tech3c_ios_setRegisterSuccessCallback([](const char* userId, const char* accessToken, const char* refreshToken, long expiryTime) {
        auto instance = Tech3CManager::getInstance();
        if (instance->_registerSuccessCallback) {
            UserInfo userInfo;
            userInfo.userId = userId;
            userInfo.accessToken = accessToken;
            userInfo.refreshToken = refreshToken;
            userInfo.expiryTime = expiryTime;
            instance->_registerSuccessCallback(userInfo);
        }
    });
    
    tech3c_ios_setErrorCallback([](const char* error) {
        auto instance = Tech3CManager::getInstance();
        if (instance->_errorCallback) {
            ErrorInfo errorInfo;
            errorInfo.message = error;
            instance->_errorCallback(errorInfo);
        }
    });
    
    tech3c_ios_setCancelCallback([]() {
        auto instance = Tech3CManager::getInstance();
        if (instance->_cancelCallback) {
            instance->_cancelCallback();
        }
    });
    
    tech3c_ios_setAuthScreenOpenedCallback([]() {
        auto instance = Tech3CManager::getInstance();
        if (instance->_authScreenOpenedCallback) {
            instance->_authScreenOpenedCallback();
        }
    });
#endif
}

void Tech3CManager::setDebugModePlatform(bool enabled) {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    setDebugModeAndroid(enabled);
#elif AX_TARGET_PLATFORM == AX_PLATFORM_IOS
    tech3c_ios_setDebugMode(enabled);
#endif
}

void Tech3CManager::setUiModePlatform(UiMode mode) {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    setUiModeAndroid(static_cast<int>(mode));
#elif AX_TARGET_PLATFORM == AX_PLATFORM_IOS
    tech3c_ios_setUiMode(static_cast<int>(mode));
#endif
}

void Tech3CManager::setLanguagePlatform(Language language) {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    setLanguageAndroid(static_cast<int>(language));
#elif AX_TARGET_PLATFORM == AX_PLATFORM_IOS
    tech3c_ios_setLanguage(static_cast<int>(language));
#endif
}

void Tech3CManager::setOrientationPlatform(OrientationMode orientation) {
#if AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID
    setOrientationAndroid(static_cast<int>(orientation));
#elif AX_TARGET_PLATFORM == AX_PLATFORM_IOS
    tech3c_ios_setOrientation(static_cast<int>(orientation));
#endif
}

} // namespace tech3c
```

### 2. Build iOS

#### 2.1 Cấu hình Cmake

Sửa file `CMakeList.txt` thêm` Source/Tech3C/Tech3C-iOS.m` vào `GAME_SOURCE`. Tham khảo tại [đây]()
```
if(APPLE AND (CMAKE_SYSTEM_NAME STREQUAL "iOS"))
    list(APPEND GAME_SOURCE
         Source/Tech3C/Tech3C-iOS.m)
endif()
```

Sau đó thêm config framework search path cho iOS. Tham khảo tại [đây]()
```
if(APPLE)
    ...
    set(CMAKE_XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "\${BUILD_DIR}/\$(CONFIGURATION)\$(EFFECTIVE_PLATFORM_NAME)")
```

#### 2.2 Chuẩn bị thư mục iOS và tạo Podfile

> ⚠️ **Lưu ý quan trọng:** Bạn cần chạy lệnh build iOS trước để sinh thư mục `build_ios_arm64`, sau đó mới tạo được `Podfile`.

**Thực hiện như sau:**

```bash
axmol build -p ios -a arm64 -c
```

Sau khi chạy thành công, bạn sẽ có thư mục `build_ios_arm64` chứa project iOS (với file `.xcodeproj`).

#### 2.3 Thêm CocoaPods

Di chuyển vào thư mục:

```bash
cd build_ios_arm64
```

Nhớ kiểm tra xem trong `build_ios_arm64` có file `Podfile` chưa. Nếu chưa có, bạn mới tạo. Dùng lệnh sau:
```bash
pod init
```

Kiểm tra file pod file `Podfile` như sau, ví dụ `SampleAxmol` thường sẽ là tên dự án game của bạn:

```ruby
# Podfile
platform :ios, '11.0'
use_frameworks!

target 'SampleAxmol' do
  pod 'Login3C', :podspec => 'https://snapface.app/ios/Login3C.podspec'
end
```

Sau đó cài đặt Pods:

```bash
pod install
```

Mở file `.xcworkspace` để build:

```bash
open SampleAxmol.xcworkspace
```

### 3. Cấu hình Info.plist

Thêm các permissions cần thiết vào `proj.ios_mac/ios/Info.plist`:

```xml
<key>NSAppTransportSecurity</key>
<dict>
    <key>NSAllowsArbitraryLoads</key>
    <true/>
</dict>

<key>NSInternetUsageDescription</key>
<string>This app needs internet access for authentication</string>
```

### 4. Build Settings

Trong Xcode, đảm bảo các settings sau:
- **iOS Deployment Target**: 11.0 trở lên
- **Enable Bitcode**: NO
- **Always Embed Swift Standard Libraries**: YES (nếu SDK sử dụng Swift)

### 6. Troubleshooting

#### 6.1 Lỗi Pod Install
```bash
# Clean pod cache
pod cache clean --all
pod deintegrate
pod install
```

#### 6.2 Lỗi Build
- Kiểm tra iOS Deployment Target >= 11.0
- Đảm bảo đã mở file `.xcworkspace` chứ không phải `.xcodeproj`
- Kiểm tra các framework đã được link đúng cách

#### 6.3 Lỗi Runtime
- Kiểm tra Info.plist đã cấu hình đúng permissions
- Đảm bảo client ID và client secret đã được set đúng
- Kiểm tra callback functions đã được set trước khi gọi showAuth()


## 3. License
Distributed under the MIT License. See `LICENSE` for more information.

## 4. Support
For support, please email contact@3cgame.vn.
