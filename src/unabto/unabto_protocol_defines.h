/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto protocol defintions - micro nabto specific.
 */

#ifndef _UNABTO_PROTOCOL_DEFINES_H_
#define _UNABTO_PROTOCOL_DEFINES_H_

/*********************** WARNING WARNING WARNING ******************************
** This file is included in both the micro world and the big nabto world.    **
** That is the content of this file is used by all compilers for the micro   **
** devices and the compilers for the base station and client software.       **
** Hence this file should only contain definitions for the protocol between  **
** the nabto micro world and the clients.                                    **
** The definitions should mainly consist of defines for the C preprocessor.  **
** Avoid using structs/unions or advanced macros that deals with the memory  **
** layouts of the different platforms.                                       **
******************************************************************************/

#define NP_VERSION         2
#define NP_VERSION_MIN     2
#define NP_VERSION_MAX     2
#define NP_STREAM_VERSION  1

/*****************************************************************************/
/**********************  PACKET   ********************************************/
/*****************************************************************************/

/* Packet header */
/* The packet header (16, 18, 24 or 26 bytes) has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  NSI.cp     id of client peer                                  |
*      +-----+----------------------------------------------------------------+
*      |  +4 |  NSI.sp     id of gsp (server peer)                            |
*      +-----+----------------------------------------------------------------+
*      |  +8 |  Type       (see NP_PACKET_HDR_TYPE_*)                         |
*      +-----+----------------------------------------------------------------+
*      |  +9 |  Version    protocol version number (NP_VERSION)               |
*      +-----+----------------------------------------------------------------+
*      | +10 |  Retrans    retransmission count (only 0-7 is allowed)         |
*      +-----+----------------------------------------------------------------+
*      | +11 |  Flags      (see NP_PACKET_HDR_FLAG_*)                         |
*      +-----+----------------------------------------------------------------+
*      | +12 |  Sequence   sequence number of packet                          |
*      +-----+----------------------------------------------------------------+
*      | +14 |  Length     number of bytes in packet including this header    |
*      +-----+----------------------------------------------------------------+
*      | +16 |  NSI.co     only if NP_PACKET_HDR_FLAG_NSI_CO is set in Flags  |
*      +-----+----------------------------------------------------------------+
*      | +24 |  Tag        only if NP_PACKET_HDR_FLAG_TAG is set in Flags     |
*      +-----+----------------------------------------------------------------+
*      NSI.cp must be >= 100 to be a valid packet.
* After the packet header one or more payloads will be present. The total
* number of bytes in the payloads are (Length - size of header) bytes.
*/
#define NP_PACKET_HDR_MIN_BYTELENGTH 16  ///< the size of the fixed size header
#define NP_PACKET_HDR_MAX_BYTELENGTH 26  ///< the max size of a header (fixed + nsi.co + tag)
#define NP_PACKET_HDR_MIN_NSI_CP    100  ///< the min value of a client peer nsi

/* Packet header types */
#define NP_PACKET_HDR_TYPE_SP_ATTACH     1
#define NP_PACKET_HDR_TYPE_SP_AUTH       2
#define NP_PACKET_HDR_TYPE_SP_ALIVE      3
#define NP_PACKET_HDR_TYPE_SP_DETACH     4

#define NP_PACKET_HDR_TYPE_CP_REQ_SP     11
#define NP_PACKET_HDR_TYPE_CP_CONN_SP    12
#define NP_PACKET_HDR_TYPE_CP_DISC_SP    13

#define NP_PACKET_HDR_TYPE_GW_CONN       21 ///< fallback connect request
#define NP_PACKET_HDR_TYPE_DATA          22
#define NP_PACKET_HDR_TYPE_GW_CONN_U     23 ///< fallback connect request (unencrypted)

#define NP_PACKET_HDR_TYPE_PROBE         30 ///< Probe packet to test for udp reachability.
#define NP_PACKET_HDR_TYPE_STATS         31 ///< Statistics information packet.
#define NP_PACKET_HDR_TYPE_PING          32 ///< Ping packet.

#define NP_PACKET_HDR_TYPE_U_INVITE      128 ///< invite request send from bs
#define NP_PACKET_HDR_TYPE_U_ATTACH      129 ///< attach request send from gsp
#define NP_PACKET_HDR_TYPE_U_ALIVE       130 ///< alive query send from gsp
#define NP_PACKET_HDR_TYPE_U_CONNECT     131 ///< connect request send from client or gsp
#define NP_PACKET_HDR_TYPE_U_DEBUG       132 ///< 
#define NP_PACKET_HDR_TYPE_U_PUSH        133 ///< Push notification
#define NP_PACKET_HDR_TYPE_U_VERIFY_PSK  134 ///< Verify a connect request
#define NP_PACKET_HDR_TYPE_U_CONNECT_PSK 135 ///< PSK based connect request

/* Values below are used in the standard Nabto protocol. */
/* Values above are used in the Micro Device protocol. */
#define NP_PACKET_HDR_TYPE_U_LIMIT    128 ///< Packets in the micro protocol should have values greater (or equal) to this.

/* Packet header flags */
#define NP_PACKET_HDR_FLAG_NONE       0x00 ///< No flags.
#define NP_PACKET_HDR_FLAG_RESPONSE   0x01 ///< Response flag. Set only in responses.
#define NP_PACKET_HDR_FLAG_EXCEPTION  0x02 ///< Exception flag, signals that the packet contains non standard data.
#define NP_PACKET_HDR_FLAG_TAG        0x40 ///< TAG flag. Set only when the header contains a tag.
#define NP_PACKET_HDR_FLAG_NSI_CO     0x80 ///< NSI flag. Set only when the NSI Controller part is used.

/* Packet header tags */
#define NP_PACKET_HDR_TAG_DEFAULT      0x0000  ///< No Tag (used for ordinary data).
#define NP_PACKET_HDR_TAG_CONN_CTRL    0x0001  ///< Connection Control - for use by application.
#define NP_PACKET_HDR_TAG_SFTP         0x0002  ///< Simple File Transfer.
#define NP_PACKET_HDR_TAG_FRAMING_CTRL 0x0003  ///< Framing Control (keep alive data etc).
#define NP_PACKET_HDR_TAG_STREAM_MIN   0x1000  ///< Minimum tag for stream usage
#define NP_PACKET_HDR_TAG_STREAM_MAX   0x1fff  ///< Maximum tag for stream usage
#define NP_PACKET_HDR_TAG_USER_MIN     0xf000  ///< Minimum tag for application usage
#define NP_PACKET_HDR_TAG_USER_MAX     0xffff  ///< Maximum tag for application usage

#define NP_PACKET_HDR_TAG_IS_USER(tag) (((tag) >= NP_PACKET_HDR_TAG_USER_MIN) && ((tag) <= NP_PACKET_HDR_TAG_USER_MAX))
#define NP_PACKET_HDR_TAG_IS_STREAM(tag) (((tag) >= NP_PACKET_HDR_TAG_STREAM_MIN) && ((tag) <= NP_PACKET_HDR_TAG_STREAM_MAX))

/*****************************************************************************/
/**********************  PAYLOADS   ******************************************/
/*****************************************************************************/

/* Payload header */
/* The payload header (4 bytes) has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Type     (see enum np_payload_type_e)                         |
*      +-----+----------------------------------------------------------------+
*      |  +1 |  Flags    (see NP_PAYLOAD_HDR_FLAG_*)                          |
*      +-----+----------------------------------------------------------------+
*      |  +2 |  Length   number of bytes in payload including this header     |
*      +-----+----------------------------------------------------------------+
* After the payload header, the payload data is coming. A total of
* (Length - NP_PAYLOAD_HDR_BYTELENGTH) bytes of data should arrive.
*/
#define NP_PAYLOAD_HDR_BYTELENGTH   4     ///< the size of a payload header

