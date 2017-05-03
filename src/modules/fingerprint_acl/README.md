# Paired public key access control module

This module can be used for paired public key fingerprint authentication of devices: A fingerprint of a client's self-signed public key is first exchanged with the device in a trusted setting - the pairing step. Subsequently, the Nabto framework securely passes the client's public key fingerprint to the device application when the client want to obtain remote access to the device. Please see section 8.2 in [TEN036 "Security in Nabto Solutions"](https://www.nabto.com/downloads/docs/TEN036%20Security%20in%20Nabto%20Solutions.pdf) for more details.

The module needs to be initialized with a database backend. The
database backend is used for storing system settings and user profiles.

## Data Model

The data model consists of system settings and user settings. 

On the system level there are 32bits which describes the access
permissions of the overall system. Further there are a uint32_t which
descibes the default permissions new users should get and 32 bits
which defines what permissions the first user of the system should
get. In most cases the first user which pairs with the device should
get more access than the next users.

```
struct fp_acl_settings {
    uint32_t systemPermissions;       ///< permission bits controlling the system
    uint32_t defaultUserPermissions;  ///< default permissions for new users
    uint32_t firstUserPermissions;    ///< permissions for the first user
};
```

The user have the following datamodel
```
struct fp_acl_user {
  uint8_t fingerprint[16]
  char userName[64]
  uint32_t permissions
}
```

A user is uniquely identified by his fingerprint, which is used as a
primary key for all operations which manipulate a specific user.

## Operating Mode

The device can either be open for local pairing or closed for local
pairing, if it's open for local pairing then it's possible to pair
with the device if it is closed for local pairing it is not possible
to pair with the device. When a user pairs with the device the
permission granted is controlled by the admin of the device.

# Example use cases for this module

The following is a high level description of how this modules is used
in very specific use cases.

## Unpaired fresh mode

The device is fresh from the package or factory reset. Meaning it's
user database is completely empty.

1. Client connects locally. Connection access is granted since device is unpaired.
2. Client calls getPublicInfo.json to find out if you are paired with the device.
3. Client goes into pairing mode and calls pairWithDevice.json.
4. Client is granted owner permissions to the device { controlled by firstUserPermissions}


## Paired device local pairing access

The device has at least one user in it's user database and 

1. Client connects locally. Connection access is granted since the connection is local. (Controlled by systemPermissions)
2. Client calls getPublicInfo.json to find out that it is not paired.
3. Client goes into pairing mode and calls pairWithDevice.json
4. Client is granted guest permissions since the device already have an owner. (controlled by defaultUserPermissions)

## Access device you are paired with

1. Client connects to the device.
2. Client calls getPublicInfo.json to ensure that the client is paired with the device.
3. Start using the device ...

## Remote Access to a device where you have been removed from the ACL
1. The device is in the list of known devices
2. Client connects to the device. The connection is not granted and fails with ACCESS_DENIED
3. The user is informed about the situation and asked to re-pair with the device.

## Local Access to a device where you have been removed from the ACL
1. the device is in the list of known devices
2. Client connects to the device, the connection is granted because the device is in local pairing mode.
3. Client calls getPublicInfo.json and discovers that it is not seen as paired.
4. Client goes into pairing mode and calls pairWithDevice.json

## How to use this module

Include the source into your project.
  * Add the functions in the file ```fp_acl_ae.h``` to your ```application_event``` handler.
  * For each call in your other ```application_event``` handler call
    ```fp_acl_is_request_allowed``` to check that the request is
    allowed.
  * before calling the acl functions be sure to call ```fp_acl_ae_init``` to initialize the acl module.


# User database

It is designed such that it should be easy to crate new user database data
backend. The acl module only depends on the ```struct fp_acl_db```
interface. Currently there one implementation of that interface in
```fp_acl_memory.h``` which is a memory backed user database with
optional persistence. Currently ```fp_acl_file.h``` implements a file
persistence layer for the memory backed user database.

## How to test this module

This module is automatic tested with the test code in
test/modules/fingerprint_acl, this test code is part of the unabto
unittest.

## Test Level

This code is tested with each release.
