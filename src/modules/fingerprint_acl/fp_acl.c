#include "fp_acl.h"

fp_acl_db aclDb;

void fp_acl_initialize(struct fp_acl_db* db)
{
    aclDb = *db;
}

