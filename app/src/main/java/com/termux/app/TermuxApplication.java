package com.termux.app;

import android.app.Application;
import android.content.Context;

import com.termux.rafacodephi.BuildConfig;
import com.termux.shared.errors.Error;
import com.termux.shared.logger.Logger;
import com.termux.shared.termux.TermuxBootstrap;
import com.termux.shared.termux.TermuxConstants;
import com.termux.shared.termux.crash.TermuxCrashUtils;
import com.termux.shared.termux.file.TermuxFileUtils;
import com.termux.shared.termux.settings.preferences.TermuxAppSharedPreferences;
import com.termux.shared.termux.settings.properties.TermuxAppSharedProperties;
import com.termux.shared.termux.shell.command.environment.TermuxShellEnvironment;
import com.termux.shared.termux.shell.am.TermuxAmSocketServer;
import com.termux.shared.termux.shell.TermuxShellManager;
import com.termux.shared.termux.theme.TermuxThemeUtils;

public class TermuxApplication extends Application {

    private static final String LOG_TAG = "TermuxApplication";

    @Override
    public void onCreate() {
        super.onCreate();

        try {
            initializeApplication();
        } catch (Exception e) {
            // Log the error but don't crash the app during initialization
            Logger.logError(LOG_TAG, "Critical error during application initialization: " + e.getMessage());
            Logger.logStackTraceWithMessage(LOG_TAG, "Application initialization failed", e);
        }
    }

    /**
     * Initialize the application with comprehensive error handling.
     * This method contains all the initialization logic that was previously in onCreate.
     */
    private void initializeApplication() {
        Context context = getApplicationContext();

        // Set crash handler for the app - do this first to catch any subsequent crashes
        try {
            TermuxCrashUtils.setDefaultCrashHandler(this);
        } catch (Exception e) {
            Logger.logError(LOG_TAG, "Failed to set crash handler: " + e.getMessage());
        }

        // Set log config for the app
        try {
            setLogConfig(context);
        } catch (Exception e) {
            Logger.logError(LOG_TAG, "Failed to set log config: " + e.getMessage());
        }

        Logger.logDebug("Starting Application");

        // Set TermuxBootstrap.TERMUX_APP_PACKAGE_MANAGER and TermuxBootstrap.TERMUX_APP_PACKAGE_VARIANT
        try {
            TermuxBootstrap.setTermuxPackageManagerAndVariant(BuildConfig.TERMUX_PACKAGE_VARIANT);
        } catch (Exception e) {
            Logger.logError(LOG_TAG, "Failed to set package manager variant: " + e.getMessage());
        }

        // Init app wide SharedProperties loaded from termux.properties
        TermuxAppSharedProperties properties = null;
        try {
            properties = TermuxAppSharedProperties.init(context);
        } catch (Exception e) {
            Logger.logError(LOG_TAG, "Failed to initialize shared properties: " + e.getMessage());
        }

        // Init app wide shell manager
        try {
            TermuxShellManager.init(context);
        } catch (Exception e) {
            Logger.logError(LOG_TAG, "Failed to initialize shell manager: " + e.getMessage());
        }

        // Set NightMode.APP_NIGHT_MODE
        try {
            if (properties != null) {
                TermuxThemeUtils.setAppNightMode(properties.getNightMode());
            }
        } catch (Exception e) {
            Logger.logError(LOG_TAG, "Failed to set night mode: " + e.getMessage());
        }

        // Check and create termux files directory. If failed to access it like in case of secondary
        // user or external sd card installation, then don't run files directory related code
        Error error = null;
        boolean isTermuxFilesDirectoryAccessible = false;
        try {
            error = TermuxFileUtils.isTermuxFilesDirectoryAccessible(this, true, true);
            isTermuxFilesDirectoryAccessible = error == null;
        } catch (Exception e) {
            Logger.logError(LOG_TAG, "Failed to check files directory accessibility: " + e.getMessage());
        }
        
        if (isTermuxFilesDirectoryAccessible) {
            Logger.logInfo(LOG_TAG, "Termux files directory is accessible");

            try {
                error = TermuxFileUtils.isAppsTermuxAppDirectoryAccessible(true, true);
                if (error != null) {
                    Logger.logErrorExtended(LOG_TAG, "Create apps/termux-app directory failed\n" + error);
                    return;
                }

                // Setup termux-am-socket server
                TermuxAmSocketServer.setupTermuxAmSocketServer(context);
            } catch (Exception e) {
                Logger.logError(LOG_TAG, "Failed to setup app directory or socket server: " + e.getMessage());
            }
        } else {
            Logger.logErrorExtended(LOG_TAG, "Termux files directory is not accessible\n" + error);
        }

        // Init TermuxShellEnvironment constants and caches after everything has been setup including termux-am-socket server
        try {
            TermuxShellEnvironment.init(this);
        } catch (Exception e) {
            Logger.logError(LOG_TAG, "Failed to initialize shell environment: " + e.getMessage());
        }

        if (isTermuxFilesDirectoryAccessible) {
            try {
                TermuxShellEnvironment.writeEnvironmentToFile(this);
            } catch (Exception e) {
                Logger.logError(LOG_TAG, "Failed to write environment to file: " + e.getMessage());
            }
        }
    }

    public static void setLogConfig(Context context) {
        Logger.setDefaultLogTag(TermuxConstants.TERMUX_APP_NAME);

        // Load the log level from shared preferences and set it to the {@link Logger.CURRENT_LOG_LEVEL}
        TermuxAppSharedPreferences preferences = TermuxAppSharedPreferences.build(context);
        if (preferences == null) return;
        preferences.setLogLevel(null, preferences.getLogLevel());
    }

}
