# Nabto ACL Cli Tool

This CLI is used to list or alter the contents of the [uNabto Fingerprint ACL](https://github.com/nabto/unabto/tree/master/src/modules/fingerprint_acl) file.

## Compiling

	mkdir build
	cmake ..
	make

## Running

	aclcli <[list]/[add]/[remove]/[set-psk]> -F <acl filename> [-f <fingerprint>] [-u <user>]
                                                         [-i <psk id>] [-k <psk>]
	Example: aclcli list -F persistence.bin
	         aclcli add -F persistence.bin -f a1:0e:...:6f -u mytest
	         aclcli remove -F persistence.bin -f a1:0e:...:6f
                 aclcli set-psk -F persistence.bin -f a1:0e:...:6f -i 01:ee:...:fb -k 12:...:aa:bb

        options:

            -F <acl filename>: name of the acl binary file 
            -f <fingerprint> : 16 byte device fingerprint as hex string (each byte seperated by ":")
            -u <user>        : name of the device
            -i <psk_id>      : pre-shared key id 
            -k <psk>         : pre-shared key 

## `list` action

The "list" action will read the acl file and list the contents.

	$ aclcli list -F persistence.bin

	    System Permissions       : e000:0000
            Default User Permissions : c000:0000
            First User Permissions   : e000:0000
            Number of users = 2
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:66  e000:0000  John
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4d  c000:0000  James

The first column in the above ouput is the 16byte device fingerprint, the second column is the permission bits and the third column is the name of the user.

## `add` action

The "add" action will append a new user to the acl file. The device fingerprint (16 byte hex string) and user display name must be passed as arguments.

	$ aclcli add -F persistence.bin -f 79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4f -u Mark

            Successfully added
        
Note: Local access and remote access will be granted to the new user added. If the user added is the first user, admin access will also be granted.

	$ aclcli list -F persistence.bin

            System Permissions       : e000:0000
            Default User Permissions : c000:0000
            First User Permissions   : e000:0000
            Number of users = 3
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:66  e000:0000  Jhon
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4d  c000:0000  James
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4f  c000:0000  Mark

## `remove` action

The "remove" action will delete the specific fingerprint from the acl file. 

	$ aclcli remove -F persistence.bin -f 79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4d

            Successfully removed

	$ aclcli list -F persistence.bin

            System Permissions       : e000:0000
            Default User Permissions : c000:0000
            First User Permissions   : e000:0000
            Number of users = 2
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:66  e000:0000  John
            79:83:c7:c8:bf:2c:64:ed:80:e0:3d:00:1d:d3:af:4f  c000:0000  Mark

## `set-psk` action

This action sets a local pre-shared key for a given entry in the ACL. This allows the client to later establish a secure local connection using the same id and key specified through the Nabto Client SDK (version 4.2 and later) using `nabtoSetLocalConnectionPsk`.

Example:

```
$ ./aclcli add -F persistence.bin -f f4:ee:dd:ee:aa:bb:ee:bc:cd:36:2a:8b:aa:bb:4a:fb -u jdoe
File persistence2.bin does not exist, creating new

$ ./aclcli list -F persistence.bin 
System Permissions       : e000:0000 
Default User Permissions : c000:0000 
First User Permissions   : e000:0000 
Number of users = 1
f4:ee:dd:ee:aa:bb:ee:bc:cd:36:2a:8b:aa:bb:4a:fb  e000:0000  jdoe

$ ./aclcli set-psk -F persistence.bin -f f4:ee:dd:ee:aa:bb:ee:bc:cd:36:2a:8b:aa:bb:4a:fb -i 01:ee:dd:ee:aa:bb:ee:bc:cd:36:2a:8b:aa:bb:4a:fb -k 12:ee:dd:ee:aa:bb:ee:bc:cd:36:2a:8b:aa:bb:4a:fb

$ ./aclcli list -F persistence.bin 
System Permissions       : e000:0000 
Default User Permissions : c000:0000 
First User Permissions   : e000:0000 
Number of users = 1
f4:ee:dd:ee:aa:bb:ee:bc:cd:36:2a:8b:aa:bb:4a:fb  e000:0000  jdoe
  PSK: [01:ee:dd:ee:aa:bb:ee:bc:cd:36:2a:8b:aa:bb:4a:fb] => [12:ee:dd:ee:aa:bb:ee:bc:cd:36:2a:8b:aa:bb:4a:fb]
```


       