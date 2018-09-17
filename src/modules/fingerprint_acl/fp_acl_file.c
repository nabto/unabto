#include "fp_acl_file.h"

#include <unabto/unabto_util.h>

fp_acl_db_status fp_acl_file_read_file(FILE* aclFile, struct fp_mem_state* acl)
{
    uint8_t buffer[FP_ACL_RECORD_SIZE];
    size_t nread;
    uint32_t version;
    uint8_t* ptr;
    uint32_t numUsers;
    uint32_t i;

    UNABTO_ASSERT((sizeof(buffer) > 20));

    memset(acl, 0, sizeof(struct fp_mem_state));

    // load version
    nread = fread(buffer, 4, 1, aclFile);
    if (nread != 1) {
        return FP_ACL_DB_LOAD_FAILED;
    }

    READ_U32(version, buffer);
    if (version != FP_ACL_FILE_VERSION) {
        return FP_ACL_DB_LOAD_FAILED;
    }

    // load system settings
    nread = fread(buffer, 16, 1, aclFile);
    if (nread != 1) {
        return FP_ACL_DB_LOAD_FAILED;
    }

    ptr = buffer;
    
    READ_FORWARD_U32(acl->settings.systemPermissions, ptr);
    READ_FORWARD_U32(acl->settings.defaultUserPermissions, ptr);
    READ_FORWARD_U32(acl->settings.firstUserPermissions, ptr);
    READ_FORWARD_U32(numUsers, ptr);

    for (i = 0; i < numUsers && i < FP_MEM_ACL_ENTRIES; i++) {
        nread = fread(buffer, FP_ACL_RECORD_SIZE, 1, aclFile);
        ptr = buffer;
        if (nread != 1) {
            return FP_ACL_DB_LOAD_FAILED;
        }
        acl->users[i].fp.hasValue = 1;

        READ_FORWARD_MEM(acl->users[i].fp.value.data, ptr, FINGERPRINT_LENGTH);
        
        READ_FORWARD_U8(acl->users[i].pskId.hasValue, ptr);
        READ_FORWARD_MEM(acl->users[i].pskId.value.data, ptr, PSK_ID_LENGTH);
        
        READ_FORWARD_U8(acl->users[i].psk.hasValue, ptr);
        READ_FORWARD_MEM(acl->users[i].psk.value.data, ptr, PSK_LENGTH);

        READ_FORWARD_MEM(acl->users[i].name, ptr, FP_ACL_FILE_USERNAME_LENGTH); 
        
        READ_FORWARD_U32(acl->users[i].permissions, ptr);
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
        return FP_ACL_DB_OK;
    }

	status = fp_acl_file_read_file(aclFile, acl);

    // close file, otherwise fp_acl_file_save_file() might throw error when updating the file
    fclose(aclFile);

    return status;
}

fp_acl_db_status fp_acl_file_save_file_temp(FILE* aclFile, struct fp_mem_state* acl)
{
    uint8_t buffer[FP_ACL_RECORD_SIZE];
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

    written = fwrite(buffer, 5*sizeof(uint32_t), 1, aclFile);
    if (written != 1) {
        return FP_ACL_DB_SAVE_FAILED;
    }

    // write user records
    
    for (i = 0; i < users; i++) {
        struct fp_acl_user* it;
        ptr = buffer;
        it = &acl->users[i];
        if (!fp_mem_is_slot_free(it)) {

            WRITE_FORWARD_MEM(ptr, it->fp.value.data, FINGERPRINT_LENGTH);
            
            WRITE_FORWARD_U8(ptr, it->pskId.hasValue);
            WRITE_FORWARD_MEM(ptr, it->pskId.value.data, PSK_ID_LENGTH);

            WRITE_FORWARD_U8(ptr, it->psk.hasValue);
            WRITE_FORWARD_MEM(ptr, it->psk.value.data, PSK_LENGTH);

            WRITE_FORWARD_MEM(ptr, it->name, FP_ACL_FILE_USERNAME_LENGTH);

            WRITE_FORWARD_U32(ptr, it->permissions);
            
            written = fwrite(buffer, ptr-buffer, 1, aclFile);
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
    
    fflush(aclFile);
    fclose(aclFile);

    if (status == FP_ACL_DB_OK) {
        // remove destination file, otherwise rename() might throw an error 
        remove(filename);
        
        if (rename(tempFilename, filename) != 0) {
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