/* Payload header types */
enum np_payload_type_e {
    NP_PAYLOAD_TYPE_CERT             = 0x30, /* '0' Certificate           */
    NP_PAYLOAD_TYPE_SP_ID            = 0x31, /* '1' ServerPeer ID         */
    NP_PAYLOAD_TYPE_NONCE            = 0x32, /* '2' Nonce                 */
    NP_PAYLOAD_TYPE_NONCE_ID         = 0x33, /* '3' Nonce with ID         */
    NP_PAYLOAD_TYPE_NOTIFY           = 0x34, /* '4' Notification          */
    NP_PAYLOAD_TYPE_IPX              = 0x35, /* '5' IP eXtended data      */
    NP_PAYLOAD_TYPE_CRYPTO           = 0x36, /* '6' Cryptographic         */
    NP_PAYLOAD_TYPE_VERIFY           = 0x37, /* '7' Verification          */
    NP_PAYLOAD_TYPE_GW               = 0x38, /* '8' Gateway               */
    NP_PAYLOAD_TYPE_EP               = 0x39, /* '9' Endpoint              */
    NP_PAYLOAD_TYPE_VERSION          = 0x3A, /* ':' Version               */
    NP_PAYLOAD_TYPE_CAPABILITY       = 0x3B, /* ';' Capabilities          */
    NP_PAYLOAD_TYPE_PIGGY            = 0x3C, /* '<' Piggyback             */
    NP_PAYLOAD_TYPE_WINDOW           = 0x3D, /* '=' Window data           */
    NP_PAYLOAD_TYPE_TIME             = 0x3E, /* '>' Time data             */
    NP_PAYLOAD_TYPE_CP_ID            = 0x3F, /* '?' ClientPeer ID         */
    NP_PAYLOAD_TYPE_DESCR            = 0x40, /* '@' Description (version) */
    NP_PAYLOAD_TYPE_SACK             = 0x41, /* 'A' Selective acknowledge */
    NP_PAYLOAD_TYPE_STATS            = 0x42, /* 'B' Statistics payload    */
    NP_PAYLOAD_TYPE_CONNECT_STATS    = 0x43, /* 'C' Connection statistics */
    NP_PAYLOAD_TYPE_RENDEZVOUS_STATS = 0x44, /* 'D' Rendezvous statistics */
    NP_PAYLOAD_TYPE_CONNECTION_STATS = 0x45, /* 'E' Connection statistics */
    NP_PAYLOAD_TYPE_ATTACH_STATS     = 0x46, /* 'F' Connection statistics */
    NP_PAYLOAD_TYPE_PING             = 0x47, /* 'G' Ping                  */
    NP_PAYLOAD_TYPE_SYSLOG_CONFIG    = 0x48, /* 'H' Syslog Configuration  */
    NP_PAYLOAD_TYPE_GWWS             = 0x49, /* 'I' websocket gateway payload          */
    NP_PAYLOAD_TYPE_SYSTEM_INFO      = 0x4A, /* 'J' system info payload type           */
    NP_PAYLOAD_TYPE_FP               = 0x4B, /* 'K' fingerprint of certificate payload */
    NP_PAYLOAD_TYPE_PUSH             = 0x4C, /* 'L' Push notification payload          */
    NP_PAYLOAD_TYPE_PUSH_DATA        = 0x4D, /* 'M' Push notification data payload     */
    NP_PAYLOAD_TYPE_STREAM_STATS     = 0x4E, /* 'N' Stream statistics */
    NP_PAYLOAD_TYPE_BASESTATION_AUTH = 0x4F, /* 'O' Basestation Auth key value pairs */
    NP_PAYLOAD_TYPE_CONNECTION_INFO  = 0x50, /* 'P' Connection Info payload */
    NP_PAYLOAD_TYPE_RANDOM           = 0x51, /* 'Q' Random data used in connection handshake */
    NP_PAYLOAD_TYPE_KEY_ID           = 0x52  /* 'R' Key id used in connection handshake */
};

/* Payload header flags */
#define NP_PAYLOAD_HDR_FLAG_NONE       0x00
#define NP_PAYLOAD_HDR_FLAG_OPTIONAL   0x80

/*****************************************************************************/
/* SP_ID payload */
/* The server peer id payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Type, one of NP_PAYLOAD_SP_ID_TYPE_*                    |
*      +-----+-----+----------------------------------------------------------+
*      |  +5 |  +1 |  sp id as a string, non null terminated                  |
*      +-----+-----+----------------------------------------------------------+
* The serverpeer ID follows right after the Type field. The total number of
* bytes in the ID are (payload Length - NP_PAYLOAD_SP_ID_BYTELENGTH) bytes.
*/

#define NP_PAYLOAD_SP_ID_BYTELENGTH          5  ///< size of a SP_ID payload without id

/* Type of serverpeer ID */
#define NP_PAYLOAD_SP_ID_TYPE_MAIL 0x01
#define NP_PAYLOAD_SP_ID_TYPE_URL  0x02


/*****************************************************************************/
/* CP_ID payload */
/* The client peer id payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Type, one of NP_PAYLOAD_CP_ID_TYPE_*                    |
*      +-----+-----+----------------------------------------------------------+
*      |  +5 |  +1 |  cp id as a string, non null terminated                  |
*      +-----+-----+----------------------------------------------------------+
* The serverpeer ID follows right after the Type field. The total number of
* bytes in the ID are (payload Length - NP_PAYLOAD_SP_ID_BYTELENGTH) bytes.
*/

#define NP_PAYLOAD_CP_ID_BYTELENGTH          5  ///< size of a CP_ID payload without id

/* Type of clientpeer ID */
#define NP_PAYLOAD_CP_ID_TYPE_MAIL                0x01
#define NP_PAYLOAD_CP_ID_TYPE_URL                 0x02

/*****************************************************************************/
/* FINGERPRINT payload
 * The fingerprint payload data has the following layout:
 *      +-----+----------------------------------------------------------------+
 *      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
 *      +-----+-----+----------------------------------------------------------+
 *      |  +1 |  +0 |  fingerprint type.                                       |
 *      +-----+-----+----------------------------------------------------------+
 *      |  +5 |  +1 |  fingerprint as a sequence of bytes.                     |
 *      +-----+-----+----------------------------------------------------------+
 * The serverpeer ID follows right after the Type field. The total number of
 * bytes in the ID are (payload Length - NP_PAYLOAD_SP_ID_BYTELENGTH) bytes.
 */

#define NP_PAYLOAD_FP_TYPE_SHA256_TRUNCATED        0x01
#define NP_TRUNCATED_SHA256_LENGTH_BYTES           16


/*****************************************************************************/
/* VERSION payload */
/* The version payload data has the following layout:
 * 
 * The payload has been created such that the string in the end of the
 * payload can contain semver prerelease info and build version info
 *
 * The extra infor needs to be formatted as
 * -prerelease.info.xyz+build.info.xyz both can be omitted then it
 * will be an empty string.
 *      +-----+----------------------------------------------------------------+
 *      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
 *      +-----+-----+----------------------------------------------------------+
 *      |  +4 |  +0 |  Type, one of NP_PAYLOAD_VERSION_TYPE_*                  |
 *      +-----+-----+----------------------------------------------------------+
 *      |  +6 |  +2 |  Major version number                                    |
 *      +-----+-----+----------------------------------------------------------+
 *      | +10 |  +6 |  Minor version number                                    |
 *      +-----+-----+----------------------------------------------------------+
 *      | +14 | +10 |  Patch version number (optional)                         |
 *      +-----+----------------------------------------------------------------+
 *      | +18 | +14 |  The rest of the payload is a label string e.g. pre.0    |
 *      |     |     |  (optional) if present patch is also needed.             |
 *      +-----+-----+----------------------------------------------------------+
 */

#define NP_PAYLOAD_VERSION_BYTELENGTH              14  ///< Minimum size of a VERSION payload
#define NP_PAYLOAD_VERSION_BYTELENGTH_PATCH        18  ///< Size with patch 


