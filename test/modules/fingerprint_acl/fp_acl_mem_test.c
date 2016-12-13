#include <modules/fingerprint_acl/fp_acl_memory.h>
#include <modules/fingerprint_acl/fp_acl_file.h>
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_logging.h>

#include <unistd.h>

bool fp_acl_mem_test_db(struct fp_acl_db* db) {
    db->clear();
    struct fp_acl_user user;
    memset(&user, 0, sizeof(struct fp_acl_user));
    memset(user.fp, 42, 16);
    const char* name = "foobar";
    memcpy(user.name, name, strlen(name)+1);
    user.permissions = 0x42424242;

    db->save(&user);
    
    void* first = db->first();
    db->next(first);
    void* user2 = db->find(user.fp);

    struct fp_acl_user user3;

    if (db->load(first, &user3) != FP_ACL_DB_OK) {
        return false;
    }

    if (memcmp(user3.fp, user.fp, 16) != 0) {
        return false;
    }

    if (db->load(user2, &user3) != FP_ACL_DB_OK) {
        return false;
    }
    if (memcmp(user3.fp, user.fp, 16) != 0) {
        return false;
    }
    
    db->remove(first);
    struct fp_acl_settings settings;
    db->load_settings(&settings);
    db->save_settings(&settings);
    
    return true;
}

bool fp_acl_mem_test() {
    struct fp_acl_settings defaultSettings;
    defaultSettings.systemPermissions = FP_ACL_SYSTEM_PERMISSION_ALL;
    defaultSettings.defaultPermissions = FP_ACL_PERMISSION_ALL;
    {
        struct fp_acl_db db;
        fp_mem_init(&db, &defaultSettings, NULL);
        if (!fp_acl_mem_test_db(&db)) {
            NABTO_LOG_ERROR(("memory backed acl test failed"));
            return false;
        }
    }

    {
        struct fp_acl_db db;

        struct fp_mem_persistence p;
        if (fp_acl_file_init("persistence.bin", "tmp.bin", &p) != FP_ACL_DB_OK) {
            NABTO_LOG_ERROR(("cannot load acl fole"));
            return false;
        }
        
        if (fp_mem_init(&db, &defaultSettings, &p) != FP_ACL_DB_OK) {
            return false;
        }
        if (!fp_acl_mem_test_db(&db)) {
            return false;
        }
    }

    {
        struct fp_acl_db db;

        struct fp_mem_persistence p;
        if (fp_acl_file_init("persistence2.bin", "tmp.bin", &p) != FP_ACL_DB_OK) {
            NABTO_LOG_ERROR(("cannot load acl file"));
            return false;
        }
        
        if (fp_mem_init(&db, &defaultSettings, &p) != FP_ACL_DB_OK) {
            return false;
        }

        if (db.clear() != FP_ACL_DB_OK) {
            return false;
        }

        struct fp_acl_user user;
        memset(&user, 0, sizeof(struct fp_acl_user));
        memset(user.fp, 42, 16);
        const char* name = "foobar";
        memcpy(user.name, name, strlen(name)+1);
        user.permissions = 0x42424242;
        if (db.save(&user) != FP_ACL_DB_OK) {
            return false;
        }

    }

    // load persistence2.bin and ensure the user exists
    {
        struct fp_acl_db db;
        
        struct fp_mem_persistence p;
        if (fp_acl_file_init("persistence2.bin", "tmp.bin", &p) != FP_ACL_DB_OK) {
            NABTO_LOG_ERROR(("cannot load acl fole"));
            return false;
        }
        if (fp_mem_init(&db, &defaultSettings, &p) != FP_ACL_DB_OK) {
            
            return false;
        }

        struct fp_acl_user user;
        memset(&user, 0, sizeof(struct fp_acl_user));
        memset(user.fp, 42, 16);
        
        void* it = db.find(user.fp);
        if (it == NULL) {
            NABTO_LOG_ERROR(("user not found"));
            return false;
        }
        
    }

    {
        unlink("persistence.bin");
        unlink("persistence2.bin");
    }
    
    return true;
}
