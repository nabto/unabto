#include <modules/fingerprint_acl/fp_acl_memory.h>

bool fp_acl_mem_test() {
    struct fp_acl_db db;
    fp_mem_init(&db);

    struct fp_acl_user user;
    memset(&user, 0, sizeof(struct fp_acl_user));
    memset(user.fp, 42, 16);
    const char* name = "foobar";
    memcpy(user.name, name, strlen(name)+1);
    user.permissions = 0x42424242;

    fp_mem_save_user(&user);
    
    void* first = fp_mem_get_first_user();
    fp_mem_next(first);
    void* user2 = fp_mem_find(user.fp);

    struct fp_acl_user user3;

    fp_mem_load_user(first, &user3);

    if (memcmp(user3.fp, user.fp, 16) != 0) {
        return false;
    }

    fp_mem_load_user(user2, &user3);
    if (memcmp(user3.fp, user.fp, 16) != 0) {
        return false;
    }
    
    fp_mem_remove_user(first);
    struct fp_acl_settings settings;
    fp_mem_load_settings(&settings);
    fp_mem_save_settings(&settings);
    
    return true;
}