/* Type of version */
#define NP_PAYLOAD_VERSION_TYPE_UNDEF 0x0000 // The object type is not defined.
#define NP_PAYLOAD_VERSION_TYPE_CP    0x0001 // Clientpeer version
#define NP_PAYLOAD_VERSION_TYPE_SP    0x0002 // Serverpeer version
#define NP_PAYLOAD_VERSION_TYPE_CO    0x0003 // Controller version
#define NP_PAYLOAD_VERSION_TYPE_GW    0x0004 // Gateway version
#define NP_PAYLOAD_VERSION_TYPE_UD    0x0005 // Micro device version

/*****************************************************************************/
/* CRYPTO payload */
/* The CRYPTO payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Crypto code                                             |
*      +-----+-----+----------------------------------------------------------+
*      |  +6 |  +2 |  One or more payloads or data depending on the crypto    |
       |     |     |  code                                                    |
*      +-----+-----+----------------------------------------------------------+
* After the crypto code one or more payloads will be present. The total number
* of bytes in the payloads are (payload Length - NP_PAYLOAD_CRYPTO_BYTELENGTH)
* bytes.
*/

#define NP_PAYLOAD_CRYPTO_BYTELENGTH   6  ///< size of a CRYPTO payload without data

/* Bit values and bit masks for crypto code */
/* -- operations -- (4 values) */
#define NP_PAYLOAD_CRYPTO_OPER_NONE           0x0000
#define NP_PAYLOAD_CRYPTO_OPER_SIGN           0x0001
#define NP_PAYLOAD_CRYPTO_OPER_CRYPT          0x0002
#define NP_PAYLOAD_CRYPTO_OPER_HASH           0x0003
#define NP_PAYLOAD_CRYPTO_CODEMASK_OPERATION  0x0003
#define NP_PAYLOAD_CRYPTO_CODESHIFT_OPERATION 0

/* -- encryption methods -- (2 bits) */
#define NP_PAYLOAD_CRYPTO_ENCR_NONE           0x0000
#define NP_PAYLOAD_CRYPTO_ENCR_CERT           0x0004
#define NP_PAYLOAD_CRYPTO_ENCR_SECR           0x0008

/* -- padding methods (used with CERT) -- (8 values) */
#define NP_PAYLOAD_CRYPTO_PADD_PKCS5          0x0000
#define NP_PAYLOAD_CRYPTO_PADD_PKCS1          0x0010
#define NP_PAYLOAD_CRYPTO_PADD_OAEP           0x0020
#define NP_PAYLOAD_CRYPTO_CODEMASK_PADDING    0x0070
#define NP_PAYLOAD_CRYPTO_CODESHIFT_PADDING   4

/* -- unused -- (1 bit) */
#define NP_PAYLOAD_CRYPTO_UNUSED              0x0080

/* -- hashing algorithms (used with HASH and with SECR+symm) -- (16 values) */
#define NP_PAYLOAD_CRYPTO_HASH_SUM            0x0000
#define NP_PAYLOAD_CRYPTO_HASH_SHA1           0x0100
#define NP_PAYLOAD_CRYPTO_HASH_HMAC_SHA1      0x0200
#define NP_PAYLOAD_CRYPTO_HASH_HMAC_SHA256    0x0300
#define NP_PAYLOAD_CRYPTO_CODEMASK_HASHING    0x0F00
#define NP_PAYLOAD_CRYPTO_CODESHIFT_HASHING   8

/* -- symmetric algorithms (used with SECR+hash) -- (16 values) */
#define NP_PAYLOAD_CRYPTO_SYMM_BF             0x1000
#define NP_PAYLOAD_CRYPTO_SYMM_AES_CBC        0x2000
#define NP_PAYLOAD_CRYPTO_CODEMASK_SYM_ALG    0xF000
#define NP_PAYLOAD_CRYPTO_CODESHIFT_SYM_ALG   12

/* -- Crypto payload flags */

/**
 * Change the default payload behavior. If the encryption method is
 * NP_PAYLOAD_CRYPTO_ENCR_SECR then the default behavior is that the
 * crypto payload does not contain payloads, but raw data. For
 * NP_PAYLOAD_CRYPTO_ENCR_NONE and NP_PAYLOAD_CRYPTO_ENCR_CERT the
 * default behavior is that the crypto payloads does contain
 * payloads. This flag will change the default behavior and make data
 * in the crypto payload be handled as payloads instead of raw data.
 */
#define NP_PAYLOAD_CRYPTO_HEADER_FLAG_PAYLOADS 0x01


/*****************************************************************************/
/* IPX payload */
/* The IPX payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Private IPv4 address    4 bytes in network byte order   |
*      +-----+-----+----------------------------------------------------------+
*      |  +8 |  +4 |  Private IP port         2 bytes in network byte order   |
*      +-----+-----+----------------------------------------------------------+
*      | +10 |  +6 |  Global IPv4 address     4 bytes in network byte order   |
*      +-----+-----+----------------------------------------------------------+
*      | +14 | +10 |  Global IP port          2 bytes in network byte order   |
*      +-----+-----+----------------------------------------------------------+
*      | +16 | +12 |  Flags                   (see NP_PAYLOAD_IPX_FLAG_*)     |
*      +-----+-----+----------------------------------------------------------+
*      | +17 | +13 |  NSI                     Optional serverpeer NSI.        |
*      +-----+-----+----------------------------------------------------------+
*      | +21 | +17 |  NSI                     Optional controller NSI.        |
*      +-----+-----+----------------------------------------------------------+
*      | +29 | +25 |  NSI                     Optional clientpeer NSI.        |
*      +-----+-----+----------------------------------------------------------+
* The only way to determine whether the NSI field is present or not is to
* consult the Length field of the payload header. If the Length is
* NP_PAYLOAD_IPX_NSI_BYTELENGTH the sp NSI field is present in the payload.
* If the Length is NP_PAYLOAD_IPX_FULL_NSI_BYTELENGTH all three NSI fields
* are present in the the payload.
*/
#define NP_PAYLOAD_IPX_BYTELENGTH           17  ///< size of a IPX payload without any NSI
#define NP_PAYLOAD_IPX_NSI_BYTELENGTH       21  ///< size of a IPX payload with sp NSI
#define NP_PAYLOAD_IPX_FULL_NSI_BYTELENGTH  33  ///< size of a IPX payload with all NSI

/* IPX payload flags */
#define NP_PAYLOAD_IPX_FLAG_NO_RENDEZVOUS 0x80  ///< disable rendez-vous
#define NP_PAYLOAD_IPX_FLAG_TRY_PRIVATE   0x40  ///< try private endpoint/address first this does not have any significance since we are trying private and public endpoints in parallel
#define NP_PAYLOAD_IPX_FLAG_CP_ASYNC      0x20  ///< client peer is able of communication asynchroniously

#define NP_PAYLOAD_IPX_NAT_MASK           0xf // the lowest 4 bits is reserved for the nat type.

/* IPX payload NAT types (old values for flags field) */
#define NP_PAYLOAD_IPX_NAT_UNKNOWN          0
#define NP_PAYLOAD_IPX_NAT_FAILURE          1
#define NP_PAYLOAD_IPX_NAT_OPEN             2
#define NP_PAYLOAD_IPX_NAT_BLOCKED          3
#define NP_PAYLOAD_IPX_NAT_CONE             4
#define NP_PAYLOAD_IPX_NAT_RESTRICTED       5
#define NP_PAYLOAD_IPX_NAT_PORT_RESTRICTED  6
#define NP_PAYLOAD_IPX_NAT_SYMMETRIC        7
#define NP_PAYLOAD_IPX_NAT_FIREWALL         8


