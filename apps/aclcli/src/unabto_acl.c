/************************************************************************************************
 * 
 * Filename   : unabto_acl.c 
 * Description: To add, remove & list ACL file contents
 * Usage      : unabto_acl <[add]/[remove]/[list]> <ACL filename>
 * Note       : During addition of new users,  by default allow local access and remote access 
 *              permissions will be granted.
 *              In case, the new user is the first user, then admin access will be granted.
 * 
 ************************************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#include "modules/cli/gopt/gopt.h"
#include "modules/util/read_hex.h"
#include "unabto/unabto_env_base.h"
#include "modules/fingerprint_acl/fp_acl_file.h"
#include "unabto/unabto_types.h"

struct configuration {
    const char *action;
    const char *aclfilename;
    const char *fingerprint;
    const char *user;
    const char *pskId;
    const char *pskKey;
};

fp_acl_db_status fp_acl_file_remove_entry(struct configuration* config, struct fp_acl_db* db);
fp_acl_db_status fp_acl_file_add_entry(struct configuration* config, struct fp_acl_db* db);
fp_acl_db_status fp_acl_file_list_entries(struct configuration* config, struct fp_acl_db* db);
fp_acl_db_status fp_acl_file_set_psk(struct configuration* config, struct fp_acl_db* db);

bool parse_argv(int argc, char* argv[], struct configuration* config);
bool fp_get_fingerprint(const char *, struct unabto_fingerprint* fpLocal);
bool fp_get_psk_id(const char *fpargv, struct unabto_psk_id* pskId);
bool fp_get_psk_key(const char *fpargv, struct unabto_psk* pskKey);

#define splithex(x)  x >> 16 , x & 0xffff

static void help(const char* errmsg, const char *progname)
{
    if (errmsg) {
        printf("ERROR: %s\n", errmsg);
    }
    printf("Usage: %s <list/add/remove> -F <acl filename> [-f <fingerprint>] [-u <user>]\n\n", progname);
    printf("Example: \n");
    printf("       %s list -F <acl filename>\n", progname);
    printf("       %s add -F <acl filename> -f <fingerprint> -u <user>\n", progname);
    printf("       %s remove -F <acl filename> -f <fingerprint>\n", progname);
    printf("       %s set-psk -F <acl filename> -f <fingerprint> -i <psk id> -k <psk>\n\n", progname);
}


int main(int argc, char* argv[]) {
    struct configuration config;
    struct fp_mem_state acl;
    struct fp_acl_settings defaultSettings;
    struct fp_acl_db db;
    struct fp_mem_persistence p;
    fp_acl_db_status st;
    FILE* aclFile;
    
    memset(&config, 0, sizeof(struct configuration));

    if (!parse_argv(argc, argv, &config)) {
        help(0, argv[0]);
        return 1;
    }

    // master switch: system allows both local and remote access to
    // users with the right privileges and is open for pairing with new users 
    defaultSettings.systemPermissions =
        FP_ACL_SYSTEM_PERMISSION_PAIRING |
        FP_ACL_SYSTEM_PERMISSION_LOCAL_ACCESS |
        FP_ACL_SYSTEM_PERMISSION_REMOTE_ACCESS;

    // first user is granted admin permission and local+remote access
    defaultSettings.firstUserPermissions =
        FP_ACL_PERMISSION_ADMIN |
        FP_ACL_PERMISSION_LOCAL_ACCESS |
        FP_ACL_PERMISSION_REMOTE_ACCESS;

    // subsequent users will just have guest privileges but still have local+remote access
    defaultSettings.defaultUserPermissions =
        FP_ACL_PERMISSION_LOCAL_ACCESS |
        FP_ACL_PERMISSION_REMOTE_ACCESS;
    
    // Test file existence
    aclFile = fopen(config.aclfilename, "rb+");
    if (aclFile == NULL) {
        printf("File %s does not exist, creating new\n", config.aclfilename);
    }
    fclose(aclFile);

    // initialise the acl database

    if ((st = fp_acl_file_init(config.aclfilename, "tmp.bin", &p)) != FP_ACL_DB_OK) {
        NABTO_LOG_ERROR(("cannot init (status %d): fp_acl_file_init: %s", st, config.aclfilename));
        return 1;
    }
        
    if ((st = fp_mem_init(&db, &defaultSettings, &p)) != FP_ACL_DB_OK) {
        NABTO_LOG_ERROR(("cannot init (status %d): fp_mem_init", st));
        return 1;
    }

    if (!strcmp(config.action, "add")) {
        if ((st = fp_acl_file_add_entry(&config, &db)) != FP_ACL_DB_OK) {
            NABTO_LOG_ERROR(("Add Failed (status %d)\n", st));
            return 1;
        }
    }
    
    if (!strcmp(config.action, "list")) {
        if ((st = fp_acl_file_list_entries(&config, &db)) != FP_ACL_DB_OK) {
            NABTO_LOG_ERROR(("List Failed (status %d)\n", st));
            return 1;
        }
    }

    if (!strcmp(config.action, "remove")) {
        if ((st = fp_acl_file_remove_entry(&config, &db)) != FP_ACL_DB_OK) {
            NABTO_LOG_ERROR(("Remove Failed (status %d)\n", st));
            return 1;
        }
    }

    if (!strcmp(config.action, "set-psk")) {
        if ((st = fp_acl_file_set_psk(&config, &db)) != FP_ACL_DB_OK) {
            NABTO_LOG_ERROR(("Set PSK Failed with status %d\n", st));
            return 1;
        }
    }

    return 0;
}

void print_hex(uint8_t* data, size_t len) {
    int i;
    for (i=0; i<len; i++) {
        if (i) {
            printf(":");
        };
        printf("%02x", data[i]);
    }
}

void print_psk(struct fp_acl_user* user) {
    if (user->pskId.hasValue) {
        printf("  PSK: [");
        print_hex(user->pskId.value.data, FP_ACL_PSK_ID_LENGTH);
        printf("] => [");
        if (user->psk.hasValue) {
            print_hex(user->psk.value.data, FP_ACL_PSK_KEY_LENGTH);
        } else {
            printf("(none)");
        }
        printf("]\n");
    }
}

// listing of ACL file
fp_acl_db_status fp_acl_file_list_entries(struct configuration* config, struct fp_acl_db* db)
{

    void* iterator;
    int numUsers = 0;

    struct fp_acl_settings acl_settings;
  
    if(db->load_settings(&acl_settings) != FP_ACL_DB_OK) {
        NABTO_LOG_ERROR(("Could not load acl"));
        return FP_ACL_DB_LOAD_FAILED;
    }

    printf("System Permissions       : %04x:%04x \n", splithex(acl_settings.systemPermissions));
    printf("Default User Permissions : %04x:%04x \n", splithex(acl_settings.defaultUserPermissions));
    printf("First User Permissions   : %04x:%04x \n", splithex(acl_settings.firstUserPermissions));


    // Count users
    iterator = db->first();
    // move pointer forward
    while (iterator != NULL) {
        iterator = db->next(iterator);
        numUsers++;
    } 

    printf("Number of users = %d\n", numUsers);
  
    numUsers = 0;

    // Run through db and list users
    iterator = db->first();
    // move pointer forward
    while (iterator != NULL) {

        struct fp_acl_user user;
        numUsers++;

        if (db->load(iterator, &user) != FP_ACL_DB_OK) {
            printf("Could not load user %d", numUsers);
        } else {
            print_hex(user.fp.value.data, FP_ACL_FP_LENGTH);
            printf("  %04x:%04x  %s\n", splithex(user.permissions), user.name);
            print_psk(&user);
        }
        iterator=db->next(iterator);

    }
  
    return FP_ACL_DB_OK;
}

// removing an entry from ACL file
fp_acl_db_status fp_acl_file_remove_entry(struct configuration* config, struct fp_acl_db* db)
{
  
    void* it;
    struct fp_acl_user user;
    struct fp_acl_settings aclSettings;

    // Load acl settings from file
    if(db->load_settings(&aclSettings) != FP_ACL_DB_OK) {
        NABTO_LOG_ERROR(("Could not load aclsettings"));
        return FP_ACL_DB_LOAD_FAILED;
    }



    // If first user - use firstUserPermissions otherwise use defaultUserPermission
    if (db->first() == NULL) {
        user.permissions = aclSettings.firstUserPermissions;
    } else {
        user.permissions = aclSettings.defaultUserPermissions;
    }

  
    if (fp_get_fingerprint(config->fingerprint, &(user.fp.value)) != 1) {
        NABTO_LOG_ERROR(("Invalid Fingerprint\n"));
        return FP_ACL_DB_LOAD_FAILED;
    }


    it = db->find(&(user.fp.value));
  
    if (it == 0 || db->load(it, &user) != FP_ACL_DB_OK) {
        printf("No such fingerprint");
    } else {
        if (db->remove(it) == FP_ACL_DB_OK) {
            printf("Remove user");
        } else {
            printf("Could not remove user");
        }
    }

  
    return FP_ACL_DB_OK;

}

// adding an entry to ACL file
fp_acl_db_status fp_acl_file_add_entry(struct configuration* config, struct fp_acl_db* db)
{
  
    fp_acl_db_status status;

    struct fp_acl_user user;
    struct fp_acl_settings aclSettings;

    fp_acl_init_user(&user);

    // Load acl settings from file
    if (db->load_settings(&aclSettings) != FP_ACL_DB_OK) {
        NABTO_LOG_ERROR(("Could not load aclsettings"));
        return FP_ACL_DB_LOAD_FAILED;
    }

    // If first user - use firstUserPermissions otherwise use defaultUserPermission
    if (db->first() == NULL) {
        user.permissions = aclSettings.firstUserPermissions;
    } else {
        user.permissions = aclSettings.defaultUserPermissions;
    }

    if (fp_get_fingerprint(config->fingerprint, &user.fp.value) != 1) {
        NABTO_LOG_ERROR(("Invalid Fingerprint\n"));
        return FP_ACL_DB_SAVE_FAILED;
    }
    user.fp.hasValue = 1;


    strncpy(user.name, config->user, FP_ACL_USERNAME_MAX_LENGTH);

    return db->save(&user);
}

fp_acl_db_status fp_acl_file_set_psk(struct configuration* config, struct fp_acl_db* db)
{
    void* it;
    struct unabto_fingerprint fp;
    struct fp_acl_user user;

    fp_get_fingerprint(config->fingerprint, &fp);
    it = db->find(&fp);
    
    if (it == 0 || db->load(it, &user) != FP_ACL_DB_OK) {
        return FP_ACL_DB_SAVE_FAILED;
    }

    if (fp_get_psk_id(config->pskId, &user.pskId.value) != 1) {
        NABTO_LOG_ERROR(("Invalid key id\n"));
        return FP_ACL_DB_SAVE_FAILED;
    }
    user.pskId.hasValue = 1;

    if (fp_get_psk_key(config->pskKey, &user.psk.value) != 1) {
        NABTO_LOG_ERROR(("Invalid key\n"));
        return FP_ACL_DB_SAVE_FAILED;
    }
    user.psk.hasValue = 1;

    return db->save(&user);
}

bool fp_read_hex(const char *fpargv, uint8_t* buf, size_t len)
{
    uint32_t i,j = 0;

    i=0;j=0; bool last;

    if(strlen(fpargv)!=len*3-1) 
        return false;

    for (j=0; j<len;j++) {
        last = (j == len-1);

        if(strchr("0123456789abcdefABCDEF", fpargv[i]) == NULL ||
           strchr("0123456789abcdefABCDEF", fpargv[i+1]) == NULL) {
            printf("invalid hexchar in finger print");
            return false;
        }
        if(!last) {
            if(fpargv[i+2] != ':') {
                printf("hexvalues must be separated by ':'  ");
                return false;
            }
            sscanf(fpargv+i, "%2hhx:", &buf[j]);
        } else {
            sscanf(fpargv+i, "%2hhx", &buf[j]);
        }
        i+=3;

    }

    return true;
}

bool fp_get_fingerprint(const char *fpargv, struct unabto_fingerprint* fp)
{
    return fp_read_hex(fpargv, fp->data, FP_ACL_FP_LENGTH);
}

bool fp_get_psk_id(const char *fpargv, struct unabto_psk_id* pskId)
{
    return fp_read_hex(fpargv, pskId->data, FP_ACL_PSK_ID_LENGTH);
}

bool fp_get_psk_key(const char *fpargv, struct unabto_psk* pskKey)
{
    return fp_read_hex(fpargv, pskKey->data, FP_ACL_PSK_KEY_LENGTH);
}

bool parse_argv(int argc, char* argv[], struct configuration* config) 
{
    const char x0s[] = "h?";     const char* x0l[] = { "help", 0 };
    const char x1s[] = "F";      const char* x1l[] = { "aclfilename", 0 };
    const char x2s[] = "f";      const char* x2l[] = { "fingerprint", 0 };
    const char x3s[] = "u";      const char* x3l[] = { "user", 0 };
    const char x4s[] = "i";      const char* x4l[] = { "psk-id", 0 };
    const char x5s[] = "k";      const char* x5l[] = { "psk-key", 0 };

    const struct { int k; int f; const char *s; const char*const* l; } opts[] = {
        { 'h', 0,           x0s, x0l },
        { 'F', GOPT_ARG,    x1s, x1l },
        { 'f', GOPT_ARG,    x2s, x2l },
        { 'u', GOPT_ARG,    x3s, x3l },
        { 'i', GOPT_ARG,    x4s, x4l },
        { 'k', GOPT_ARG,    x5s, x5l },
        { 0, 0, 0, 0 }
    };
    void *options = gopt_sort( & argc, (const char**)argv, opts);

    if( gopt( options, 'h')) {
        help(0, argv[0]);
        exit(0);
    }

    if (argc <= 1) return false;
    config->action = strdup(argv[1]);
    if (strcmp(config->action, "list") != 0 &&
        strcmp(config->action, "add") != 0 &&
        strcmp(config->action, "remove") != 0 &&
        strcmp(config->action, "set-psk") != 0) {
        return false;
    }

    if (!gopt_arg(options, 'F', &config->aclfilename)) {
        return false;
    }

    if (strcmp(config->action, "add") == 0 ||
        strcmp(config->action, "remove") == 0 ||
        strcmp(config->action, "set-psk") == 0) {
        if (!gopt_arg(options, 'f', &config->fingerprint)) {
            return false;
        }
    }

    if (strcmp(config->action, "add") == 0) {
        if (!gopt_arg(options, 'u', &config->user)) {
            return false;
        }
    }

    if (strcmp(config->action, "set-psk") == 0) {
        if (!gopt_arg(options, 'i', &config->pskId)) {
            return false;
        }
        if (!gopt_arg(options, 'k', &config->pskKey)) {
            return false;
        }
    }

    return true;
}

 
