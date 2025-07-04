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
        maven { url 'https://dl.3cgame.vn/repository/' }
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
    
    public static void setActivity(Activity activity) {
        sCurrentActivity = activity;
        if (activity != null && sAppContext == null) {
            sAppContext = activity.getApplicationContext();
        }
    }
    
    public static void initialize(String clientId, String clientSecret) {
        if (sAppContext == null) {
            nativeOnError("Application context is null");
            return;
        }
        
        try {
            Tech3CIdController.initialize(sAppContext, clientId, clientSecret)
                .setDebug(true)
                .setUiMode(UiMode.DIALOG)
                .setLanguageDisplay(Language.VIETNAMESE)
                .setOnAuthCallback(new OnAuthCallback() {
                    @Override
                    public void onLoginSuccess(String userId, String accessToken, String refreshToken, LoginType loginType, long expiryTime) {
                        nativeOnLoginSuccess(userId, accessToken, refreshToken, loginType.ordinal(), expiryTime);
                    }
                    
                    @Override
                    public void onRegisterSuccess(String accessToken, String refreshToken, String userId, long expiryTime) {
                        nativeOnRegisterSuccess(userId, accessToken, refreshToken, expiryTime);
                    }
                    
                    @Override
                    public void onAuthCancelled() {
                        nativeOnAuthCancelled();
                    }
                    
                    @Override
                    public void onAuthScreenOpened() {
                        nativeOnAuthScreenOpened();
                    }
                    
                    @Override
                    public void onError(Tech3CIdException exception) {
                        nativeOnError(exception.getMessage());
                    }
                });
            
            sIsInitialized = true;
        } catch (Exception e) {
            nativeOnError("Failed to initialize: " + e.getMessage());
        }
    }
    
    public static void showAuth() {
        if (!sIsInitialized || sCurrentActivity == null) {
            nativeOnError("SDK not initialized or activity is null");
            return;
        }
        
        try {
            Tech3CIdController.shared().showAuth();
        } catch (Exception e) {
            nativeOnError("Failed to show auth: " + e.getMessage());
        }
    }
    
    public static void logout() {
        Log.d(TAG, "User logged out");
    }
    
    // Configuration methods
    public static void setDebugMode(boolean debug) {
        if (sIsInitialized) {
            Tech3CIdController.shared().setDebug(debug);
        }
    }
    
    public static void setUiMode(int uiMode) {
        if (sIsInitialized) {
            UiMode mode = uiMode == 1 ? UiMode.FULLSCREEN : UiMode.DIALOG;
            Tech3CIdController.shared().setUiMode(mode);
        }
    }
    
    public static void setLanguage(int language) {
        if (sIsInitialized) {
            Language lang = Language.ENGLISH;
            switch (language) {
                case 1: lang = Language.VIETNAMESE; break;
                case 2: lang = Language.CHINESE; break;
                case 3: lang = Language.KHMER; break;
                case 4: lang = Language.LAO; break;
                case 5: lang = Language.THAI; break;
            }
            Tech3CIdController.shared().setLanguageDisplay(lang);
        }
    }
    
    public static void setOrientation(int orientation) {
        if (sIsInitialized) {
            OrientationMode mode = OrientationMode.AUTO;
            switch (orientation) {
                case 1: mode = OrientationMode.LANDSCAPE; break;
                case 2: mode = OrientationMode.PORTRAIT; break;
            }
            Tech3CIdController.shared().setOrientation(mode);
        }
    }
    
    // Cleanup
    public static void cleanup() {
        sCurrentActivity = null;
        sIsInitialized = false;
    }
    
    public static void onActivityResumed(Activity activity) {
        sCurrentActivity = activity;
    }
    
    public static void onActivityDestroyed() {
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


# Hướng dẫn tích hợp Tech3C SDK cho Axmol - iOS

## 1. iOS

### 1. Cấu hình

#### 1.1 Cấu hình Deployment Target
Tech3C SDK yêu cầu iOS Deployment Target từ 11.0 trở lên. Vào `proj.ios_mac/ios/Info.plist` và kiểm tra:

```xml
<key>MinimumOSVersion</key>
<string>11.0</string>
```

#### 1.2 Thêm CocoaPods
Nếu chưa có file `Podfile` trong thư mục `proj.ios_mac/ios/`, tạo file `Podfile`:

```ruby
# Podfile
platform :ios, '11.0'
use_frameworks!

target 'YourAppName' do
  pod 'Login3C', :podspec => 'https://snapface.app/ios/Login3C.podspec'
end
```

Sau đó chạy:
```bash
cd proj.ios_mac/ios/
pod install
```

#### 1.3 Cấu hình Xcode Project
Mở file `.xcworkspace` (không phải `.xcodeproj`) và thêm các framework cần thiết:
- `SystemConfiguration.framework`
- `Security.framework`
- `WebKit.framework`

### 2. Tích hợp Code

#### 2.1 Tạo cấu trúc thư mục
```
Classes/
├── Tech3C/
│   ├── Tech3CManager.h
│   ├── Tech3CManager.cpp
│   ├── Tech3CTypes.h
│   └── Tech3C-iOS.h
├── Scenes/
│   ├── LoginScene.h
│   └── LoginScene.cpp
└── AppDelegate.cpp
```

#### 2.2 Tạo iOS Bridge Header
Tạo file `Classes/Tech3C/Tech3C-iOS.h`:

```objc
#ifndef Tech3C_iOS_h
#define Tech3C_iOS_h

#ifdef __cplusplus
extern "C" {
#endif

// Initialize SDK
bool tech3c_ios_initialize(const char* clientId, const char* clientSecret);

// Show auth screen
void tech3c_ios_showAuth();

// Logout
void tech3c_ios_logout();

// Check login status
bool tech3c_ios_isLoggedIn();

// Get current user info
const char* tech3c_ios_getCurrentUserId();
const char* tech3c_ios_getCurrentAccessToken();

// Configuration
void tech3c_ios_setDebugMode(bool enabled);
void tech3c_ios_setUiMode(int uiMode); // 0: Dialog, 1: Fullscreen
void tech3c_ios_setLanguage(int language); // 0: English, 1: Vietnamese, 2: Chinese, 3: Khmer, 4: Lao, 5: Thai
void tech3c_ios_setOrientation(int orientation); // 0: Auto, 1: Landscape, 2: Portrait

// Callback setters
void tech3c_ios_setLoginSuccessCallback(void (*callback)(const char* userId, const char* accessToken, const char* refreshToken, int loginType, long expiryTime));
void tech3c_ios_setRegisterSuccessCallback(void (*callback)(const char* userId, const char* accessToken, const char* refreshToken, long expiryTime));
void tech3c_ios_setErrorCallback(void (*callback)(const char* error));
void tech3c_ios_setCancelCallback(void (*callback)());
void tech3c_ios_setAuthScreenOpenedCallback(void (*callback)());

#ifdef __cplusplus
}
#endif

#endif /* Tech3C_iOS_h */
```

#### 2.3 Tạo iOS Implementation
Tạo file `proj.ios_mac/ios/Tech3CBridge.mm`:

```objc
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <Login3C/Login3C.h>
#import "Tech3C-iOS.h"

@interface Tech3CBridge : NSObject <Login3CDelegate>
@property (nonatomic, strong) Login3C *login3C;
@property (nonatomic, assign) void (*loginSuccessCallback)(const char*, const char*, const char*, int, long);
@property (nonatomic, assign) void (*registerSuccessCallback)(const char*, const char*, const char*, long);
@property (nonatomic, assign) void (*errorCallback)(const char*);
@property (nonatomic, assign) void (*cancelCallback)(void);
@property (nonatomic, assign) void (*authScreenOpenedCallback)(void);
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

- (instancetype)init {
    self = [super init];
    if (self) {
        self.login3C = [[Login3C alloc] init];
        self.login3C.delegate = self;
    }
    return self;
}

- (BOOL)initializeWithClientId:(NSString *)clientId clientSecret:(NSString *)clientSecret {
    if (!clientId || !clientSecret) {
        return NO;
    }
    
    Login3CConfig *config = [[Login3CConfig alloc] init];
    config.clientId = clientId;
    config.clientSecret = clientSecret;
    config.debugMode = YES;
    config.uiMode = Login3CUiModeDialog;
    config.language = Login3CLanguageVietnamese;
    config.orientation = Login3COrientationAuto;
    
    return [self.login3C initializeWithConfig:config];
}

- (void)showAuth {
    UIViewController *rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;
    [self.login3C showAuthFromViewController:rootViewController];
}

- (void)logout {
    [self.login3C logout];
}

- (BOOL)isLoggedIn {
    return [self.login3C isLoggedIn];
}

- (NSString *)getCurrentUserId {
    return [self.login3C getCurrentUserId];
}

- (NSString *)getCurrentAccessToken {
    return [self.login3C getCurrentAccessToken];
}

// Configuration methods
- (void)setDebugMode:(BOOL)enabled {
    [self.login3C setDebugMode:enabled];
}

- (void)setUiMode:(NSInteger)uiMode {
    Login3CUiMode mode = (uiMode == 1) ? Login3CUiModeFullscreen : Login3CUiModeDialog;
    [self.login3C setUiMode:mode];
}

- (void)setLanguage:(NSInteger)language {
    Login3CLanguage lang = Login3CLanguageEnglish;
    switch (language) {
        case 1: lang = Login3CLanguageVietnamese; break;
        case 2: lang = Login3CLanguageChinese; break;
        case 3: lang = Login3CLanguageKhmer; break;
        case 4: lang = Login3CLanguageLao; break;
        case 5: lang = Login3CLanguageThai; break;
    }
    [self.login3C setLanguage:lang];
}

- (void)setOrientation:(NSInteger)orientation {
    Login3COrientation orient = Login3COrientationAuto;
    switch (orientation) {
        case 1: orient = Login3COrientationLandscape; break;
        case 2: orient = Login3COrientationPortrait; break;
    }
    [self.login3C setOrientation:orient];
}

#pragma mark - Login3CDelegate

- (void)login3C:(Login3C *)login3C didLoginSuccessWithUserId:(NSString *)userId accessToken:(NSString *)accessToken refreshToken:(NSString *)refreshToken loginType:(Login3CLoginType)loginType expiryTime:(NSTimeInterval)expiryTime {
    if (self.loginSuccessCallback) {
        self.loginSuccessCallback([userId UTF8String], [accessToken UTF8String], [refreshToken UTF8String], (int)loginType, (long)expiryTime);
    }
}

- (void)login3C:(Login3C *)login3C didRegisterSuccessWithUserId:(NSString *)userId accessToken:(NSString *)accessToken refreshToken:(NSString *)refreshToken expiryTime:(NSTimeInterval)expiryTime {
    if (self.registerSuccessCallback) {
        self.registerSuccessCallback([userId UTF8String], [accessToken UTF8String], [refreshToken UTF8String], (long)expiryTime);
    }
}

- (void)login3C:(Login3C *)login3C didFailWithError:(NSError *)error {
    if (self.errorCallback) {
        self.errorCallback([[error localizedDescription] UTF8String]);
    }
}

- (void)login3CDidCancel:(Login3C *)login3C {
    if (self.cancelCallback) {
        self.cancelCallback();
    }
}

- (void)login3CDidOpenAuthScreen:(Login3C *)login3C {
    if (self.authScreenOpenedCallback) {
        self.authScreenOpenedCallback();
    }
}

@end

#pragma mark - C Interface

bool tech3c_ios_initialize(const char* clientId, const char* clientSecret) {
    NSString *nsClientId = [NSString stringWithUTF8String:clientId];
    NSString *nsClientSecret = [NSString stringWithUTF8String:clientSecret];
    return [[Tech3CBridge shared] initializeWithClientId:nsClientId clientSecret:nsClientSecret];
}

void tech3c_ios_showAuth() {
    [[Tech3CBridge shared] showAuth];
}

void tech3c_ios_logout() {
    [[Tech3CBridge shared] logout];
}

bool tech3c_ios_isLoggedIn() {
    return [[Tech3CBridge shared] isLoggedIn];
}

const char* tech3c_ios_getCurrentUserId() {
    NSString *userId = [[Tech3CBridge shared] getCurrentUserId];
    return userId ? [userId UTF8String] : "";
}

const char* tech3c_ios_getCurrentAccessToken() {
    NSString *token = [[Tech3CBridge shared] getCurrentAccessToken];
    return token ? [token UTF8String] : "";
}

void tech3c_ios_setDebugMode(bool enabled) {
    [[Tech3CBridge shared] setDebugMode:enabled];
}

void tech3c_ios_setUiMode(int uiMode) {
    [[Tech3CBridge shared] setUiMode:uiMode];
}

void tech3c_ios_setLanguage(int language) {
    [[Tech3CBridge shared] setLanguage:language];
}

void tech3c_ios_setOrientation(int orientation) {
    [[Tech3CBridge shared] setOrientation:orientation];
}

void tech3c_ios_setLoginSuccessCallback(void (*callback)(const char*, const char*, const char*, int, long)) {
    [Tech3CBridge shared].loginSuccessCallback = callback;
}

void tech3c_ios_setRegisterSuccessCallback(void (*callback)(const char*, const char*, const char*, long)) {
    [Tech3CBridge shared].registerSuccessCallback = callback;
}

void tech3c_ios_setErrorCallback(void (*callback)(const char*)) {
    [Tech3CBridge shared].errorCallback = callback;
}

void tech3c_ios_setCancelCallback(void (*callback)()) {
    [Tech3CBridge shared].cancelCallback = callback;
}

void tech3c_ios_setAuthScreenOpenedCallback(void (*callback)()) {
    [Tech3CBridge shared].authScreenOpenedCallback = callback;
}
```

#### 2.4 Cập nhật Tech3CManager để hỗ trợ iOS
Trong `Classes/Tech3C/Tech3CManager.cpp`, thêm iOS implementation:

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

### 3. Sử dụng

#### 3.1 Khởi tạo SDK
Trong `AppDelegate.cpp`:

```cpp
#include "Tech3C/Tech3CManager.h"

bool AppDelegate::applicationDidFinishLaunching() {
    // Initialize SDK
    auto manager = tech3c::Tech3CManager::getInstance();
    bool success = manager->initialize("your_client_id", "your_client_secret");
    
    if (success) {
        // Configure SDK
        manager->setDebugMode(true);
        manager->setUiMode(tech3c::UiMode::DIALOG);
        manager->setLanguage(tech3c::Language::VIETNAMESE);
        manager->setOrientation(tech3c::OrientationMode::AUTO);
        
        // Setup callbacks
        manager->setLoginSuccessCallback([](const tech3c::UserInfo& userInfo) {
            AXLOGD("iOS Login success: %s", userInfo.userId.c_str());
        });
        
        manager->setErrorCallback([](const tech3c::ErrorInfo& error) {
            AXLOGE("iOS Error: %s", error.message.c_str());
        });
        
        manager->setCancelCallback([]() {
            AXLOGD("iOS Auth cancelled");
        });
    }
    
    return true;
}
```

#### 3.2 Hiển thị Login
```cpp
// Show login dialog
auto manager = tech3c::Tech3CManager::getInstance();
manager->showAuth();
```

#### 3.3 Logout
```cpp
// Logout user
manager->logout();
```

#### 3.4 Check User Status
```cpp
// Check if user is logged in
if (manager->isLoggedIn()) {
    auto userInfo = manager->getCurrentUser();
    AXLOGD("Current user: %s", userInfo.userId.c_str());
}
```

### 4. Cấu hình Info.plist

Thêm các permissions cần thiết vào `proj.ios_mac/ios/Info.plist`:

```xml
<key>NSAppTransportSecurity</key>
<dict>
    <key>NSAllowsArbitraryLoads</key>
    <true/>
</dict>

<key>NSInternetUsageDescription</key>
<string>This app needs internet access for authentication</string>

<key>CFBundleURLTypes</key>
<array>
    <dict>
        <key>CFBundleURLName</key>
        <string>tech3c.auth</string>
        <key>CFBundleURLSchemes</key>
        <array>
            <string>your-app-scheme</string>
        </array>
    </dict>
</array>
```

### 5. Build Settings

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