/*****************************************************************************/
/* CAPABILITY payload */
/* The CAPABILITY payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Type                                                    |
*      +-----+-----+----------------------------------------------------------+
*      |  +5 |  +1 |  Flags       32 flag bits                                |
*      +-----+-----+----------------------------------------------------------+
*      |  +9 |  +5 |  Mask        32 mask bits                                |
*      +-----+-----+----------------------------------------------------------+
*      | +13 |  +9 |  Optional number of encryption codes uint16              |
*      +-----+-----+----------------------------------------------------------+
*      | +15 | +11 |  Encryption codes, 16bit each                            |
*      +-----+-----+----------------------------------------------------------+

* Only bits in the Flags field with corresponding bits set in the Mask fields are valid.
*/

#define NP_PAYLOAD_CAPABILITY_BYTELENGTH   13  ///< size of a CAPABILITY payload without encryption codes

/* Type of capabilities */
#define NP_PAYLOAD_CAPA_TYPE_NONE        0

/* Bit numbers of bits in Mask and Flags fields */
#define NP_PAYLOAD_CAPA_BIT_CAP_DIR         0  ///< peer is able to use Direct Connection
#define NP_PAYLOAD_CAPA_BIT_CAP_FB_TCP      1  ///< peer is able to use FB
#define NP_PAYLOAD_CAPA_BIT_CAP_ENCR_OFF    2  ///< the micro device is running unencrypted
#define NP_PAYLOAD_CAPA_BIT_CAP_MICRO       3  ///< the device is a micro device
#define NP_PAYLOAD_CAPA_BIT_CAP_UDP         4  ///< peer is able to use direct UDP
#define NP_PAYLOAD_CAPA_BIT_CAP_TAG         5  ///< peer is able to handle packet tags
#define NP_PAYLOAD_CAPA_BIT_CAP_FRCTRL      6  ///< peer is able to handle Framing Control messages
#define NP_PAYLOAD_CAPA_BIT_CAP_PROXY       7  ///< peer has to communicate through a http proxy
#define NP_PAYLOAD_CAPA_BIT_CAP_ASYNC       8  ///< peer is able to treat async application requests (unabto)
#define NP_PAYLOAD_CAPA_BIT_CAP_FB_TCP_U    9  ///< peer is able to use FB (unencrypted handshake)
#define NP_PAYLOAD_CAPA_BIT_CAP_CLIENT_U    10 ///< client is able to use unencrypted fallback handshake
#define NP_PAYLOAD_CAPA_BIT_CAP_FP          11 ///< Peer is able understand fignerprints in the connect


/*****************************************************************************/
/* NOTIFY payload */
/* The NOTIFY payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Notification code       4 bytes in network byte order   |
*      +-----+-----+----------------------------------------------------------+
*      |  +8 |  +4 |  NSI                     Optional NSI (of micro device)  |
*      +-----+-----+----------------------------------------------------------+
* The only way to determine whether the NSI field is present or not is to
* consult the Length field of the payload header. If the Length is
* NP_PAYLOAD_NOTIFY_NSI_BYTELENGTH the NSI field is present in the payload.
*/
#define NP_PAYLOAD_NOTIFY_BYTELENGTH       8  ///< size of a NOTIFY payload without NSI
#define NP_PAYLOAD_NOTIFY_NSI_BYTELENGTH  12  ///< size of a NOTIFY payload with NSI

/* Notification codes for NP_PACKET_HDR_TYPE_U_CONNECT responses */
#define NP_PAYLOAD_NOTIFY_CONNECT_OK              0x0001  ///< The SP_ATTACH procedure succeeded.
#define NP_PAYLOAD_NOTIFY_ERROR                   0x8000  ///< The SP_ATTACH procedure failed.
#define NP_PAYLOAD_NOTIFY_ERROR_UNKNOWN_SERVER    0x8001  ///< The ServerPeer is unknown.
#define NP_PAYLOAD_NOTIFY_ERROR_SP_CERTIFICATE    0x8002  ///< The SP certificate didn't verify.
#define NP_PAYLOAD_NOTIFY_ERROR_CP_CERTIFICATE    0x8003  ///< The CP certificate didn't verify.
#define NP_PAYLOAD_NOTIFY_ERROR_CP_ACCESS         0x8004  ///< The CP wasn't granted access.
#define NP_PAYLOAD_NOTIFY_ERROR_PROTOCOL_VERSION  0x8005  ///< The protocol version is not supported.
#define NP_PAYLOAD_NOTIFY_ERROR_BAD_CERT_ID       0x8006  ///< The ID in the certificate wasn't as expected.
#define NP_PAYLOAD_NOTIFY_ERROR_UNKNOWN_MICRO     0x8007  ///< The Micro Server is unknown or not attached.
#define NP_PAYLOAD_NOTIFY_ERROR_ENCR_MISMATCH     0x8008  ///< The peers don't agree on en/decryption.
#define NP_PAYLOAD_NOTIFY_ERROR_BUSY_MICRO        0x8009  ///< The Micro Server is busy
#define NP_PAYLOAD_NOTIFY_ERROR_MICRO_REQ_ERR     0x800A  ///< The Micro Server can't give response to dialogue request
#define NP_PAYLOAD_NOTIFY_ERROR_MICRO_REATTACHING 0x800B  ///< The Micro Server is re-attaching
#define NP_PAYLOAD_NOTIFY_ERROR_SELF_SIGNED       0x800C  ///< The CP certificate is self signed and not accepted by the gsp.
#define NP_PAYLOAD_NOTIFY_ERROR_CP_ACCESS_BS       0x800D  ///< The CP is rejected access by the basestation.
#define NP_PAYLOAD_NOTIFY_ERROR_CONNECTION_ABORTED 0x800E  ///< The CP has aborted the connection attempt.

/* Notification codes for NP_PACKET_HDR_TYPE_U_ATTACH responses */
#define NP_PAYLOAD_NOTIFY_ATTACH_OK          0x00000001l  ///< The device is attached.

/* Notification codes for NP_PACKET_HDR_TYPE_U_ALIVE responses */
#define NP_PAYLOAD_NOTIFY_ALIVE_OK           0x00000001l  ///< The device is alive.

/* Notification codes for NP_PACKET_HDR_TYPE_U_DEBUG */
#define NP_PAYLOAD_NOTIFY_DEBUG_OK           0x0001
#define NP_PAYLOAD_NOTIFY_DEBUG_ERROR        0x8000
#define NP_PAYLOAD_NOTIFY_SYSLOG_DISABLED    0x8001

/*****************************************************************************/
/* VERIFY payload */
/* The VERIFY payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Checksum         12 bytes                               |
*      +-----+-----+----------------------------------------------------------+
* The checksum is filled with 0xCD by default when sent from the micro device to the gsp.
* The checksum is filled with 0xAB by default when sent from the gsp to the micro device.
*/

#define NP_PAYLOAD_VERIFY_BYTELENGTH        16  ///< size of a VERIFY payload

/* Checksum filler bytes */
#define NP_PAYLOAD_VERIFY_GSP_FILLERBYTE  0xAB  ///< filler byte in checksum from gsp
#define NP_PAYLOAD_VERIFY_DEV_FILLERBYTE  0xCD  ///< filler byte in checksum from micro device


/*****************************************************************************/
/* EP payload */
/* The EP payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  IPv4 address       4 bytes in network byte order        |
*      +-----+-----+----------------------------------------------------------+
*      |  +8 |  +4 |  Port number        2 bytes in network byte order        |
*      +-----+-----+----------------------------------------------------------+
* The EP payload describes an endpoint in a peer.
*/

#define NP_PAYLOAD_EP_BYTELENGTH            10  ///< size of a EP payload


/*****************************************************************************/
/* GW payload */
/* The GW payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  IPv4 address       4 bytes in network byte order        |
*      +-----+-----+----------------------------------------------------------+
*      |  +8 |  +4 |  Port number        2 bytes in network byte order        |
*      +-----+-----+----------------------------------------------------------+
*      | +10 |  +6 |  NSI.peer           id of peer (server or client)        |
*      +-----+-----+----------------------------------------------------------+
*      | +14 | +10 |  Gateway ID         unique id for fallback connection    |
*      +-----+-----+----------------------------------------------------------+
* The GW payload describes an endpoint of a gateway connection.
* The gateway ID follows right after the NSI.peer field. The total number of
* bytes in the Gateway ID field are (payload Length - min size of payload header)
* bytes.
*/

