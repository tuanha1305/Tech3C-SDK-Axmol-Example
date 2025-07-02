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