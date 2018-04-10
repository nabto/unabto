# This CLI is used to list or alter the contents of the ACL file

## To compile:

	mkdir build
	cmake ..
	make

## To execute:

	unabto_acl <[list]/[add]/[remove]> -F <acl filename> [-f <fingerprint>] [-u <user>]
	Example: unabto_acl list -F persistence.bin
	         unabto_acl add -F persistence.bin -f a1:0e:...:6f -u mytest
	         unabto_acl remove -F persistence.bin -f a1:0e:...:6f

        options:

            -F <acl filename>: name of the acl binary file 
            -f <figerprint>  : 16 byte device fingerprint as hex string (each byte seperated by ":")
            -u <user>        : name of the device

## list:

	The "list" action will be read the acl file and list down the contents of acl file

	>> unabto_acl list -F persistence.bin

	    System Permissions       : e000:0000
            Default User Permissions : c000:0000
            First User Permissions   : e000:0000
            Number of users = 2
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:66  e000:0000  Jhon
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4d  c000:0000  James

        The first column in the above ouput is the 16byte device fingerprint, the second column is acl bits 
        and the third column is the name of the user.

## add:

	The "add" action will append the new users to the acl file. The device fingerprint (16 byte hex string)
        and user name must be passed as arguments

	>> unabto_acl add -F persistence.bin -f 79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4f -u Mark

            Successfully added
        
        Note: Local access & Remote access will be granted to the new user added. 
              If the user added is the first user, then admin access will be granted.

	>> unabto_acl list -F persistence.bin

            System Permissions       : e000:0000
            Default User Permissions : c000:0000
            First User Permissions   : e000:0000
            Number of users = 3
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:66  e000:0000  Jhon
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4d  c000:0000  James
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4f  c000:0000  Mark

## remove:

	The "remove" action will delete the specific device fingerprint from the acl file.  The device
        fingerpring (16 byte hex string) to be passes as argument

	>> unabto_acl remove -F persistence.bin -f 79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4d

            Successfully removed

	>> unabto_acl list -F persistence.bin

            System Permissions       : e000:0000
            Default User Permissions : c000:0000
            First User Permissions   : e000:0000
            Number of users = 2
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:66  e000:0000  Jhon
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4f  c000:0000  Mark
