#include "fp_acl_memory.h"

static struct fp_mem_state state;
static struct fp_mem_persistence persistence;

static fp_acl_db_status fp_mem_persistence_null_save(struct fp_mem_state* unused) {
    return FP_ACL_DB_OK;
}

static fp_acl_db_status fp_mem_persistence_null_load(struct fp_mem_state* unused) {
    return FP_ACL_DB_OK;
}

// The acl list is a compacted memory representation of the devices.

bool fp_mem_is_slot_free(struct fp_acl_user* ix)
{
    return !ix->fp.hasValue;
}

struct fp_acl_user* fp_mem_find_free_slot()
{
    int i;
    for (i = 0; i < FP_MEM_ACL_ENTRIES; i++) {
        struct fp_acl_user* ix = &state.users[i];
        if (fp_mem_is_slot_free(ix)) {
            return ix;
        }
    }
    return NULL;
}

fp_acl_db_status fp_mem_init(struct fp_acl_db* db,
                             struct fp_acl_settings* defaultSettings,
                             struct fp_mem_persistence* p)
{
    if (db == NULL || defaultSettings == NULL) {
        return FP_ACL_DB_FAILED;
    }
    if (p != NULL) {
        persistence = *p;
    } else {
        persistence.save = &fp_mem_persistence_null_save;
        persistence.load = &fp_mem_persistence_null_load;
    }
    
    memset(&state, 0, sizeof(struct fp_mem_state));
    state.settings = *defaultSettings;

    db->first = &fp_mem_get_first_user;
    db->next = &fp_mem_next;
    db->find = &fp_mem_find;
    db->save = &fp_mem_save_user;
    db->load = &fp_mem_load_user;
    db->remove = &fp_mem_remove_user;
    db->clear = &fp_mem_clear;

    db->load_settings = &fp_mem_load_settings;
    db->save_settings = &fp_mem_save_settings;

    return persistence.load(&state);
}

void* fp_mem_get_first_user()
{
    int i;
    for (i = 0; i < FP_MEM_ACL_ENTRIES; i++) {
        struct fp_acl_user* ix = &state.users[i];
        if (!fp_mem_is_slot_free(ix)) {
            return (void*)ix;
        }
    }
    return NULL;
}

void* fp_mem_next(void* current)
{
    struct fp_acl_user* it = (struct fp_acl_user*)current;
    it++;
    for (; it < state.users + FP_MEM_ACL_ENTRIES; it++) {
        if (!fp_mem_is_slot_free(it)) {
            return it;
        }
    }
    return NULL;
}

void* fp_mem_find(const struct unabto_fingerprint* fp)
{
    int i;
    for (i = 0; i < FP_MEM_ACL_ENTRIES; i++) {
        struct fp_acl_user* ix = &state.users[i];
        if (memcmp(ix->fp.value, fp->value, sizeof(struct unabto_fingerprint)) == 0) {
            return ix;
        }
    }
    return NULL;
}

fp_acl_db_status fp_mem_save_user(struct fp_acl_user* user)
{
    void* index = fp_mem_find(&(user->fp));

    if (index == NULL) {
        index = fp_mem_find_free_slot();
    }
    
    if (index != NULL) {
        memcpy((struct fp_acl_user*)index, user, sizeof(struct fp_acl_user));
        return persistence.save(&state);
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
    return persistence.save(&state);
}

fp_acl_db_status fp_mem_clear()
{
    memset(&state, 0, sizeof(struct fp_mem_state));
    return persistence.save(&state);
}


fp_acl_db_status fp_mem_load_settings(struct fp_acl_settings* settings)
{
    memcpy(settings, &state.settings, sizeof(struct fp_acl_settings));
    return FP_ACL_DB_OK;
}

fp_acl_db_status fp_mem_save_settings(struct fp_acl_settings* settings)
{
    memcpy(&state.settings, settings, sizeof(struct fp_acl_settings));
    return persistence.save(&state);
}
