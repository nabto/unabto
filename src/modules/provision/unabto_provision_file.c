#include "unabto_config.h"
#include "unabto/unabto_common_main.h"
#include "unabto_provision_file.h"

#if NABTO_ENABLE_PROVISIONING

static bool validate_string(char *string) {
    size_t i;
    size_t count = 0;
    char delim = UNABTO_PROVISION_FILE_DELIMITER;
    for (i = 0; i < strlen(string); i++) {
        count += (string[i] == delim);
    }
    return count == 1 && string[i-1] != delim && string[0] != delim;
}

static bool set_unabto_id(nabto_main_setup *nms, char *id)
{
    char *nabtoId = malloc(sizeof(char) * strlen(id));
    if (!nabtoId) {
        NABTO_LOG_ERROR(("Failed to allocate id"));
        return false;
    }

    sprintf(nabtoId, "%s", id);
    nms->id = nabtoId;
    return true;
}

bool unabto_provision_parse_data(nabto_main_setup *nms, char *data, char *key)
{
    NABTO_LOG_TRACE(("Parsing provision data: [%s]", data));
    char* tok;
    if (!validate_string(data)) {
        NABTO_LOG_ERROR(("Invalid provision string: %s", data));
        return false;
    }

    char delim[16];
    snprintf(delim, sizeof(16), "%c\n\r\t ", UNABTO_PROVISION_FILE_DELIMITER);
    tok = strtok(data, delim);
    if (!set_unabto_id(nms, tok)) {
        return false;
    }

    tok = strtok(NULL, delim);
    if (strlen(tok) != PRE_SHARED_KEY_SIZE * 2) {
        NABTO_LOG_ERROR(("Invalid key in provision string: %s", tok));
        return false;
    }

    strcpy(key, tok);
    return true;
}

bool unabto_provision_read_file(const char* path, char *text, size_t size)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        NABTO_LOG_TRACE(("Provisioning file not found at '%s'", path));
        return false;
    }
    bool ok = fgets(text, size, fp) > 0;
    if (!ok) {
        NABTO_LOG_TRACE(("Empty provisioning file found at '%s'", path));
    }
    return (fclose(fp) == 0) && ok;
}

bool unabto_provision_test_create_file(const char* path)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        NABTO_LOG_ERROR(("Could not open provisioning file '%s' for writing", path));
        return false;
    }
    bool ok = fprintf(fp, "%s", "") == 0;
    return (fclose(fp) == 0) && ok;
}

bool unabto_provision_write_file(const char* path, nabto_main_setup* nms)
{
    char text[128] = {0};
    size_t i = 0;
    size_t len = 0;

    len = snprintf(text, sizeof(text), "%s%c", nms->id, UNABTO_PROVISION_FILE_DELIMITER);
    for (i = 0; i < sizeof(nms->presharedKey)/sizeof(nms->presharedKey[0]) && len < sizeof(text); i++) {
        len += sprintf(text + len, "%02x", nms->presharedKey[i]);
    }

    FILE *fp = fopen(path, "wb");
    if (!fp) {
        NABTO_LOG_ERROR(("Could not open provisioning file '%s' for writing", path));
        return false;
    }
    bool ok = (fprintf(fp, "%.*s", (int)(sizeof(text)), text) == strlen(text));
    return (fclose(fp) == 0) && ok;
}

#endif
