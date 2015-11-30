/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_STORAGE

#include "settings.h"
#include <fcntl.h>
#include <unabto/unabto_external_environment.h>

#define MAXIMUM_LINE_LENGTH             1024

static bool initialized = false;
static char settingsFile[1024];
static char tempSettingsFile[1024];

static void initialize(void);

bool settings_read_string(const char* key, char* value)
{
    bool keyFound = false;
    FILE* file;
    
    if(initialized == false)
    {
        initialize();
    }
    
    file = fopen(settingsFile, "r");

    if(file != NULL)
    {
        int keyLength = strlen(key);
        char line[MAXIMUM_LINE_LENGTH];

        while(fgets(line, sizeof(line), file) == line && keyFound == false)
        {
            if(memcmp(line, key, keyLength) == 0 && line[keyLength] == '=')
            {
                int valueLength;

                strcpy(value, line + keyLength + 1);

                valueLength = strlen(value);
                if(value[valueLength - 1] == '\n')
                {
                    value[valueLength - 1] = 0;
                }

                keyFound = true;
            }
        }

        fclose(file);
    }

    return keyFound;
}

bool settings_write_string(const char* key, const char* value)
{
    char newLine[MAXIMUM_LINE_LENGTH];
    int testLength;
    FILE* tempFile;
    FILE* existingFile;
    bool writeDone = false;
    bool writeLineFeedDone = false;
    bool fileIsEmpty = true;
    
    if(initialized == false)
    {
        initialize();
    }

    sprintf(newLine, "%s=%s", key, value);

    testLength = strlen(key) + 1;

    tempFile = fopen(tempSettingsFile, "w");
    if(tempFile == NULL)
    {
        return false;
    }

    // if the settings file already exist copy it to the new settings file replacing the specified setting if it exists.
    existingFile = fopen(settingsFile, "r");
    if(existingFile != NULL)
    {
        char existingLine[MAXIMUM_LINE_LENGTH];
        while(fgets(existingLine, sizeof(existingLine), existingFile) == existingLine)
        {
            if(strlen(existingLine) > 0)
            {
                fileIsEmpty = false;
                if(memcmp(existingLine, newLine, testLength) != 0)
                {
                    if(writeDone == true && writeLineFeedDone == false)
                    {
                        fputc('\n', tempFile);
                        writeLineFeedDone = true;
                    }
                    fputs(existingLine, tempFile);
                }
                else
                {
                    fputs(newLine, tempFile);
                    writeDone = true;
                }
            }
        }

        fclose(existingFile);
    }

    if(writeDone == false)
    {
        if(fileIsEmpty == false)
        {
            fputc('\n', tempFile);
        }
        fputs(newLine, tempFile);
    }

    fclose(tempFile);

    {
        bool done = false;
        nabto_stamp_t timeout;
        nabtoSetFutureStamp(&timeout, 300);
        
        do
        {
            NABTO_LOG_TRACE(("Trying to swap temp settings file to actual settings file..."));
            remove(settingsFile); // this may fail if file has already been deleted so don't test the return value
            if(rename(tempSettingsFile, settingsFile) == 0)
            {
                done = true;
            }
            else if(nabtoIsStampPassed(&timeout))
            {
                NABTO_LOG_TRACE(("Unable to swap settings files!"));
                return false;
            }
        } while(done == false);

        //while(remove(settingsFile) != 0 || rename(tempSettingsFile, settingsFile) != 0)
        //{
        //    if(nabtoIsStampPassed(&timeout))
        //    {
        //        NABTO_LOG_TRACE(("Write settings - timeout"));
        //        return false;
        //    }
        //    NABTO_LOG_TRACE(("Write settings failed - trying again."));
        //}
        NABTO_LOG_TRACE(("Settings files swapped successfully"));
    }
    
    return true;
}

bool settings_read_int(const char* key, int* value)
{
    char rawValue[100];

    if(settings_read_string(key, rawValue) == false)
    {
        return false;
    }

    *value = strtol(rawValue, NULL, 0);

    return true;
}

bool settings_write_int(const char* key, int value)
{
    char rawValue[20];

    sprintf(rawValue, "%i", value);
    
    return settings_write_string(key, rawValue);
}

bool settings_read_bool(const char* key, bool* value)
{
    char rawValue[100];

    if(settings_read_string(key, rawValue) == false)
    {
        return false;
    }
    
    *value = strcmp(rawValue, "true") == 0;

    return true;
}

bool settings_write_bool(const char* key, bool value)
{
    return settings_write_string(key, value ? "true" : "false");
}

static void initialize(void)
{
    initialized = true;

#if WIN32
    strcpy(settingsFile, "c:/settings.txt");
    strcpy(tempSettingsFile, "c:/settings.txt");
    sprintf(tempSettingsFile + strlen(tempSettingsFile), ".%u", GetCurrentProcessId());
#else
    strcpy(settingsFile, "settings.txt");
    strcpy(tempSettingsFile, "settings.txt");
    sprintf(tempSettingsFile + strlen(tempSettingsFile), ".%u", getpid());
#endif
}
