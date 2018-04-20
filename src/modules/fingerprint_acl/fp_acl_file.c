#include "fp_acl_file.h"

#include <unabto/unabto_util.h>

#include "errno.h"

enum {
    USER_RECORD_SIZE = FP_ACL_FP_LENGTH + FP_ACL_FILE_USERNAME_LENGTH + 4
};

fp_acl_db_status fp_acl_file_read_file(FILE* aclFile, struct fp_mem_state* acl)
{
    uint8_t buffer[128];
    size_t readen;
    uint32_t version;
    uint8_t* ptr;
    uint32_t numUsers;
    uint32_t i;
    memset(acl, 0, sizeof(struct fp_mem_state));

    // load version
    readen = fread(buffer, 4, 1, aclFile);
    if (readen != 1) {
        return FP_ACL_DB_LOAD_FAILED;
    }

    READ_U32(version, buffer);
    if (version != FP_ACL_FILE_VERSION) {
        NABTO_LOG_ERROR(("Incompatible ACL version: read version %d, needs version %d", version, FP_ACL_FILE_VERSION));
        return FP_ACL_DB_LOAD_FAILED;
    }

    // load system settings
    readen = fread(buffer, 16, 1, aclFile);
    if (readen != 1) {
        return FP_ACL_DB_LOAD_FAILED;
    }

    ptr = buffer;
    
    READ_FORWARD_U32(acl->settings.systemPermissions, ptr);
    READ_FORWARD_U32(acl->settings.defaultUserPermissions, ptr);
    READ_FORWARD_U32(acl->settings.firstUserPermissions, ptr);
    READ_FORWARD_U32(numUsers, ptr);

    for (i = 0; i < numUsers && i < FP_MEM_ACL_ENTRIES; i++) {
        readen = fread(buffer, USER_RECORD_SIZE, 1, aclFile);
        if (readen != 1) {
            return FP_ACL_DB_LOAD_FAILED;
        }
        memcpy(acl->users[i].fp, buffer, FP_ACL_FP_LENGTH);
        memcpy(acl->users[i].name, buffer + FP_ACL_FP_LENGTH, FP_ACL_FILE_USERNAME_LENGTH); // guaranteed by compile time check
        READ_U32(acl->users[i].permissions, buffer + FP_ACL_FP_LENGTH + FP_ACL_FILE_USERNAME_LENGTH);
    }

    return FP_ACL_DB_OK;
}

static char* filename = NULL;
static char* tempFilename = NULL;

fp_acl_db_status fp_acl_file_load_file(struct fp_mem_state* acl)
{
    fp_acl_db_status status;
    FILE* aclFile = fopen(filename, "rb+");
    if (aclFile == NULL) {
        // there no saved acl file, consider it as a completely normal bootstrap scenario
        NABTO_LOG_INFO(("File %s does not exist yet", filename));
        return FP_ACL_DB_OK;
    }

	status = fp_acl_file_read_file(aclFile, acl);

    // close file, otherwise fp_acl_file_save_file() might throw error when updating the file
    fclose(aclFile);

    return status;
}

fp_acl_db_status fp_acl_file_save_file_temp(FILE* aclFile, struct fp_mem_state* acl)
{
    uint8_t buffer[128];
    uint8_t* ptr = buffer;
    uint32_t users = 0;
    int i;
    size_t written;
    for (i = 0; i < FP_MEM_ACL_ENTRIES; i++) {
        struct fp_acl_user* it = &acl->users[i];
        if (!fp_mem_is_slot_free(it)) {
            users++;
        }
    }

    WRITE_FORWARD_U32(ptr, FP_ACL_FILE_VERSION);
    WRITE_FORWARD_U32(ptr, acl->settings.systemPermissions);
    WRITE_FORWARD_U32(ptr, acl->settings.defaultUserPermissions);
    WRITE_FORWARD_U32(ptr, acl->settings.firstUserPermissions);
    WRITE_FORWARD_U32(ptr, users);

    written = fwrite(buffer, 20, 1, aclFile);
    if (written != 1) {
        return FP_ACL_DB_SAVE_FAILED;
    }

    // write user records
    
    for (i = 0; i < users; i++) {
        struct fp_acl_user* it = &acl->users[i];
        if (!fp_mem_is_slot_free(it)) {
            memcpy(buffer, it->fp, 16);
            memcpy(buffer+16, it->name, 64);
            WRITE_U32(buffer + 16 + 64, it->permissions);
            written = fwrite(buffer, 84, 1, aclFile);
            if (written != 1) {
                return FP_ACL_DB_SAVE_FAILED;
            }
        }
    }
    return FP_ACL_DB_OK;
}

fp_acl_db_status fp_acl_file_save_file(struct fp_mem_state* acl)
{
    fp_acl_db_status status;
    FILE* aclFile = fopen(tempFilename, "wb+");

    if (aclFile == NULL) {
        return FP_ACL_DB_SAVE_FAILED;
    }
    status = fp_acl_file_save_file_temp(aclFile, acl);
    
    if (fflush(aclFile) != 0) {
        NABTO_LOG_ERROR(("Could not flush temp file, errno=%d", errno));
        fclose(aclFile);
        return FP_ACL_DB_SAVE_FAILED;
    }

    if (fclose(aclFile) != 0) {
        NABTO_LOG_ERROR(("Could not close temp file, errno=%d", errno));
        return FP_ACL_DB_SAVE_FAILED;
    }

    if (status == FP_ACL_DB_OK) {
        // remove destination file, otherwise rename() might throw an error
        if (remove(filename) != 0) {
            NABTO_LOG_ERROR(("Could not remove temp file, errno=%d", errno));
            return FP_ACL_DB_SAVE_FAILED;
        }
        
        if (rename(tempFilename, filename) != 0) {
            NABTO_LOG_ERROR(("Could not rename temp file, errno=%d", errno));
            return FP_ACL_DB_SAVE_FAILED;
        } 
    }

    return status;
}



fp_acl_db_status fp_acl_file_init(const char* file, const char* tempFile, struct fp_mem_persistence* p)
{
    p->load = &fp_acl_file_load_file;
    p->save = &fp_acl_file_save_file;

    filename = strdup(file);
    tempFilename = strdup(tempFile);

    return FP_ACL_DB_OK;
}