#define NP_PAYLOAD_GW_MIN_BYTELENGTH        14  ///< min size of a GW payload

/*****************************************************************************/
/* GW WS payload */
/* The GW payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  websocket host string length                            |
*      +-----+-----+----------------------------------------------------------+
*      |  +6 |  +2 |  websocket host string                                   |
*      +-----+-----+----------------------------------------------------------+
*      | +10 |  +6 |  NSI.peer           id of peer (server or client)        |
*      +-----+-----+----------------------------------------------------------+
*      | +14 | +10 |  Gateway ID         unique id for fallback connection    |
*      +-----+-----+----------------------------------------------------------+
* The GW payload describes an endpoint of a gateway websocket connection.
* The gateway ID follows right after the NSI.peer field. The total number of
* bytes in the Gateway ID are the rest of the bytes in the payload.
*/


/*****************************************************************************/
/* Basestation AUTH Payload */
/* The Basestation AUTH  payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*        A list of strings paired to be key value pairs.
*      +-----+-----+----------------------------------------------------------+
*      |     |  +x |  string length (16bits)                                  |
*      +-----+-----+----------------------------------------------------------+
*      |     |+x+2 |  string data                                             |
*      +-----+-----+----------------------------------------------------------+
*/


/*****************************************************************************/
/* NONCE payload */
/* The NONCE payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Nonce              any number of bytes                  |
*      +-----+-----+----------------------------------------------------------+
* The nonce data follows right after the payload header. The total number of
* bytes in the nonce are (payload Length - size of payload header) bytes.
*/

#define NP_PAYLOAD_NONCE_BYTELENGTH          4  ///< size of a NONCE payload without data

/* Different types of (unecrypted) fallback connections (inside a NONCE inside a GW_CONN_U packet) */
#define NP_GW_CONN_U_TYPE_TCP             0  ///< tcp fallback connection
#define NP_GW_CONN_U_TYPE_WEBSOCKET       1  ///< websocket fallback connection.

/* Different flags for (unecrypted) fallback connections (inside a NONCE inside a GW_CONN_U packet) */
#define NP_GW_CONN_U_FLAG_NONE            0x0000  ///< default behavior
#define NP_GW_CONN_U_FLAG_RELIABLE        0x0001  ///< reliable connection - no retrans needed
#define NP_GW_CONN_U_FLAG_KEEP_ALIVE      0x0002  ///< keep alive traffic needed
#define NP_GW_CONN_U_FLAG_CLIENT          0x0004  ///< set if this is a client.
#define NP_GW_CONN_U_FLAG_SERVER          0x0008  ///< set if this is a server.


/*****************************************************************************/
/* WINDOW payload */
/* The WINDOW payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Flags             streaming window flags                |
*      +-----+-----+----------------------------------------------------------+
*      |  +5 |  +1 |  Version           streaming version (NP_STREAM_VERSION) |
*      +-----+-----+----------------------------------------------------------+
*      |  +6 |  +2 |  Initiator         id of initiator                       |
*      +-----+-----+----------------------------------------------------------+
*      |  +8 |  +4 |  Responder         id of responder                       |
*      +-----+-----+----------------------------------------------------------+
*      | +10 |  +6 |  Sequence          transmit sequence number              |
*      +-----+-----+----------------------------------------------------------+
*      | +14 | +10 |  Acknowledgment    acknowledgment sequence number        |
*      +-----+-----+----------------------------------------------------------+
*
* If the Flags field only has the NP_PAYLOAD_WINDOW_FLAG_ACK flag, the
* following data is present also:
*
*      +-----+-----+----------------------------------------------------------+
*      | +18 | +14 |  Size               window size                          |
*      +-----+-----+----------------------------------------------------------+
*
* If the Flags field has the NP_PAYLOAD_WINDOW_FLAG_SYN flag set, the
* following data may be present also:
*
*      +-----+-----+----------------------------------------------------------+
*      | +18 | +14 |  Options                      streaming options          |
*      +-----+-----+----------------------------------------------------------+
*      | +20 | +16 |  Receiver packet size         2 bytes in network order   |
*      +-----+-----+----------------------------------------------------------+
*      | +22 | +18 |  Receiver window size         2 bytes in network order   |
*      +-----+-----+----------------------------------------------------------+
*      | +24 | +20 |  Transmitter packet size      2 bytes in network order   |
*      +-----+-----+----------------------------------------------------------+
*      | +26 | +22 |  Transmitter window size      2 bytes in network order   |
*      +-----+-----+----------------------------------------------------------+
*      | +28 | +24 |  Max retransmissions          2 bytes in network order   |
*      +-----+-----+----------------------------------------------------------+
*      | +30 | +26 |  Retransmission timeout       2 bytes in network order   |
*      +-----+-----+----------------------------------------------------------+
* Consult the Length field of the payload header to determine whether the SYN data
* is present or not.
* If the Length is NP_PAYLOAD_WINDOW_SYN_BYTELENGTH the SYN data is present in the
* payload.
*/

#define NP_PAYLOAD_WINDOW_BYTELENGTH        18  ///< size of WINDOW payload (standard)
#define NP_PAYLOAD_WINDOW_ACK_BYTELENGTH    20  ///< size of WINDOW payload with ACK
#define NP_PAYLOAD_WINDOW_SYN_BYTELENGTH    32  ///< size of WINDOW payload with SYN

/* WINDOW payload flags */
#define NP_PAYLOAD_WINDOW_FLAG_NON        0x00  ///< no flags
#define NP_PAYLOAD_WINDOW_FLAG_ACK        0x10  ///< acknowledge packet
#define NP_PAYLOAD_WINDOW_FLAG_RST        0x20  ///< rst packet.
#define NP_PAYLOAD_WINDOW_FLAG_FIN        0x40  ///< tear-down packet
#define NP_PAYLOAD_WINDOW_FLAG_SYN        0x80  ///< connect packet

/* WINDOW payload streaming options (For the options field in the syn data)*/
#define NP_PAYLOAD_STREAM_FLAG_WSRF     0x0001  ///< WSRF enabled (window size reduction feature)
#define NP_PAYLOAD_STREAM_FLAG_SACK     0x0002  ///< SACK enabled (selective acknowledge)


/*****************************************************************************/
/* SACK payload */
/* The SACK payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  List               a maximum of 4 32-bit pairs          |
*      +-----+-----+----------------------------------------------------------+
* The list of acknowledge data follows right after the header field. The
* acknowledge data is a list containing a pair of 32-bit numbers. Each pair
* contains sequence numbers. All sequence number between the two numbers
* are acknowledged. The total number of pairs in the list can be found as
* (payload length - size of payload header) / 8. The total number of
* bytes in the List field are (payload Length - size of payload header).
*/

#define NP_PAYLOAD_SACK_BYTELENGTH           4  ///< size of a SACK payload without list
#define NP_PAYLOAD_SACK_MAX_PAIRS            4  ///< Maximum number of sack pairs in a sack payload.
#define NP_PAYLOAD_SACK_MAX_BYTELENGTH       NP_PAYLOAD_SACK_BYTELENGTH + NP_PAYLOAD_SACK_MAX_PAIRS * 8

/*****************************************************************************/
/* Stats payload */
/* The stats payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Stats type                                              |
*      +-----+-----+----------------------------------------------------------+
*/

#define NP_PAYLOAD_STATS_BYTELENGTH          5  ///< size of a stats payload

