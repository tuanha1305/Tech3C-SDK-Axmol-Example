//
//  Tech3C-iOS.m
//  Axmol Tech3C iOS Bridge
//
//  Created on 4/22/25.
//

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

// todo: update 10/02/2026
void tech3c_ios_setEnableRequireBOD(bool enable) {
    Tech3CIdController *controller = [Tech3CIdController shared];
    if (controller) {
        [controller setEnableRequireBOD:enable];
        NSLog(@"[Tech3C Bridge] Enable Require BOD: %s", enable ? "true" : "false");
    } else {
        NSLog(@"[Tech3C Bridge] ERROR: Controller not initialized");
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
