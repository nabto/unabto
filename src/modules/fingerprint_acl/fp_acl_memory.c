#include "fp_acl_memory.h"

#define MEM_ACL_ENTRIES 32

static struct fp_acl_user users[MEM_ACL_ENTRIES];
static struct fp_acl_settings savedSettings;

// The acl list is a compacted memory representation of the devices.

bool fp_mem_is_slot_free(struct fp_acl_user* ix)
{
    fingerprint emptyFp;
    memset(emptyFp, 0, sizeof(fingerprint));
    bool fpIsEmpty = (memcmp(ix->fp, emptyFp, sizeof(fingerprint)) == 0);
    return fpIsEmpty;
}

struct fp_acl_user* fp_mem_find_free_slot()
{
    int i;
    for (i = 0; i < MEM_ACL_ENTRIES; i++) {
        struct fp_acl_user* ix = &users[i];
        if (fp_mem_is_slot_free(ix)) {
            return ix;
        }
    }
    return NULL;
}
    
void fp_mem_init(struct fp_acl_db* db)
{
    memset(&savedSettings, 0, sizeof(savedSettings));
    memset(users,0,sizeof(users));
    db->first = &fp_mem_get_first_user;
    db->next = &fp_mem_next;
    db->find = &fp_mem_find;
    db->save = &fp_mem_save_user;
    db->load = &fp_mem_load_user;
    db->remove = &fp_mem_remove_user;

    db->load_settings = &fp_mem_load_settings;
    db->save_settings = &fp_mem_save_settings;
}

void* fp_mem_get_first_user()
{
    return (void*)users;
}

void* fp_mem_next(void* current)
{
    struct fp_acl_user* it = (struct fp_acl_user*)current;
    it++;
    for (; it < users + MEM_ACL_ENTRIES; it++) {
        if (!fp_mem_is_slot_free(it)) {
            return it;
        }
    }
    return NULL;
}

void* fp_mem_find(fingerprint fp)
{
    int i;
    for (i = 0; i < MEM_ACL_ENTRIES; i++) {
        struct fp_acl_user* ix = &users[i];
        if (memcmp(ix->fp, fp, sizeof(fingerprint)) == 0) {
            return ix;
        }
    }
    return NULL;
}

fp_acl_db_status fp_mem_save_user(struct fp_acl_user* user)
{
    void* index = fp_mem_find(user->fp);

    if (index == NULL) {
        index = fp_mem_find_free_slot();
    }
    
    if (index != NULL) {
        memcpy((struct fp_acl_user*)index, user, sizeof(struct fp_acl_user));
        return FP_ACL_DB_OK;
    } else {
        return FP_ACL_DB_FULL;
    }
}

fp_acl_db_status fp_mem_load_user(void* it, struct fp_acl_user* user)
{
    struct fp_acl_user* ix = (struct fp_acl_user*)(it);
    memcpy(user, ix, sizeof(struct fp_acl_user));
    return FP_ACL_DB_OK;
}

fp_acl_db_status fp_mem_remove_user(void* it)
{
    struct fp_acl_user* ix = (struct fp_acl_user*)(it);
    memset(ix, 0, sizeof(struct fp_acl_user));
    return FP_ACL_DB_OK;
}

fp_acl_db_status fp_mem_load_settings(struct fp_acl_settings* settings)
{
    memcpy(settings, &savedSettings, sizeof(struct fp_acl_settings));
    return FP_ACL_DB_OK;
}

fp_acl_db_status fp_mem_save_settings(struct fp_acl_settings* settings)
{
    memcpy(&savedSettings, settings, sizeof(struct fp_acl_settings));
    return FP_ACL_DB_OK;
}