#define NP_PAYLOAD_STATS_TYPE_DEVICE_CONNECTION_OPEN          1
#define NP_PAYLOAD_STATS_TYPE_DEVICE_CONNECTION_FAIL          2
#define NP_PAYLOAD_STATS_TYPE_DEVICE_CONNECTION_CLOSE         3
#define NP_PAYLOAD_STATS_TYPE_CLIENT_CONNECTION_OPEN          4
#define NP_PAYLOAD_STATS_TYPE_CLIENT_CONNECTION_FAIL          5
#define NP_PAYLOAD_STATS_TYPE_CLIENT_CONNECTION_CLOSE         6
#define NP_PAYLOAD_STATS_TYPE_DEVICE_ATTACH_FAIL              7
#define NP_PAYLOAD_STATS_TYPE_DEVICE_STREAM_CLOSE             8

/*****************************************************************************
 * Connect stats payload *
 * Deprecated use connection info payload instead
 * The connect stats payload data has the following layout:
 *      +-----+----------------------------------------------------------------+
 *      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
 *      +-----+-----+----------------------------------------------------------+
 *      |  +4 |  +0 |  Version                                                 |
 *      +-----+-----+----------------------------------------------------------+
 *      |  +5 |  +1 |  Connection type                                         |
 *      +-----+-----+----------------------------------------------------------+
 */

#define NP_PAYLOAD_CONNECT_STATS_BYTELENGTH   6  ///< size of a connect stats payload

#define NP_PAYLOAD_CONNECT_STATS_VERSION 1

enum np_payload_connect_stats_connection_type_e {
    NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_CONNECTING   = 0,  ///< Still trying to connect
    NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_LOCAL        = 1,  
    NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_P2P          = 2,
    NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_UDP_RELAY    = 3,
    NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_TCP_RELAY    = 4,
    NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_HTTP_RELAY   = 5,
    NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_LOCAL_LEGACY = 6
};


/*****************************************************************************/
/* Rendezvous stats payload */
/* The rendezvous stats payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Version                                                 |
*      +-----+-----+----------------------------------------------------------+
*      |  +5 |  +1 |  Client nat type (same as NP_PAYLOAD_IPX_NAT*)           |
*      +-----+-----+----------------------------------------------------------+
*      |  +6 |  +2 |  uNabto nat type (same as NP_PAYLOAD_IPX_NAT*)           |
*      +-----+-----+----------------------------------------------------------+
*      |  +7 |  +3 |  rendezvous ports opened                                 |
*      +-----+-----+----------------------------------------------------------+
*      |  +9 |  +5 |  rendezvous sockets opened                               |
*      +-----+-----+----------------------------------------------------------+
*/

#define NP_PAYLOAD_RENDEZVOUS_STATS_BYTELENGTH   11  ///< size of a rendezvous stats payload

#define NP_PAYLOAD_RENDEZVOUS_STATS_VERSION 1

/*****************************************************************************
* Connection stats payload *
* Deprecated use connection info payload instead
* The connection stats payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Version                                                 |
*      +-----+-----+----------------------------------------------------------+
*      |  +5 |  +1 |  Connection age milliseconds                             |
*      +-----+-----+----------------------------------------------------------+
*      |  +9 |  +5 |  packets received                                        |
*      +-----+-----+----------------------------------------------------------+
*      |  +13|  +9 |  packets sent                                            |
*      +-----+-----+----------------------------------------------------------+
*      |  +17|  +13|  bytes received                                          |
*      +-----+-----+----------------------------------------------------------+
*      |  +21|  +17|  bytes sent                                              |
*      +-----+-----+----------------------------------------------------------+
*/

#define NP_PAYLOAD_CONNECTION_STATS_BYTELENGTH   25  ///< size of a connection stats payload

#define NP_PAYLOAD_CONNECTION_STATS_VERSION 1

/*****************************************************************************/
/* Connection info payload this is a replacement for the connection stats payload */
/* The connection info payload data has the following layout:
 *      List of the following data type
 *      +-----+----------------------------------------------------------+
 *      | +0 | Value type (uint8_t)                                      |
 *      +-----+----------------------------------------------------------+
 *      | +1 | Value Length (uint8_t) (including type and length)        |
 *      +-----+----------------------------------------------------------+
 *      | +2 | Value format which depends on the type                    |
 *      +-----+----------------------------------------------------------+
 */

enum np_payload_connection_info_e {
    NP_PAYLOAD_CONNECTION_INFO_TYPE              = 1, // uint8_t connection type see 
    NP_PAYLOAD_CONNECTION_INFO_DURATION          = 2, // duration in milliseconds
    NP_PAYLOAD_CONNECTION_INFO_SENT_BYTES        = 3, // uint32_t
    NP_PAYLOAD_CONNECTION_INFO_SENT_PACKETS      = 4, // uint32_t
    NP_PAYLOAD_CONNECTION_INFO_RECEIVED_BYTES    = 5, // uint32_t
    NP_PAYLOAD_CONNECTION_INFO_RECEIVED_PACKETS  = 6, // uint32_t
    NP_PAYLOAD_CONNECTION_INFO_CLIENT_IP         = 7, // uint32_t public ipv4 in network byte order
    NP_PAYLOAD_CONNECTION_INFO_DEVICE_IP         = 8  // uint32_t public ipv4 in network byte order
};

/*****************************************************************************/
/* Attach stats payload */
/* The attachment statistics payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Version                                                 |
*      +-----+-----+----------------------------------------------------------+
*      |  +5 |  +1 |  Status Code                                             |
*      +-----+-----+----------------------------------------------------------+
*      |  +6 |  +2 |  attach flags                                            |
*      +-----+-----+----------------------------------------------------------+
*/

#define NP_PAYLOAD_ATTACH_STATS_BYTELENGTH   7  ///< size of a rendezvous stats payload

#define NP_PAYLOAD_ATTACH_STATS_VERSION 1

#define NP_PAYLOAD_ATTACH_STATS_STATUS_OK 0
#define NP_PAYLOAD_ATTACH_STATS_STATUS_FAILED 1
#define NP_PAYLOAD_ATTACH_STATS_STATUS_VERIFICATION_FAILED 2
#define NP_PAYLOAD_ATTACH_STATS_STATUS_DECRYPTION_FAILED 3
#define NP_PAYLOAD_ATTACH_STATS_STATUS_ATTACH_TIMED_OUT 4
#define NP_PAYLOAD_ATTACH_STATS_STATUS_GSP_ATTACH_FAILED 5
#define NP_PAYLOAD_ATTACH_STATS_STATUS_CONTROLLER_INVITE_FAILED 6
#define NP_PAYLOAD_ATTACH_STATS_STATUS_DNS_LOOKUP_FAILED 7
#define NP_PAYLOAD_ATTACH_STATS_STATUS_NONCE_VERIFICATION_FAILED 8
#define NP_PAYLOAD_ATTACH_STATS_STATUS_INTEGRITY_CHECK_FAILED 9

#define NP_PAYLOAD_ATTACH_STATS_FLAGS_SECURE_ATTACH 0x1

/*****************************************************************************/
/* Stream stats payload */
/* The attachment statistics payload data has the following layout:
 *      +-----+----------------------------------------------------------------+
 *      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
        +-----+----------------------------------------------------------------+
 *      List of the following data type
 *      +-----+----------------------------------------------------------+
 *      | +0 | Value type (uint8_t)                                      |
 *      +-----+----------------------------------------------------------+
 *      | +1 | Value Length (uint8_t) (including type and length)        |
 *      +-----+----------------------------------------------------------+
 *      | +2 | Value format which depends on the type                    |
 *      +-----+----------------------------------------------------------+
 */

#define NP_PAYLOAD_STREAM_STATS_VERSION 1

