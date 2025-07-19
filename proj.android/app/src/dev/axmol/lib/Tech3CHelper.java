package dev.axmol.lib;

import android.app.Activity;
import android.content.Context;
import android.util.Log;

import java.lang.ref.WeakReference;

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
    private static WeakReference<Activity> sCurrentActivityRef;
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
        sCurrentActivityRef = activity != null ? new WeakReference<>(activity) : null;
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
     * Get current activity safely
     * @return Current activity or null if not available
     */
    private static Activity getCurrentActivity() {
        return sCurrentActivityRef != null ? sCurrentActivityRef.get() : null;
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

        Activity currentActivity = getCurrentActivity();
        if (currentActivity == null) {
            Log.e(TAG, "Current activity is null, cannot show auth");
            nativeOnError("Current activity is null");
            return;
        }

        try {
            Log.d(TAG, "Showing authentication screen");
            Tech3CIdController.shared().showAuth(currentActivity);
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
        if (sCurrentActivityRef != null) {
            sCurrentActivityRef.clear();
            sCurrentActivityRef = null;
        }
        sIsInitialized = false;
        sAppContext = null;
    }

    /**
     * Called when activity resumes
     * @param activity Current activity
     */
    public static void onActivityResumed(Activity activity) {
        sCurrentActivityRef = new WeakReference<>(activity);
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
        if (sCurrentActivityRef != null) {
            Activity activity = sCurrentActivityRef.get();
            if (activity != null) {
                Log.d(TAG, "Clearing reference to destroyed activity: " + activity.getClass().getSimpleName());
            }
            sCurrentActivityRef.clear();
            sCurrentActivityRef = null;
        }
    }
}
