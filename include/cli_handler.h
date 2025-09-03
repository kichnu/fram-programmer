#ifndef CLI_HANDLER_H
#define CLI_HANDLER_H

#include <Arduino.h>
#include "fram_programmer.h"

// CLI Commands
enum CLICommand {
    CMD_HELP,
    CMD_DETECT,
    CMD_INFO,
    CMD_BACKUP,
    CMD_RESTORE,
    CMD_PROGRAM,
    CMD_VERIFY,
    CMD_CONFIG,
    CMD_TEST,
    CMD_UNKNOWN
};

// CLI Handler functions
void initCLI();
void handleCLI();
CLICommand parseCommand(const String& input);
void executeCommand(CLICommand cmd, const String& args);

// Command implementations
void cmdHelp();
void cmdDetect();
void cmdInfo();
void cmdBackup();
void cmdRestore();
void cmdProgram();
void cmdVerify();
void cmdConfig();
void cmdTest();

// Input handling
bool parseJSONCredentials(const String& json, DeviceCredentials& creds);
bool parseTextCredentials(DeviceCredentials& creds);
String readSerialLine();
void printPrompt();

// Output formatting
void printSuccess(const String& message);
void printError(const String& message);
void printWarning(const String& message);
void printInfo(const String& message);
void printHexDump(const uint8_t* data, size_t length);

// Backup/Restore helpers
bool sendBackupData();
bool receiveRestoreData(uint8_t* buffer, size_t expected_size);

#endif // CLI_HANDLER_H