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
void tech3c_ios_setEnableRequireBOD(bool enable); // todo: update

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