enum np_payload_stream_stats_e {
    NP_PAYLOAD_STREAM_STATS_SENT_PACKETS                          = 1,  /* uint32_t */
    NP_PAYLOAD_STREAM_STATS_SENT_BYTES                            = 2,  /* uint32_t */
    NP_PAYLOAD_STREAM_STATS_SENT_RESENT_PACKETS                   = 3,  /* uint32_t */
    NP_PAYLOAD_STREAM_STATS_RECEIVED_PACKETS                      = 4,  /* uint32_t */
    NP_PAYLOAD_STREAM_STATS_RECEIVED_BYTES                        = 5,  /* uint32_t */
    NP_PAYLOAD_STREAM_STATS_RECEIVED_RESENT_PACKETS               = 6,  /* uint32_t */
    NP_PAYLOAD_STREAM_STATS_REORDERED_OR_LOST_PACKETS             = 7,  /* uint32_t */
    NP_PAYLOAD_STREAM_STATS_USER_WRITE                            = 8,  /* uint32_t number of times write was called on the stream */
    NP_PAYLOAD_STREAM_STATS_USER_READ                             = 9,  /* uint32_t number of times read was called on the stream */

    NP_PAYLOAD_STREAM_STATS_RTT_MIN                               = 10, /* uint16_t round trip time */
    NP_PAYLOAD_STREAM_STATS_RTT_MAX                               = 11, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_RTT_AVG                               = 12, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_CWND_MIN                              = 13, /* uint16_t congestion window size */
    NP_PAYLOAD_STREAM_STATS_CWND_MAX                              = 14, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_CWND_AVG                              = 15, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_SS_THRESHOLD_MIN                      = 16, /* uint16_t slow start threshold */
    NP_PAYLOAD_STREAM_STATS_SS_THRESHOLD_MAX                      = 17, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_SS_THRESHOLD_AVG                      = 18, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_FLIGHT_SIZE_MIN                       = 19, /* uint16_t packets awaiting acknowledgedment on the network */
    NP_PAYLOAD_STREAM_STATS_FLIGHT_SIZE_MAX                       = 20, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_FLIGHT_SIZE_AVG                       = 21, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_DURATION                              = 22, /* uint32_t duration in ms */
    NP_PAYLOAD_STREAM_STATS_STATUS                                = 23, /* uint8_t (np_payload_stream_stats_status_e)*/
    NP_PAYLOAD_STREAM_STATS_CP_ID                                 = 24, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_SP_ID                                 = 25, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_TAG                                   = 26, /* uint16_t */
    NP_PAYLOAD_STREAM_STATS_TIME_FIRST_MB_RECEIVED                = 27, /* uint32_t duration in ms */
    NP_PAYLOAD_STREAM_STATS_TIME_FIRST_MB_SENT                    = 28, /* uint32_t duration in ms */
    NP_PAYLOAD_STREAM_STATS_TIMEOUTS                              = 29  /* uint32_t counter of stream congestion cointrol timeouts*/
    
};

enum np_payload_stream_stats_status_e {
    NP_PAYLOAD_STREAM_STATS_STATUS_OPEN = 1,
    NP_PAYLOAD_STREAM_STATS_STATUS_CLOSED = 2,
    NP_PAYLOAD_STREAM_STATS_STATUS_CLOSED_ABORTED = 3
};

/*****************************************************************************/
/* System info payload */
/* The system info payload has the following payload data format. The
 * idea is to provide a lightweight optional data structure for system
 * information of statistical characterization.
 * +-----+----------------------------------------------------------------+
 * | +0 | Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)                |
 * +-----+-----+----------------------------------------------------------+
 * List of the following data type
 * +-----+----------------------------------------------------------+
 * | +0 | Value type (uint8_t)                                      |
 * +-----+----------------------------------------------------------+
 * | +1 | Value Length (uint8_t)  (including type and length bytes) |
 * +-----+----------------------------------------------------------+
 * | +2 | Value format which depends on the type                    |
 * +-----+----------------------------------------------------------+
 */
enum np_payload_system_info_e {
    NP_PAYLOAD_SYSTEM_INFO_STUN_DNS_LATENCY         = 1, /* uint32_t milliseconds */
    NP_PAYLOAD_SYSTEM_INFO_STUN_UDP_LATENCY         = 2, /* uint32_t milliseconds */
    NP_PAYLOAD_SYSTEM_INFO_STUN_MAPPING             = 3, /* uint8_t mapping type. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_FILTERING           = 4, /* uint8_t filtering type. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_MISSING_FIREWALL    = 5, /* uint8_t boolean. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_PORT_PRESERVING     = 6, /* uint8_t boolean. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_HAIR_PINNING        = 7, /* uint8_t boolean. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_GENERIC_NAT_ALG     = 8, /* uint8_t boolean. */
    NP_PAYLOAD_SYSTEM_INFO_NAT64                    = 9, /* uint8_t nat64 type. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_GLOBAL_IP_CHANGES   = 10 /* uint8_t boolean. */
};

enum np_payload_system_info_stun_mapping_e {
    NP_PAYLOAD_SYSTEM_INFO_STUN_MAPPING_BLOCKING          = 0x01, /* StunServer could not be reached. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_MAPPING_NO_NAT            = 0x02, /* It's a global ip address. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_MAPPING_NAT               = 0x03, /* There's some kind of NAT but the type is unknown. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_MAPPING_INDEPENDENT       = 0x04, /* The NAT mapping is independent. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_MAPPING_ADDRESS_DEPENDENT = 0x05, /* The NAT mapping depends on the remote address. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_MAPPING_PORT_DEPENDENT    = 0x06  /* The NAT mapping depends on the remote address and port. */
};

enum np_payload_system_info_stun_filtering_e {
    NP_PAYLOAD_SYSTEM_INFO_STUN_FILTERING_PENDING           = 0x01, /* The filtering test is not finished. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_FILTERING_INDEPENDENT       = 0x02, /* The filtering is independent of the remote address (full cone). */
    NP_PAYLOAD_SYSTEM_INFO_STUN_FILTERING_ADDRESS_DEPENDENT = 0x03, /* The filtering depends on the remote address. */
    NP_PAYLOAD_SYSTEM_INFO_STUN_FILTERING_PORT_DEPENDENT    = 0x04  /* The filtering depends on the remote address and port. */
};

enum np_payload_system_info_nat64_e {
    NP_PAYLOAD_SYSTEM_INFO_NAT64_NOT_PRESENT     =  0x01, /* There's no NAT64 present. */
    NP_PAYLOAD_SYSTEM_INFO_NAT64_UNPREDICTABLE   =  0x02, /* There's NAT64 but the prefix is unpredictable. */
    NP_PAYLOAD_SYSTEM_INFO_NAT64_PREDICTABLE     =  0x03  /* There's NAT64 and the prefix is predictable. */
};

/*****************************************************************************/
/* DESCR payload */
/* The DESCR payload data has the following layout:
*      +-----+----------------------------------------------------------------+
*      |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)              |
*      +-----+-----+----------------------------------------------------------+
*      |  +4 |  +0 |  Type               Type of description                  |
*      +-----+-----+----------------------------------------------------------+
*      |  +5 |  +1 |  Data               Description - any number of bytes    |
*      +-----+-----+----------------------------------------------------------+
* The description data follows right after the Type field. The total number of
* bytes in the Data field are (payload Length - NP_PAYLOAD_DESCR_BYTELENGTH)
* bytes.
*/

#define NP_PAYLOAD_DESCR_BYTELENGTH            5  ///< size of a DESCR payload without data

/* Type of descriptions */
#define NP_PAYLOAD_DESCR_TYPE_VERSION          1  ///< version number of the micro device
#define NP_PAYLOAD_DESCR_TYPE_URL              2  ///< location of html device driver
#define NP_PAYLOAD_DESCR_TYPE_ATTACH_PORT      3  ///< deprecated - may still be in use in old code
#define NP_PAYLOAD_DESCR_TYPE_LOCAL_CONN       4  ///< send if the device supports local connections
#define NP_PAYLOAD_DESCR_TYPE_FP               5  ///< send if the device supports fingerprints
#define NP_PAYLOAD_DESCR_TYPE_LOCAL_CONN_PSK   6  ///< send if the device supports psk authenticated local connections


