## Fingerprint acl module.

This module can be used for fingerprint authentication of devices.

The module needs to be initialized with a database backend. The
database backend is used for storing system settings and user profiles.

# Data Model

The data model consists of system settings and user settings. 

On the system level there are 32bits which describes the access
permissions of the overall system. Further there are a uint32_t which
descibes the default permissions new users should get.

```
struct fp_acl_settings {
    uint32_t systemPermissions;   ///< permission bits controlling the system
    uint32_t defaultPermissions;  ///< default permissions for new users
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

## Example use cases for this module

The following is a high level description of how this modules is used
in very specific use cases.

# Unpaired fresh mode

The device is fresh from the package or factory reset. Meaning it's
user database is completely empty.

1. Client connects locally. Connection access is granted since device is unpaired.
2. Client calls getPublicInfo.json to find out if you are paired with the device.
3. Client goes into pairing mode and calls pairWithDevice.json.
4. Client is granted owner permissions to the device


# paired device local pairing access

The device has atleast one user in it's user database and 

1. Client connects locally. Connection access is granted since the connection is local. (Controlled by systemPermissions)
2. Client calls getPublicInfo.json to find out that it is not paired.
3. Client goes into pairing mode and calls pairWithDevice.json
4. Client is granted guest permissions since the device already have an owner. (controlled by defaultPermissions)

# access device you are paired with

1. Client connects to the device.
2. Client calls getPublicInfo.json to ensure that the client is paired with the device.
3. Start using the device ...

# Remote Access to a device where you have been removed from the ACL
1. The device is in the list of known devices
2. Client connects to the device. The connection is not granted and fails with ACCESS_DENIED
3. The user is informed about the situation and asked to re-pair with the device.

# Local Access to a device where you have been removed from the ACL
1. the device is in the list of known devices
2. Client connects to the device, the connection is granted because the device is in local pairing mode.
3. Client calls getPublicInfo.json and discovers that it is not seen as paired.
4. Client goes into pairing mode and calls pairWithDevice.json

## How to use this module.

Include the source into your project.
  * Add the functions in the file fp_acl_ae.h to your application_event handler.
  * For each call in your other application_event handler call
    ```fp_acl_is_request_allowed``` to check that the request is
    allowed.
  * before calling the acl functions be sure to call ```fp_acl_ae_init``` to initialize the acl module.

## How to test this module.

This module is automatic tested with the test code en test/modules/fingerprint_acl

## Test Level

This code is testet.
