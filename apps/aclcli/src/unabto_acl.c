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
#include "unabto/unabto_common_main.h"
#include "modules/fingerprint_acl/fp_acl_file.h"

struct configuration {
    const char *action;
    const char *aclfilename;
    const char *fingerprint;
    const char *user;
};

fp_acl_db_status fp_acl_file_remove_file(struct configuration* config, struct fp_acl_db* db);
fp_acl_db_status fp_acl_file_add_file(struct configuration* config, struct fp_acl_db* db);
fp_acl_db_status fp_acl_file_list_file(struct configuration* config, struct fp_acl_db* db);

bool parse_argv(int argc, char* argv[], struct configuration* config);
bool fp_get_fingerprint(const char *, fingerprint fpLocal);

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
    printf("       %s remove -F <acl filename> -f <fingerprint>\n\n", progname);
}


int main(int argc, char* argv[]) {
    struct configuration config;
    memset(&config, 0, sizeof(struct configuration));

    if (!parse_argv(argc, argv, &config)) {
        help(0, argv[0]);
        return 1;
    }

    struct fp_mem_state acl;

    struct fp_acl_settings defaultSettings;
    defaultSettings.systemPermissions = FP_ACL_SYSTEM_PERMISSION_ALL;
    defaultSettings.defaultUserPermissions = FP_ACL_PERMISSION_ALL;


    // Test file existence
    FILE* aclFile = fopen(config.aclfilename, "rb+");
    if (aclFile == NULL) {
        NABTO_LOG_ERROR(("Could not load aclfile %s", config.aclfilename));
        return FP_ACL_DB_LOAD_FAILED;
    }
    fclose(aclFile);


    // initialise the acl database
    struct fp_acl_db db;
    struct fp_mem_persistence p;

    if (fp_acl_file_init(config.aclfilename, "tmp.bin", &p) != FP_ACL_DB_OK) {
        NABTO_LOG_ERROR(("cannot init: fp_acl_file_init:%s", config.aclfilename));
        return FP_ACL_DB_FAILED;
    }
        
    if (fp_mem_init(&db, &defaultSettings, &p) != FP_ACL_DB_OK) {
        NABTO_LOG_ERROR(("cannot init: fp_mem_init"));
        return FP_ACL_DB_FAILED;
    }

    if (!strcmp(config.action, "add")) {
        if (fp_acl_file_add_file(&config, &db) != FP_ACL_DB_OK)
            printf("Add Failed\n");
    }
    
    if (!strcmp(config.action, "list")) {
        if (fp_acl_file_list_file(&config, &db) != FP_ACL_DB_OK)
            printf("List Failed\n");
    }

    if (!strcmp(config.action, "remove")) {
        if (fp_acl_file_remove_file(&config, &db) != FP_ACL_DB_OK)
            printf("Remove Failed\n");
    }

    exit(0);
}

// listing of ACL file
fp_acl_db_status fp_acl_file_list_file(struct configuration* config, struct fp_acl_db* db)
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

        if(db->load(iterator, &user) != FP_ACL_DB_OK) {
            printf("Could not load user %d", numUsers);
        } else {
            int j;
            for (j=0; j<FP_ACL_FP_LENGTH;j++) {
                if (j) printf(":");
                printf("%02x", user.fp[j]);
            }
      
            printf("  %04x:%04x  %s\n", splithex(user.permissions) , user.name);
        }
        iterator=db->next(iterator);

    }
  
    return FP_ACL_DB_OK;
}

// removing an entry from ACL file
fp_acl_db_status fp_acl_file_remove_file(struct configuration* config, struct fp_acl_db* db)
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

  
    if (fp_get_fingerprint(config->fingerprint, user.fp) != 1) {
        NABTO_LOG_ERROR(("Invalid Fingerprint\n"));
        return FP_ACL_DB_LOAD_FAILED;
    }


    it = db->find(user.fp);
  
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
fp_acl_db_status fp_acl_file_add_file(struct configuration* config, struct fp_acl_db* db)
{
  
    fp_acl_db_status status;

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

  
    if (fp_get_fingerprint(config->fingerprint, user.fp) != 1) {
        NABTO_LOG_ERROR(("Invalid Fingerprint\n"));
        return FP_ACL_DB_LOAD_FAILED;
    }


    memcpy(user.name, config->user, FP_ACL_USERNAME_MAX_LENGTH);

    db->save(&user);

    return FP_ACL_DB_OK;

}

bool fp_get_fingerprint(const char *fpargv, fingerprint fpLocal)
{
    uint32_t i,j = 0;

    i=0;j=0; bool last;

    if(strlen(fpargv)!=FP_ACL_FP_LENGTH*3-1) 
        return false;

    for (j=0; j<FP_ACL_FP_LENGTH;j++) {
        last = (j == FP_ACL_FP_LENGTH-1);

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
            sscanf(fpargv+i, "%2hhx:", &fpLocal[j]);
        } else {
            sscanf(fpargv+i, "%2hhx", &fpLocal[j]);
        }
        i+=3;

    }

    //for (j=0; j<FP_ACL_FP_LENGTH;j++) 
    //  printf("%02x ", fpLocal[j]);
    return true;
}

bool parse_argv(int argc, char* argv[], struct configuration* config) 
{
    const char x0s[] = "h?";     const char* x0l[] = { "help", 0 };
    const char x1s[] = "F";      const char* x1l[] = { "aclfilename", 0 };
    const char x2s[] = "f";      const char* x2l[] = { "fingerprint", 0 };
    const char x3s[] = "u";      const char* x3l[] = { "user", 0 };

    const struct { int k; int f; const char *s; const char*const* l; } opts[] = {
        { 'h', 0,           x0s, x0l },
        { 'F', GOPT_ARG,    x1s, x1l },
        { 'f', GOPT_ARG,    x2s, x2l },
        { 'u', GOPT_ARG,    x3s, x3l },
        { 0, 0, 0, 0 }
    };
    void *options = gopt_sort( & argc, (const char**)argv, opts);

    if( gopt( options, 'h')) {
        help(0, argv[0]);
        exit(0);
    }

    if (argc <= 1) return false;
    config->action = strdup(argv[1]);
    if (strcmp(config->action, "list") && strcmp(config->action, "add") && strcmp(config->action, "remove")) {
        return false;
    }

    if (!gopt_arg(options, 'F', &config->aclfilename)) {
        return false;
    }

    if (!strcmp(config->action, "add") || !strcmp(config->action, "remove")) {
        if (!gopt_arg(options, 'f', &config->fingerprint)) {
            return false;
        }
    }

    if (!strcmp(config->action, "add")) {
        if (!gopt_arg(options, 'u', &config->user)) {
            return false;
        }
    }

    return true;
}

 
