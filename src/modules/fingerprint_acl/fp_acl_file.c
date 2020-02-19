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

        READ_FORWARD_MEM(acl->users[i].pushToken, ptr, FP_ACL_PUSH_TOKEN_MAX_LENGTH);
        NABTO_LOG_WARN(("%s", acl->users[i].pushToken));

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

            WRITE_FORWARD_MEM(ptr, it->pushToken, FP_ACL_PUSH_TOKEN_MAX_LENGTH);

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

static fp_acl_db_status convert_users_from_early_versions(FILE* aclFile, uint8_t* buffer, uint8_t* ptr, struct fp_mem_state* temp_acl, uint32_t numUsers)
{
    uint32_t i;
    for (i = 0; i < numUsers && i < FP_MEM_ACL_ENTRIES; i++) {
        uint32_t fp_size = 16;
        uint32_t name_size = 64;
        uint32_t size = fp_size + name_size;
        size_t nread;

        nread = fread(buffer, size, 1, aclFile);
        ptr = buffer;
        if (nread != 1) {
            return FP_ACL_DB_LOAD_FAILED;
        }

        temp_acl->users[i].fp.hasValue = 1;
        READ_FORWARD_MEM(temp_acl->users[i].fp.value.data, ptr, fp_size);

        temp_acl->users[i].pskId.hasValue = 0;
        temp_acl->users[i].psk.hasValue = 0;

        READ_FORWARD_MEM(temp_acl->users[i].name, ptr, name_size);

        READ_FORWARD_U32(temp_acl->users[i].permissions, ptr);
    }
    return FP_ACL_DB_OK;
}

static fp_acl_db_status fp_acl_file_convert_to_newest_version(FILE* aclFile, uint32_t from_version)
{
    struct fp_mem_state temp_acl = {0};

    uint8_t buffer[FP_ACL_RECORD_SIZE];
    size_t nread;
    uint8_t* ptr;
    uint32_t numUsers;
    fp_acl_db_status status;

    switch (from_version) {
        case 1: {
            nread = fread(buffer, 12, 1, aclFile);
            if (nread != 1) {
                return FP_ACL_DB_LOAD_FAILED;
            }

            ptr = buffer;

            READ_FORWARD_U32(temp_acl.settings.systemPermissions, ptr);
            READ_FORWARD_U32(temp_acl.settings.defaultUserPermissions, ptr);
            READ_FORWARD_U32(numUsers, ptr);
            temp_acl.settings.firstUserPermissions = 0;

            status = convert_users_from_early_versions(aclFile, buffer, ptr, &temp_acl, numUsers);
            if (status != FP_ACL_DB_OK) {
                return status;
            }
            break;
        }

        case 2: {
            nread = fread(buffer, 12, 1, aclFile);
            if (nread != 1) {
                return FP_ACL_DB_LOAD_FAILED;
            }

            ptr = buffer;

            READ_FORWARD_U32(temp_acl.settings.systemPermissions, ptr);
            READ_FORWARD_U32(temp_acl.settings.defaultUserPermissions, ptr);
            READ_FORWARD_U32(temp_acl.settings.firstUserPermissions, ptr);
            READ_FORWARD_U32(numUsers, ptr);

            status = convert_users_from_early_versions(aclFile, buffer, ptr, &temp_acl, numUsers);
            if (status != FP_ACL_DB_OK) {
                return status;
            }
            break;
        }

        case 3: {
            uint32_t i;

            nread = fread(buffer, 16, 1, aclFile);
            if (nread != 1) {
                return FP_ACL_DB_LOAD_FAILED;
            }

            ptr = buffer;

            READ_FORWARD_U32(temp_acl.settings.systemPermissions, ptr);
            READ_FORWARD_U32(temp_acl.settings.defaultUserPermissions, ptr);
            READ_FORWARD_U32(temp_acl.settings.firstUserPermissions, ptr);
            READ_FORWARD_U32(numUsers, ptr);

            for (i = 0; i < numUsers && i < FP_MEM_ACL_ENTRIES; i++) {
                uint32_t fp_size = 16;
                uint32_t psk_id_size = 16;
                uint32_t psk_size = 16;
                uint32_t name_size = 64;
                uint32_t size = fp_size + psk_id_size + psk_size + name_size + 6;

                nread = fread(buffer, size, 1, aclFile);
                ptr = buffer;
                if (nread != 1) {
                    return FP_ACL_DB_LOAD_FAILED;
                }

                temp_acl.users[i].fp.hasValue = 1;
                READ_FORWARD_MEM(temp_acl.users[i].fp.value.data, ptr, fp_size);

                READ_FORWARD_U8(temp_acl.users[i].pskId.hasValue, ptr);
                READ_FORWARD_MEM(temp_acl.users[i].pskId.value.data, ptr, psk_id_size);

                READ_FORWARD_U8(temp_acl.users[i].psk.hasValue, ptr);
                READ_FORWARD_MEM(temp_acl.users[i].psk.value.data, ptr, psk_size);

                READ_FORWARD_MEM(temp_acl.users[i].name, ptr, name_size);

                READ_FORWARD_U32(temp_acl.users[i].permissions, ptr);

            }
            break;
        }
    }

    return fp_acl_file_save_file(&temp_acl);
}

/**
 * Do the best possible to ensure a file is written to the disk.
 */
static void fp_acl_file_flush_and_sync_to_disk(FILE* file)
{
    fflush(file);
#ifndef WIN32
    fsync(fileno(file));
#endif
}

fp_acl_db_status fp_acl_file_save_file(struct fp_mem_state* acl)
{
    fp_acl_db_status status;
    FILE* aclFile = fopen(tempFilename, "wb+");

    if (aclFile == NULL) {
        return FP_ACL_DB_SAVE_FAILED;
    }
    status = fp_acl_file_save_file_temp(aclFile, acl);

    fp_acl_file_flush_and_sync_to_disk(aclFile);

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
    fp_acl_db_status status;
    FILE* aclFile;

    p->load = &fp_acl_file_load_file;
    p->save = &fp_acl_file_save_file;

    filename = strdup(file);
    tempFilename = strdup(tempFile);

    // Check the file version, convert to new format if need be.
    aclFile = fopen(filename, "rb+");

    if (aclFile != NULL) {
        uint32_t buffer[1];
        size_t nread;
        uint32_t version;

        status = FP_ACL_DB_OK;
        nread = fread(&buffer, 4, 1, aclFile);
        if (nread == 1) {
            READ_U32(version, buffer);
            if (version < FP_ACL_FILE_VERSION) {
                status = fp_acl_file_convert_to_newest_version(aclFile, version);
            }
        }
        fclose(aclFile);
        return status;
    } else {
        return FP_ACL_DB_OK;
    }

}