/*****************************************************************************/
/* PIGGY Payload */
/* The PIGGY Payload has the following layout:
*    +-----+-----------------------------------------------------------------+
*    |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)               |
*    +-----+-----------------------------------------------------------------+
*    |  +4 |  +0 | Unused               Reserved for future use.             |
*    +-----+-----+-----------------------------------------------------------+
*    |  +8 |  +4 | Data                 Piggyback message data.              |
*    +-----+-----+-----------------------------------------------------------+
* The message data follows right after the Unused field. The total number of
* bytes in the message are (payload Length - NP_PAYLOAD_PIGGY_SIZE_WO_DATA)
* bytes.
*/

#define NP_PAYLOAD_PIGGY_SIZE_WO_DATA       8 ///< Size of the piggy payload with 0 bytes of data.

/*****************************************************************************/
/* PING Payload */
/* The Ping Payload has the following layout:
*    +-----+-----------------------------------------------------------------+
*    |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)               |
*    +-----+-----------------------------------------------------------------+
*    |  +4 |  +0 | Identifier                                                |
*    +-----+-----+-----------------------------------------------------------+
*    |  +6 |  +2 | Sequence number                                           |
*    +-----+-----+-----------------------------------------------------------+
*    |  +8 |  +4 | Flags                                                     |
*    +-----+-----+-----------------------------------------------------------+
*    |  +9 |  +5 | Sent stamp                                                |
*    +-----+-----+-----------------------------------------------------------+
*    | +17 | +13 | Data                 Ping data                            |
*    +-----+-----+-----------------------------------------------------------+
*/

// If this is a ping response set the PING response flag in the ping payload flags.
#define NP_PAYLOAD_PING_FLAG_RSP 0x1

#define NP_PAYLOAD_PING_SIZE_WO_DATA      17 ///< Size of the ping payload without data.



/*****************************************************************************/
/* Syslog Config Payload */
/* The Syslog config Payload has the following layout:
*    +-----+-----------------------------------------------------------------+
*    |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)               |
*    +-----+-----------------------------------------------------------------+
*    |  +4 |  +0 | flags                                                     |
*    +-----+-----+-----------------------------------------------------------+
*    |  +5 |  +1 | syslog facility                                           |
*    +-----+-----+-----------------------------------------------------------+
*    |  +6 |  +2 | syslog port    (syslog port number)                       |
*    +-----+-----+-----------------------------------------------------------+
*    |  +8 |  +4 | syslog ip      (syslog ip4 address)                       |
*    +-----+-----+-----------------------------------------------------------+
*    | +12 | +10 | syslog expire  (number of seconds to run syslog)          |
*    +-----+-----+-----------------------------------------------------------+
*    | +16 | +12 | string syslog log pattern                                 |
*    +-----+-----+-----------------------------------------------------------+
*    |  +? |  +? | string syslog hostname (Syslog hostname)                  |
*    +-----+-----+-----------------------------------------------------------+
*
* The syslog pattern string is of the format module.level e.g. *.trace
* The syslog hostname string is a possibility tpo override the
* hostname string the client would otherwise use.
*/

#define NP_PAYLOAD_SYSLOG_CONFIG_SIZE_WO_STRINGS    16 ///< Size of the syslog payload without data.

/**
 * If this flag is set in the packet syslogging should be disabled.
 */
#define NP_PAYLOAD_SYSLOG_FLAG_ENABLE              0x01 


/******************************************************************************/
/* Push notification payload */
/* The Push notification payload has the following layout:
 *    +-----+-----------------------------------------------------------------+
 *    |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)               |
 *    +-----+-----------------------------------------------------------------+
 *    |  +4 |  +0 | Sequence number                                           |
 *    +-----+-----+-----------------------------------------------------------+
 *    |  +8 |  +4 | PNS id                                                    |
 *    +-----+-----+-----------------------------------------------------------+
 *    |  +10|  +6 | Flags                                                     |
 *    +-----+-----+-----------------------------------------------------------+
 */

#define NP_PAYLOAD_PUSH_BYTELENGTH    11 ///< Size of the push notification payload

#define NP_PAYLOAD_PUSH_FLAG_SEND                    0x00
#define NP_PAYLOAD_PUSH_FLAG_ACK                     0x10
#define NP_PAYLOAD_PUSH_FLAG_FAIL                    0x20
#define NP_PAYLOAD_PUSH_FLAG_QUOTA_EXCEEDED          0x40
#define NP_PAYLOAD_PUSH_FLAG_QUOTA_EXCEEDED_REATTACH 0x80

/******************************************************************************/
/* Push notification Data payload */
/* The Push notification Data payload has the following layout:
 *    +-----+-----------------------------------------------------------------+
 *    |  +0 |  Payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes)               |
 *    +-----+-----------------------------------------------------------------+
 *    |  +4 |  +0 | purpose                                                   |
 *    +-----+-----+-----------------------------------------------------------+
 *    |  +5 |  +1 | encoding                                                  |
 *    +-----+-----+-----------------------------------------------------------+
 *    |  +6 |  +2 | Data                                                      |
 *    +-----+-----+-----------------------------------------------------------+
 */

#define NP_PAYLOAD_PUSH_DATA_SIZE_WO_DATA    6 ///< Size of the push notification payload
#define NP_PAYLOAD_PUSH_DATA_PURPOSE_STATIC  1 ///< purpose value for static data from client
#define NP_PAYLOAD_PUSH_DATA_PURPOSE_DYNAMIC 2 ///< Purpose value for dynamic data from uNabto
#define NP_PAYLOAD_PUSH_DATA_ENCODING_JSON   1 ///< JSON encoded payload
#define NP_PAYLOAD_PUSH_DATA_ENCODING_TLV    2 ///< TLV encoded payload

/******************************************************************************/
/* Push notification data types                                               */
/* The Push notification data payload of type                                 */
/* NP_PAYLOAD_PUSH_DATA_PURPOSE_DYNAMIC can contain the following data with   */
/* the format, where length is the combined length of Data, Type and Length:  */
/*  +-----+-------------------------------------------------------------------+
 *  |  +0 | Type                                                              |
 *  +-----+-------------------------------------------------------------------+
 *  |  +1 | Length                                                            |
 *  +-----+-------------------------------------------------------------------+
 *  |  +2 | Data                                                              |
 *  +-----+-------------------------------------------------------------------+
 */
#define NP_PAYLOAD_PUSH_DATA_VALUE_TITLE                1
#define NP_PAYLOAD_PUSH_DATA_VALUE_BODY                 2
#define NP_PAYLOAD_PUSH_DATA_VALUE_BODY_LOC_KEY         3
#define NP_PAYLOAD_PUSH_DATA_VALUE_BODY_LOC_STRING_ARG  4
#define NP_PAYLOAD_PUSH_DATA_VALUE_TITLE_LOC_KEY        5
#define NP_PAYLOAD_PUSH_DATA_VALUE_TITLE_LOC_STRING_ARG 6

/*******************
 * Legacy Protocol *
 *******************/

/**
 * The legacy protocol is a non connection based protocol which only
 * consists of stateless request/response communication for new
 * deployments it's only used for local discovery packets.
 */

/**
 * The legacy protocol header consists of 12 bytes. The first field in 
 * 
 *  +-----+-------------------------------------------------------------------+
 *  |  +0 | Hdr (think header header)                                         |
 *  +-----+-------------------------------------------------------------------+
 *  |  +4 | Sequence number                                                   |
 *  +-----+-------------------------------------------------------------------+
 *  |  +2 | Nabto (reserved)                                                  |
 *  +-----+-------------------------------------------------------------------+
 */
#define NP_LEGACY_PACKET_HDR_SIZE 12

#define NP_LEGACY_PACKET_HDR_TYPE_APPLICATION 0x00
#define NP_LEGACY_PACKET_HDR_TYPE_DISCOVERY   0x01

#define NP_LEGACY_PACKET_HDR_FLAG_RSP 0x00800000ul
#define NP_LEGACY_PACKET_HDR_FLAG_ERR 0x00400000ul


#endif
