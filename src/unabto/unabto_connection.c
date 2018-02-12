/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer Connections - Implementation.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_UNABTO

#include "unabto_env_base.h"

#if NABTO_ENABLE_CONNECTIONS

#include "unabto_connection.h"
#include "unabto_external_environment.h"
#include "unabto_app.h"
#include "unabto_packet_util.h"
#include "unabto_logging.h"
#include "unabto_util.h"
#include "unabto_main_contexts.h"
#include "unabto_memory.h"
#include "unabto_stream.h"
#include "unabto_version.h"
#include "unabto_packet.h"
#include "unabto_app_adapter.h"

#include <unabto/unabto_tcp_fallback.h>
#include <unabto/unabto_dns_fallback.h>

#include <unabto/unabto_extended_rendezvous.h>

#include <string.h>

// cache connection timeout such that it should not be recalculated for each network packet.
// 1. Whenever a timestamp is modified in a connection the cache should be invalidated.
// 2. Whenever the cached timeout is passed the cache is invalidated.
// these rules ensure that the cache never has invalid data.
// this cache is used instead of implementing a full blown priority queue.
NABTO_THREAD_LOCAL_STORAGE nabto_stamp_t connection_timeout_cache_stamp;
NABTO_THREAD_LOCAL_STORAGE bool connection_timeout_cache_cached = false;

#if !NABTO_ENABLE_DYNAMIC_MEMORY

NABTO_THREAD_LOCAL_STORAGE nabto_connect connections[NABTO_MEMORY_CONNECTIONS_SIZE];

#endif


static void unabto_connection_set_future_stamp(nabto_stamp_t* stamp, uint16_t future)
{
    connection_timeout_cache_cached = false;
    nabtoSetFutureStamp(stamp, future);
}



#if NABTO_ENABLE_UCRYPTO
static bool unabto_connection_verify_and_decrypt_connect_packet(nabto_packet_header* hdr, uint8_t** decryptedDataStart, uint16_t* decryptedDataLength);
#endif

static bool rendezvous_event(message_event* event, nabto_packet_header* hdr);
static bool connect_event(message_event* event, nabto_packet_header* hdr);

static bool conclude_connection(nabto_connect* con, nabto_endpoint* peer, uint32_t interval);

static void send_rendezvous_to_all(nabto_connect* con);


/**
 * Called after rendezvous ends and we have found a unique connection
 */
static void nabto_rendezvous_end(nabto_connect* con);

/**
 * Called to stop the rendezvous, either because of timeout or if a
 * connection candidate has been found.
 */
static void nabto_rendezvous_end(nabto_connect* con);

static void nabto_rendezvous_start(nabto_connect* con);

/** initialise a connection */
static void nabto_reset_connection(nabto_connect* con)
{
    nabto_stamp_t tmp = nabtoGetStamp();
    NABTO_LOG_TRACE((PRInsi " Reset connection", MAKE_NSI_PRINTABLE(0, con->spnsi, 0)));
    memset(con, 0, offsetof(nabto_connect, cryptoctx));
    con->stamp = tmp;
    con->socket = NABTO_INVALID_SOCKET;
    con->stats.connectionStart = tmp;
#if NABTO_ENABLE_TCP_FALLBACK
    unabto_tcp_fallback_init(con);
    con->tcpFallbackConnectionStamp = tmp;
#endif                

    nabto_crypto_reset(&con->cryptoctx);
}

/** initialise the connection data */
void nabto_init_connections(void)
{
    NABTO_LOG_TRACE(("Init connections"));
    memset(connections, 0, sizeof(struct nabto_connect_s) * NABTO_MEMORY_CONNECTIONS_SIZE);
}

static nabto_connect* nabto_reserve_connection(void)
{
    nabto_connect* con;

    for (con = connections; con < connections + NABTO_MEMORY_CONNECTIONS_SIZE; ++con) {
        if (con->state == CS_IDLE) {
            nabto_reset_connection(con);
            return con;
        }
    }
    
    return 0;
}

#define MAX_TIMEOUT 300ul
void nabto_release_connection_req(nabto_connect* con)
{
    NABTO_LOG_DEBUG((PRInsi " Release connection req", MAKE_NSI_PRINTABLE(0, con->spnsi, 0)));
    if (con->state != CS_CLOSE_REQUESTED) {
        uint32_t timeout = con->timeOut;
        if (timeout > MAX_TIMEOUT) {
            timeout = MAX_TIMEOUT;
        }
        con->state = CS_CLOSE_REQUESTED;
        unabto_connection_set_future_stamp(&con->stamp, timeout);
#if NABTO_ENABLE_STREAM
        nabto_stream_connection_closed(con);
#endif
    } else {
        NABTO_LOG_TRACE(("nabto_release_connection_req on closed connection"));
    }
}

void nabto_release_connection(nabto_connect* con)
{
    if (con->state != CS_IDLE) {
        NABTO_LOG_INFO((PRInsi " Release connection (record %i)", MAKE_NSI_PRINTABLE(0, con->spnsi, 0), nabto_connection_index(con)));
        if (con->state != CS_CLOSE_REQUESTED) {
            con->state = CS_CLOSE_REQUESTED;
#if NABTO_ENABLE_STREAM
            nabto_stream_connection_released(con);
#endif
            framework_connection_released(con);
        }

        // trigger sending of statistics packet
        con->sendConnectionEndedStatistics = true;

        // in case the rendezvous state is still active.
        nabto_rendezvous_end(con);
        
#if NABTO_ENABLE_TCP_FALLBACK
        if (con->hasTcpFallbackCapabilities) {
            unabto_tcp_fallback_close(con);
        }
#endif
        con->state = CS_IDLE;
        nabto_crypto_release(&con->cryptoctx);

    } else {
        NABTO_LOG_TRACE(("nabto_release_connection called on non used connection"));
    }
}

void nabto_terminate_connections() {
    nabto_connect* con;

    for (con = connections; con < connections + NABTO_MEMORY_CONNECTIONS_SIZE; ++con) {
        if (con->state != CS_IDLE) {
            nabto_release_connection(con);
        }
    }

    NABTO_LOG_TRACE(("All connections released!"));
}

nabto_connect* nabto_find_connection(uint32_t spnsi) {
    nabto_connect* con;

    if (spnsi == 0) {
        NABTO_LOG_TRACE(("Connection not found! (SPNSI=0)"));
        return 0;
    }

    for (con = connections; con < connections + NABTO_MEMORY_CONNECTIONS_SIZE; ++con) {
        if (con->spnsi == spnsi && con->state != CS_IDLE) {
            return con;
        }
    }

    NABTO_LOG_TRACE(("Connection not found! (SPNSI=%" PRIu32 ")", spnsi));
    return 0;
}

int nabto_connection_index(nabto_connect* con)
{
    if (con == 0) {
        return -1;
    }

    return (int)(con - connections);
}


bool nabto_connect_event(message_event* event, nabto_packet_header* hdr)
{
    /**
     * We can get here if we got either a connect request or
     * if the connect request is a rendezvous event.
     *
     * If the first payload is an endpoint it's a rendezvous event.
     * If the request is a request for a new connection the first payload will be an IPX payload.
     */
    const uint8_t* begin = nabtoCommunicationBuffer+hdr->hlen;
    const uint8_t* end = nabtoCommunicationBuffer + hdr->len;
    struct unabto_payload_packet payload;
    const uint8_t* next = unabto_read_payload(begin, end, &payload);

    if (next == NULL) {
        NABTO_LOG_ERROR(("Missing payload in connect packet"));
        return false;
    }

    if (payload.type == NP_PAYLOAD_TYPE_EP) {
        return rendezvous_event(event, hdr);
    } 
    
    if (payload.type == NP_PAYLOAD_TYPE_IPX) {
        return connect_event(event, hdr);
    }
    
    NABTO_LOG_ERROR(("U_CONNECT was neither a rendezvous nor a connect event."));
    return false;
}

bool nabto_connect_event_from_gsp(message_event* event, nabto_packet_header* hdr) {
    if (nmc.context.state != NABTO_AS_ATTACHED) {
        NABTO_LOG_TRACE(("Received connect packet from the GSP but we are not attached yet, so we discard the packet."));
        return false;
    }

    return connect_event(event, hdr);
}


/**
 * Build U_CONNECT response to the GSP or Local Connection
 * @param buf                the destination buffer
 * @param seq                the sequence number
 * @param notif              the result notification
 * @param nsi                the nsi value of the connection
 * @param cpnsi              the nsi of the clientpeer to put into the packet.
 * @param spnsi              the nsi of the serverpeer to put into the packet.
 * @param isLocalConnectRsp  true if a capabilities packet 
 * @return                   the size of the response
 */
static size_t mk_connect_rsp(uint8_t* buf, uint8_t* end, uint16_t seq, uint32_t notif, uint32_t nsi, uint32_t cpnsi, uint32_t spnsi, bool isLocalConnectRsp)
{
    uint8_t* ptr = insert_header(buf, cpnsi, spnsi, U_CONNECT, true, seq, 0, 0);
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_NOTIFY, 0, 8);
    WRITE_U32(ptr, notif); ptr += 4;
    WRITE_U32(ptr, nsi);   ptr += 4;
    
    if (isLocalConnectRsp) {
        ptr = insert_capabilities(ptr, end, 1 /*unenc*/);
    }

    insert_length(buf, ptr - buf);
    return ptr - buf;
}


bool connect_event(message_event* event, nabto_packet_header* hdr)
{
    nabto_endpoint* peer = &event->udpMessage.peer;
    bool isLocal = (event->udpMessage.socket == nmc.socketLocal);
    

    NABTO_LOG_TRACE(("U_CONNECT: Searching for hdr->nsi_cp=%" PRIu32 " (should not be found)", hdr->nsi_cp));
    if (nabto_find_connection(hdr->nsi_cp) == NULL) {
        uint32_t nsi;
        uint32_t ec = 0;
        nabto_connect* con = nabto_init_connection(hdr, &nsi, &ec, isLocal);
        if (con) {
            size_t olen;
            NABTO_LOG_TRACE(("Couldn't find connection (good!). Created a new connection (nsi=%" PRIu32 ") con->spnsi=%" PRIu32, nsi, con->spnsi));
            olen = mk_connect_rsp(nabtoCommunicationBuffer, nabtoCommunicationBuffer + nabtoCommunicationBufferSize, hdr->seq, NOTIFY_CONNECT_OK, con->spnsi, hdr->nsi_cp, hdr->nsi_sp, isLocal);
            
            if (olen == 0) {
                NABTO_LOG_ERROR(("U_CONNECT out of resources in connect event."));
                nabto_release_connection(con); // no resources available
            } else {
                if (EP_EQUAL(*peer, nmc.context.gsp)) {
                    send_to_basestation(nabtoCommunicationBuffer, olen, peer);
                } else {
                    nabto_write(event->udpMessage.socket, nabtoCommunicationBuffer, olen, peer->addr, peer->port);
                }
                
                con->state = CS_CONNECTING;

                if (!con->noRendezvous) {
                    nabto_rendezvous_start(con);
                }
#if NABTO_ENABLE_TCP_FALLBACK
                if (con->hasTcpFallbackCapabilities) {
                    unabto_tcp_fallback_connect(con);
                }
#endif
                return true;
            }
        } else if (nsi && ec) {
            // build negative answer
            size_t olen = mk_connect_rsp(nabtoCommunicationBuffer, nabtoCommunicationBuffer + nabtoCommunicationBufferSize, hdr->seq, ec, nsi, hdr->nsi_cp, hdr->nsi_sp, isLocal);
            NABTO_LOG_TRACE((PRInsi " Deny connection, result: %" PRIu32, MAKE_NSI_PRINTABLE(0, nsi, 0), ec));
            nabto_write(event->udpMessage.socket, nabtoCommunicationBuffer, olen, peer->addr, peer->port);
            return true;
        } else {
            NABTO_LOG_ERROR(("U_CONNECT was a malformed connect event."));
        }
    }
    return false;
}

static void send_rendezvous_socket(nabto_socket_t socket, nabto_connect* con, uint16_t seq, nabto_endpoint* dest, nabto_endpoint *myAddress)
{
    uint8_t* ptr;
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    
    ptr = insert_header(buf, 0, con->spnsi, U_CONNECT, false, seq, 0, 0);
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_EP, 0, 6);
    WRITE_U32(ptr, dest->addr); ptr += 4;
    WRITE_U16(ptr, dest->port); ptr += 2;
    if (seq > 0) {
        if (!myAddress) {
            NABTO_LOG_ERROR(("Send rendezvous called with an invalid address"));
            return;
        } else {
            ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_EP, 0, 6);
            WRITE_U32(ptr, myAddress->addr); ptr += 4;
            WRITE_U16(ptr, myAddress->port); ptr += 2;
        }
    }
    {
        size_t len = ptr - buf;

        insert_length(buf, len);

        if (seq) {
            NABTO_LOG_DEBUG((PRInsi " RENDEZVOUS Send to " PRIep ": seq=%" PRIu16 "  " PRIep, MAKE_NSI_PRINTABLE(0, con->spnsi, 0), MAKE_EP_PRINTABLE(*dest), seq, MAKE_EP_PRINTABLE(*myAddress)));
        } else {
            NABTO_LOG_DEBUG((PRInsi " RENDEZVOUS Send to " PRIep ": seq=0", MAKE_NSI_PRINTABLE(0, con->spnsi, 0), MAKE_EP_PRINTABLE(*dest)));
        }
        if (dest->addr != 0 && dest->port != 0) {
            nabto_write(socket, buf, len, dest->addr, dest->port);
        } else {
            NABTO_LOG_TRACE(("invalid rendezvous packet thrown away"));
        }
    }
}

static void send_rendezvous(nabto_connect* con, uint16_t seq, nabto_endpoint* dest, nabto_endpoint *myAddress) {
    send_rendezvous_socket(nmc.socketGSP, con, seq, dest, myAddress);
}

nabto_connect* nabto_get_new_connection(uint32_t nsi)
{
    nabto_connect* con;
    con  = nabto_find_connection(nsi);
    if (con) {
        return NULL; //nsi is already present - dont allow another one to be created
    }
    con = nabto_reserve_connection();
    if (!con) {
        return NULL; //error - no more connections available
    }
    con->spnsi = nsi;
    return con;
}

/// Retrieve the next NSI. @return the NSI
static uint16_t fresh_nsi(void) {
    uint16_t i;
    static uint16_t nsiStore = 0;  ///< the micoro device's very persistant NSI part:)

    if (nsiStore == 0) {
        // Make a random value in the range [100..1000]
        nabto_random((uint8_t*)&nsiStore, sizeof(nsiStore));
        nsiStore %= 900;
        nsiStore += 100;
    }

    i = 0;
    // Increment the nsi until we find a free NSI.
    do {
        nsiStore++;
        if (nsiStore > 1000) {
            nsiStore = 100;
        }
        i++;
    } while((i <= 900) && (nabto_find_connection(nsiStore) != NULL));
    
    return nsiStore;
}

#define IPX_PAYLOAD_LENGTH_WITHOUT_NSI  (NP_PAYLOAD_IPX_BYTELENGTH - NP_PAYLOAD_HDR_BYTELENGTH)
#define IPX_PAYLOAD_LENGTH_WITH_NSI     (NP_PAYLOAD_IPX_NSI_BYTELENGTH - NP_PAYLOAD_HDR_BYTELENGTH)
#define IPX_PAYLOAD_LENGTH_FULL_NSI     (NP_PAYLOAD_IPX_FULL_NSI_BYTELENGTH - NP_PAYLOAD_HDR_BYTELENGTH)


#if NABTO_ENABLE_UCRYPTO
bool unabto_connection_verify_and_decrypt_connect_packet(nabto_packet_header* hdr, uint8_t** decryptedDataStart, uint16_t* decryptedDataLength) {
    uint8_t* begin = nabtoCommunicationBuffer + hdr->hlen;
    uint8_t* end = nabtoCommunicationBuffer + hdr->len;

    struct unabto_payload_crypto crypto;

    {
        struct unabto_payload_packet payload;
        if (!unabto_find_payload(begin, end, NP_PAYLOAD_TYPE_CRYPTO, &payload)) {
            NABTO_LOG_TRACE(("########    U_CONNECT without Crypto payload from"));
            return false;
        }
        if (!unabto_payload_read_crypto(&payload, &crypto)) {
            NABTO_LOG_TRACE(("cannot read crypto payload"));
            return false;
        }
    }

    if (!unabto_crypto_verify_and_decrypt(hdr, nmc.context.cryptoConnect, &crypto, decryptedDataStart, decryptedDataLength)) 
    {
        NABTO_LOG_TRACE(("U_CONNECT verify or decryption failed"));
        return false;
    }

    if (decryptedDataLength == 0) {
        NABTO_LOG_TRACE(("U_CONNECT Decryption fail, no data"));
        return false;
    }
    return true;
}
#endif

nabto_connect* nabto_init_connection(nabto_packet_header* hdr, uint32_t* nsi, uint32_t* ec, bool isLocal)
{
    nabto_connect* con;

    const uint8_t* begin = nabtoCommunicationBuffer + hdr->hlen;
    const uint8_t* end = nabtoCommunicationBuffer + hdr->len;

    struct unabto_payload_ipx ipxData;
    {
        struct unabto_payload_packet ipxPayload;   
        if (!unabto_find_payload(begin, end, NP_PAYLOAD_TYPE_IPX, &ipxPayload)) {
            NABTO_LOG_ERROR(("Missing ipx payload in connect"));
            return 0;
        }
        
        *nsi = 0;
        *ec  = 0;

        if (!unabto_payload_read_ipx(&ipxPayload, &ipxData)) {
            NABTO_LOG_ERROR(("Cannot parse ipx payload"));
            return 0;
        }
    }

    if (ipxData.haveSpNsi) {
        *nsi = ipxData.spNsi;
        NABTO_LOG_TRACE(("IPX payload with NSI (SPNSI=%" PRIu32 ")", *nsi));
    } else {
        *nsi = fresh_nsi();
        NABTO_LOG_TRACE(("IPX payload without NSI (fresh NSI=%" PRIu32 ")", *nsi));
    }

    if (*nsi == 0) {
        NABTO_LOG_ERROR(("Trying to create connection with spnsi == 0"));
        return 0;
    }

    if (nabto_find_connection(*nsi)) {
        NABTO_LOG_DEBUG((PRInsi " A connection already exists this is probably a retransmission", MAKE_NSI_PRINTABLE(0, *nsi, 0)));
        *ec = NOTIFY_CONNECT_OK;
        return 0;
    }

    con = nabto_get_new_connection(*nsi);

    if (con == 0) {
        if (nabto_find_connection(*nsi)) {
            NABTO_LOG_DEBUG((PRInsi " U_CONNECT: A connection resource is already pending new connection", MAKE_NSI_PRINTABLE(0, *nsi, 0)));
        } else {
            NABTO_LOG_INFO((PRInsi " U_CONNECT: No connection resources free for new connection", MAKE_NSI_PRINTABLE(0, *nsi, 0)));
#if NABTO_ENABLE_DEVICE_BUSY_AS_FATAL
            NABTO_LOG_FATAL((PRInsi " U_CONNECT: No free connections configured to be considered fatal", MAKE_NSI_PRINTABLE(0, *nsi, 0)));
#endif
        }
        *ec = NOTIFY_ERROR_BUSY_MICRO;
        return 0;
    }

    NABTO_LOG_DEBUG((PRInsi " U_CONNECT: Connecting using record %i", MAKE_NSI_PRINTABLE(0, *nsi, 0), nabto_connection_index(con)));

    con->cp.privateEndpoint.addr = ipxData.privateIpAddress;
    con->cp.privateEndpoint.port = ipxData.privateIpPort;
    con->cp.globalEndpoint.addr = ipxData.globalIpAddress;
    con->cp.globalEndpoint.port = ipxData.globalIpPort;
    
    con->noRendezvous = (ipxData.flags & NP_PAYLOAD_IPX_FLAG_NO_RENDEZVOUS) ? 1 : 0;
    con->cpEqual      = EP_EQUAL(con->cp.privateEndpoint, con->cp.globalEndpoint);
    con->cpAsync      = (ipxData.flags & NP_PAYLOAD_IPX_FLAG_CP_ASYNC) ? 1 : 0;
    con->clientNatType      = (ipxData.flags & NP_PAYLOAD_IPX_NAT_MASK);
    con->isLocal      = isLocal;
    NABTO_LOG_INFO((PRInsi " U_CONNECT: cp.private: " PRIep " cp.global: " PRIep ", noRdv=%" PRIu8 ", cpeq=%" PRIu8 ", asy=%" PRIu8 ", NATType: %" PRIu8 , MAKE_NSI_PRINTABLE(0, *nsi, 0), MAKE_EP_PRINTABLE(con->cp.privateEndpoint), MAKE_EP_PRINTABLE(con->cp.globalEndpoint), con->noRendezvous, con->cpEqual, con->cpAsync, con->clientNatType));

#if NABTO_ENABLE_TCP_FALLBACK
    if (ipxData.haveFullNsi) {
        memcpy(con->consi, ipxData.coNsi, 8);
        con->nsico = con->consi;
        con->cpnsi = ipxData.cpNsi;
    }
#endif

    {
        struct unabto_payload_packet cpIdPayload;
        if (unabto_find_payload(begin, end, NP_PAYLOAD_TYPE_CP_ID, &cpIdPayload)) {
            struct unabto_payload_typed_buffer cpId;
            con->clientId[0] = 0;
            if (unabto_payload_read_typed_buffer(&cpIdPayload, &cpId)) {
                if (cpId.type == 1) { // 1 == EMAIL
                    size_t sz = cpId.dataLength;
                    if (sz >= sizeof(con->clientId)) {
                        if (sizeof(con->clientId) > 1) {
                            NABTO_LOG_WARN(("Client ID truncated"));
                        }
                        sz = sizeof(con->clientId) - 1;
                    }
                    if (sz) {
                        memcpy(con->clientId, (const void*) cpId.dataBegin, sz);
                    }
                    con->clientId[sz] = 0;
                }
            }
            NABTO_LOG_TRACE(("Connection opened from '%s' (to %s)", con->clientId, nmc.nabtoMainSetup.id));
        }
    }

#if NABTO_ENABLE_TCP_FALLBACK
    {
        struct unabto_payload_packet gwPayload;
        if (unabto_find_payload(begin, end, NP_PAYLOAD_TYPE_GW, &gwPayload) && ipxData.haveFullNsi) {
            struct unabto_payload_gw gw;
            NABTO_LOG_TRACE(("The connect contains a gateway payload."));
            if (unabto_payload_read_gw(&gwPayload, &gw)) {
                if (gw.gwIdLength != 20) {
                    NABTO_LOG_ERROR(("Gw id should be 20 bytes long."));
                } else {
                    con->fallbackHost.addr = gw.ipAddress;
                    con->fallbackHost.port = gw.port;
                    memcpy(con->gatewayId, gw.gwId, 20);
                    con->hasTcpFallbackCapabilities = true;
                }
            }
        }
    }
#endif
    {
        struct unabto_payload_packet fingerprintPayload;
        if (unabto_find_payload(begin, end, NP_PAYLOAD_TYPE_FP, &fingerprintPayload)) {
            struct unabto_payload_typed_buffer fingerprint;
            if (unabto_payload_read_typed_buffer(&fingerprintPayload, &fingerprint)) {
                if (fingerprint.type == NP_PAYLOAD_FP_TYPE_SHA256_TRUNCATED) {
                    if (fingerprint.dataLength  ==  NP_TRUNCATED_SHA256_LENGTH_BYTES) {
                        con->hasFingerprint = true;
                        memcpy(con->fingerprint, fingerprint.dataBegin, NP_TRUNCATED_SHA256_LENGTH_BYTES);
                    } else {
                        NABTO_LOG_ERROR(("fingerprint has the wrong length %"PRIu16, fingerprint.dataLength));
                    }
                } else {
                    NABTO_LOG_TRACE(("cannot read fignerprint type: %"PRIu8, fingerprint.type));
                }
            }
        }
    }

#if NABTO_ENABLE_UCRYPTO
    if (nmc.context.nonceSize == NONCE_SIZE && !isLocal) {
        uint8_t* decryptedDataStart;
        uint16_t decryptedDataLength;
        if (!unabto_connection_verify_and_decrypt_connect_packet(hdr, &decryptedDataStart, &decryptedDataLength)) {
            NABTO_LOG_ERROR(("Failed to read crypto payload in U_CONNECT"));
        } else {
            unabto_crypto_reinit_d(&con->cryptoctx, nmc.nabtoMainSetup.cryptoSuite, decryptedDataStart, decryptedDataLength);
        }
    } else
#endif
    {
        NABTO_LOG_TRACE(("########    U_CONNECT without crypto payload, this is expected for local connections"));
        unabto_crypto_reinit_d(&con->cryptoctx, CRYPT_W_NULL_DATA, 0, 0);
    }

    con->timeOut = CONNECTION_TIMEOUT;
    unabto_connection_set_future_stamp(&con->stamp, 20000); /* give much extra time during initialisation */

    if (!verify_connection_encryption(con)) {
        *ec = NOTIFY_ERROR_ENCR_MISMATCH;
        goto init_error;
    }

#if NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK
    if (!allow_client_access(con)) {
        *ec = NOTIFY_ERROR_CP_ACCESS;
        goto init_error;
    }
#endif

    if (con->cpEqual) {
        NABTO_LOG_DEBUG((PRInsi " U_CONNECT: addr:" PRIep " rendezvous:%" PRIu8, MAKE_NSI_PRINTABLE(0, *nsi, 0), MAKE_EP_PRINTABLE(con->cp.privateEndpoint), (uint8_t)(!con->noRendezvous)));
    } else {
        NABTO_LOG_DEBUG((PRInsi " U_CONNECT: private:" PRIep ", global:" PRIep " rendezvous:%" PRIu8, MAKE_NSI_PRINTABLE(0, *nsi, 0), MAKE_EP_PRINTABLE(con->cp.privateEndpoint), MAKE_EP_PRINTABLE(con->cp.globalEndpoint), (uint8_t)(!con->noRendezvous)));
    }

    NABTO_LOG_INFO(("Connection opened from '%s' (to %s). Encryption code %i. Fingerprint " PRIfp, con->clientId, nmc.nabtoMainSetup.id, con->cryptoctx.code, MAKE_FP_PRINTABLE(con->fingerprint)));

    return con;

init_error:
    nabto_release_connection(con);
    return 0;
}

/**
 * This code makes a connection unique, we have tried to make
 * connections through several channels. But when we receives the
 * first real data message on one channel we know that the client
 * ended up using this specific channel.
 */
void nabto_connection_end_connecting(nabto_connect* con, message_event* event) {
    if (event->type == MT_UDP) {
#if NABTO_ENABLE_TCP_FALLBACK
        if (con->hasTcpFallbackCapabilities) {
            unabto_tcp_fallback_close(con);
        }
#endif
        con->state = CS_CONNECTED;
        con->type = NCT_REMOTE_P2P;
        con->conAttr = CON_ATTR_DEFAULT;

        con->socket = event->udpMessage.socket;
        NABTO_LOG_TRACE(("Received data on socket %i", con->socket));

        con->peer = event->udpMessage.peer;
        if (EP_EQUAL(event->udpMessage.peer, nmc.context.gsp)) {
            NABTO_LOG_INFO((PRInsi " UDP Fallback through the GSP ", MAKE_NSI_PRINTABLE(0, con->spnsi, 0)));
        } else {
            NABTO_LOG_INFO((PRInsi " Direct UDP connection", MAKE_NSI_PRINTABLE(0, con->spnsi, 0)));
        }
    }

#if NABTO_ENABLE_TCP_FALLBACK    
    if (event->type == MT_TCP_FALLBACK) {
        con->state = CS_CONNECTED;
        con->type = NCT_REMOTE_RELAY_MICRO;
        con->conAttr |= con->fbConAttr;
        NABTO_LOG_INFO((PRInsi " TCP FAllback connection ", MAKE_NSI_PRINTABLE(0, con->spnsi, 0)));
    }
#endif

    nabto_rendezvous_end(con);

    con->sendConnectStatistics = true;
    // trigger recalculation of timeout such that statistics can be sent.
    connection_timeout_cache_cached = false;
} 

void nabto_connection_event(nabto_connect* con, message_event* event) {
    /**
     * If we end here we got a data packet on the connection
     */
    if (con->state == CS_CONNECTING) {
        nabto_connection_end_connecting(con, event);
    }

    if (con->state > CS_CONNECTING) {
        if (event->type == MT_UDP) {
            con->socket = event->udpMessage.socket;
            con->peer = event->udpMessage.peer;
        }
    }


}


/******************************************************************************/

void nabto_rendezvous_stop(nabto_connect* con) {
    nabto_rendezvous_connect_state* rcs = &con->rendezvousConnectState;
    
    if (rcs->state == RS_CONNECTING) {
        NABTO_LOG_INFO(("Extended rendezvous ports opened: %i, 0 is perfectly fine.", rcs->portsOpened));
        rcs->state = RS_DONE;
    }
}

void nabto_rendezvous_end(nabto_connect* con) {
    nabto_rendezvous_stop(con);
}


void nabto_rendezvous_start(nabto_connect* con) 
{
    nabto_rendezvous_connect_state* rcs = &con->rendezvousConnectState;
    rcs->state = RS_CONNECTING;

    unabto_connection_set_future_stamp(&rcs->timeout, 5000);
    unabto_connection_set_future_stamp(&rcs->timestamp, 0);

#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
    if (nmc.context.natType == NP_PAYLOAD_IPX_NAT_SYMMETRIC && nmc.nabtoMainSetup.enableExtendedRendezvousMultipleSockets) {
        con->rendezvousConnectState.openManySockets = true;
        unabto_connection_set_future_stamp(&con->rendezvousConnectState.openManySocketsStamp, 20);
    }
#endif

    if (con->clientNatType == NP_PAYLOAD_IPX_NAT_SYMMETRIC) {
        unabto_extended_rendezvous_init_port_sequence(&con->rendezvousConnectState.portSequence, con->cp.globalEndpoint.port);
        con->rendezvousConnectState.openManyPorts = true; 
        unabto_connection_set_future_stamp(&con->rendezvousConnectState.openManyPortsStamp, 0);
    }
    send_rendezvous_to_all(con);
}


/******************************************************************************/

bool rendezvous_event(message_event* event, nabto_packet_header* hdr)
{
    uint8_t*          begin = nabtoCommunicationBuffer + hdr->hlen;
    uint8_t*          end = nabtoCommunicationBuffer + hdr->len;

    nabto_endpoint  epUD;
    nabto_connect*  con   = 0;
    nabto_endpoint  src;
    uint32_t        interval = 5000;
    struct unabto_payload_packet payload;

    src = event->udpMessage.peer;
    NABTO_LOG_TRACE((PRInsi " Received from " PRIep " seq=%" PRIu16, MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0), MAKE_EP_PRINTABLE(src), hdr->seq));

    if (!unabto_find_payload(begin, end, NP_PAYLOAD_TYPE_EP, &payload)) {
        NABTO_LOG_TRACE(("Can't read first EP"));
        return false;
    }

    {
        struct unabto_payload_ep ep;
        if (!unabto_payload_read_ep(&payload, &ep)) {
            NABTO_LOG_TRACE(("Can't read first ep payload"));
            return false;
        }
        epUD.addr = ep.address;
        epUD.port = ep.port;
    }

    // try to read the second ep payload.
    if (unabto_find_payload(payload.dataEnd, end, NP_PAYLOAD_TYPE_EP, &payload)) {
        // The second EP is not neccessary since we read the destination ep from the udp packet.
    }

    // read nonce with an interval
    if (unabto_find_payload(begin, end, NP_PAYLOAD_TYPE_NONCE, &payload)) {
        if (payload.dataLength == 4) {
            READ_U32(interval, payload.dataBegin);
            NABTO_LOG_TRACE((PRInsi " Read interval from packet: %" PRIu32, MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0), interval));
            if (interval == 0) {
                NABTO_LOG_WARN(("Interval was 0, setting to 5000"));
                interval = 5000;
            }
        }
    }

    con  = nabto_find_connection(hdr->nsi_sp);

    if (!con) {
        NABTO_LOG_ERROR(("Connection was not found, nsi: %i", hdr->nsi_sp));
    } else {
        if (hdr->seq == 0) {
            send_rendezvous_socket(event->udpMessage.socket, con, 1, &src, &epUD);
        } else if (hdr->seq == 1) {
            send_rendezvous_socket(event->udpMessage.socket, con, 2, &src, &epUD);
            if (!conclude_connection(con, &src, interval)) {
                return false;
            }
        } else if (hdr->seq == 2) {
            if (!conclude_connection(con, &src, interval)) {
                return false;
            }
        } else {
            NABTO_LOG_ERROR(("Invalid Sequence Number"));
        }
    }
    return false;
}

bool conclude_connection(nabto_connect* con, nabto_endpoint* peer, uint32_t interval) {
    NABTO_LOG_TRACE(("conclude connection, %i", interval));
    if (con->rendezvousConnectState.state != RS_DONE) {
        con->timeOut = 7*interval/2;
        unabto_connection_set_future_stamp(&con->stamp, con->timeOut); // Give extra time in the connect phase.
    }
    nabto_rendezvous_stop(con);
    return true;
}

void send_rendezvous_to_all(nabto_connect* con) {
    send_rendezvous(con, 0, &con->cp.privateEndpoint, 0);
    send_rendezvous(con, 0, &con->cp.globalEndpoint, 0);
    unabto_connection_set_future_stamp(&con->rendezvousConnectState.timestamp, 1000);
}

void rendezvous_time_event(nabto_connect* con) 
{
    nabto_rendezvous_connect_state* rcs = &con->rendezvousConnectState;
    if (rcs->state == RS_CONNECTING) {
        if (nabtoIsStampPassed(&rcs->timestamp)) 
        {
            send_rendezvous_to_all(con);
        }

#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS         
        if (rcs->openManySockets &&
            nabtoIsStampPassed(&rcs->openManySocketsStamp))
        {
            if (rcs->socketsOpened < NABTO_EXTENDED_RENDEZVOUS_MAX_SOCKETS) {
                nabto_socket_t* candidate = &extended_rendezvous_sockets[rcs->socketsOpened];

                send_rendezvous_socket(*candidate, con, 0, &con->cp.globalEndpoint, 0);

                rcs->socketsOpened++;

                unabto_connection_set_future_stamp(&con->rendezvousConnectState.openManySocketsStamp, 20);
            } else {
                rcs->openManySockets = false;
            }
        }
#endif

        if (rcs->openManyPorts &&
            nabtoIsStampPassed(&rcs->openManyPortsStamp))
        {
            int i;
            for (i = 0; i < 10; i++) {
                nabto_endpoint newEp;
                uint16_t newPort = unabto_extended_rendezvous_next_port(&rcs->portSequence, rcs->portsOpened);
                
                newEp.addr = con->cp.globalEndpoint.addr;
                newEp.port = newPort;
                
                send_rendezvous(con, 0, &newEp, 0);
                rcs->portsOpened++;
            }
            unabto_connection_set_future_stamp(&rcs->openManyPortsStamp, 50);
        }

        if(nabtoIsStampPassed(&rcs->timeout)) {
#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
            NABTO_LOG_INFO(("Rendezvous timeout. Extended rendezvous sockets opened %i, 0 is perfectly fine", rcs->socketsOpened));
#endif
            nabto_rendezvous_stop(con);
        }
    }
} 

void statistics_time_event(nabto_connect* con) {
    if (con->sendConnectStatistics) {
        send_connection_statistics(con, NP_PAYLOAD_STATS_TYPE_DEVICE_CONNECTION_OPEN);
        con->sendConnectStatistics = false;
    }

    if (con->sendConnectionEndedStatistics) {
        send_connection_ended_statistics(con);
        con->sendConnectionEndedStatistics = false;
    }
}

void nabto_time_event_connection(void)
{
    // If the timeout is cached and the timeout is not passed then we
    // should not revisit the connection structure.
    bool nothingToDo = connection_timeout_cache_cached && !nabtoIsStampPassed(&connection_timeout_cache_stamp);
    if (nothingToDo) {
        return;
    } else {
        nabto_connect* con;
        connection_timeout_cache_cached = false;
        
        for (con = connections; con < connections + NABTO_MEMORY_CONNECTIONS_SIZE; ++con) {
            if (con->state != CS_IDLE) {
                rendezvous_time_event(con);
#if NABTO_ENABLE_TCP_FALLBACK
                if (con->hasTcpFallbackCapabilities) {
                    unabto_tcp_fallback_time_event(con);
                }
#endif
                
                if (nabto_connection_has_keep_alive(con) && nabtoIsStampPassed(&con->stamp)) {
                    NABTO_LOG_DEBUG((PRInsi " Connection timeout", MAKE_NSI_PRINTABLE(0, con->spnsi, 0))); //, Stamp value is: %ul", con->spnsi, con->stamp));
                    nabto_release_connection(con);
                }
                statistics_time_event(con);
            }
        }
    }
}

bool verify_connection_encryption(nabto_connect* con) {

    // Local connections can use whatever crypto they like.
    if (con->isLocal) {
        return true;
    } 
    
    // If we don't want encryption it's ok.
    if (!nmc.nabtoMainSetup.secureData) {
        return true;
    }

    // If secure data is chosen and the connection isn't local.
    // The encryption for the new connection should match the crypto suite
    // set in the setup.
    if (nmc.nabtoMainSetup.secureData && !con->isLocal) {
        if (con->cryptoctx.code == nmc.nabtoMainSetup.cryptoSuite) {
            return true;
        }
    }

    NABTO_LOG_ERROR(("The connection's encryption capabilities doesn't match the expected capabilities"));
    return false;
}

nabto_connection_type get_connection_type(nabto_connect* con) {
    if (con->isLocal) {
        return NCT_LOCAL;
    }
    if (con->state == CS_CONNECTING) {
        return NCT_CONNECTING;
    }
    if (con->state == CS_CONNECTED && con->type == NCT_REMOTE_RELAY_MICRO) {
        return NCT_REMOTE_RELAY_MICRO;
    }

    if (EP_EQUAL(con->peer, nmc.context.gsp)) {
        return NCT_REMOTE_RELAY;
    }
    return NCT_REMOTE_P2P;
}

uint16_t unabto_count_active_connections()
{
    uint16_t activeConnections = 0;
    nabto_connect* con;

    for (con = connections; con < connections + NABTO_MEMORY_CONNECTIONS_SIZE; ++con) {
        if (con->state != CS_IDLE) {
            activeConnections++;
        }
    }
    return activeConnections;
}

bool nabto_write_con(nabto_connect* con, uint8_t* buf, size_t len) {
    con->stats.packetsSent++;
    con->stats.bytesSent += len;
#if NABTO_ENABLE_TCP_FALLBACK
    if (con->type == NCT_REMOTE_RELAY_MICRO) {
        unabto_tcp_fallback_error status = unabto_tcp_fallback_write(con, buf, len);
        return (status == UTFE_OK);
    }
#endif

#if NABTO_ENABLE_DNS_FALLBACK
    if (nmc.nabtoMainSetup.enableDnsFallback) {
        if (EP_EQUAL(con->peer, nmc.context.gsp) && con->socket == nmc.socketGSP && nmc.context.useDnsFallback) {
            return (unabto_dns_fallback_send_to(buf, (uint16_t)len, con->peer.addr, con->peer.port) > 0);
        }
    }
#endif
    
    if (con->type == NCT_REMOTE_RELAY || con->type == NCT_REMOTE_P2P || con->type == NCT_LOCAL) {
        return (nabto_write(con->socket, buf, len, con->peer.addr, con->peer.port) > 0);
    }
    return false;
}

uint8_t* insert_rendezvous_stats_payload(uint8_t* ptr, uint8_t* end, nabto_connect* con) {
    UNABTO_ASSERT(ptr <= end);
    if (end-ptr < NP_PAYLOAD_RENDEZVOUS_STATS_BYTELENGTH) {
        return NULL;
    }
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_RENDEZVOUS_STATS, 0, 7);
    
    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_RENDEZVOUS_STATS_VERSION);
    WRITE_FORWARD_U8(ptr, con->clientNatType);
    WRITE_FORWARD_U8(ptr, nmc.context.natType);
    WRITE_FORWARD_U16(ptr, con->rendezvousConnectState.portsOpened);
    WRITE_FORWARD_U16(ptr, con->rendezvousConnectState.socketsOpened);

    return ptr;
}

uint8_t* insert_cp_id_payload(uint8_t* ptr, uint8_t* end, nabto_connect* con) {
    size_t cpIdLength = strlen(con->clientId);

    UNABTO_ASSERT(ptr <= end);
    if ((size_t)(end - ptr) < NP_PAYLOAD_CP_ID_BYTELENGTH + cpIdLength) {
        return NULL;
    }

    ptr = insert_payload(ptr, end,  NP_PAYLOAD_TYPE_CP_ID, 0, 1+cpIdLength);

    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_CP_ID_TYPE_MAIL);
    memcpy(ptr, (const void*) con->clientId, cpIdLength);
    
    ptr += cpIdLength;

    return ptr;
}

uint8_t* insert_connection_info_payload(uint8_t* ptr, uint8_t* end, nabto_connect* con) {
    nabto_stamp_t now;
    uint32_t connectionAge;
    uint8_t connectionType;
    uint8_t* payloadBegin = ptr;

    now = nabtoGetStamp();
    connectionAge = nabtoStampDiff2ms(nabtoStampDiff(&now, &con->stats.connectionStart));

    switch (get_connection_type(con)) {
        case NCT_LOCAL:              connectionType = NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_LOCAL; break;
        case NCT_CONNECTING:         connectionType = NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_CONNECTING; break;
        case NCT_REMOTE_RELAY:       connectionType = NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_UDP_RELAY; break;
        case NCT_REMOTE_RELAY_MICRO: connectionType = NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_TCP_RELAY; break;
        case NCT_REMOTE_P2P:         connectionType = NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_P2P; break;
        default:                     connectionType = NP_PAYLOAD_CONNECT_STATS_CONNECTION_TYPE_CONNECTING; break;
    }

    if (ptr == NULL) {
        return NULL;
    }
    
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_CONNECTION_INFO, 0, 0);

    ptr = unabto_stats_write_u8(ptr, end, NP_PAYLOAD_CONNECTION_INFO_TYPE, connectionType);
    ptr = unabto_stats_write_u32(ptr, end, NP_PAYLOAD_CONNECTION_INFO_DURATION, connectionAge);
    ptr = unabto_stats_write_u32(ptr, end, NP_PAYLOAD_CONNECTION_INFO_SENT_PACKETS, con->stats.packetsSent);
    ptr = unabto_stats_write_u32(ptr, end, NP_PAYLOAD_CONNECTION_INFO_SENT_BYTES, con->stats.bytesSent);
    ptr = unabto_stats_write_u32(ptr, end, NP_PAYLOAD_CONNECTION_INFO_RECEIVED_PACKETS, con->stats.packetsReceived);
    ptr = unabto_stats_write_u32(ptr, end, NP_PAYLOAD_CONNECTION_INFO_RECEIVED_BYTES, con->stats.bytesReceived);

    if (con->cp.globalEndpoint.addr != 0x00000000) {
        ptr = unabto_stats_write_u32(ptr, end, NP_PAYLOAD_CONNECTION_INFO_CLIENT_IP, con->cp.globalEndpoint.addr);
    }

    if (ptr != NULL) {
        WRITE_U16(payloadBegin + 2, ptr - payloadBegin);
    }
    
    return ptr;
}

void send_connection_statistics(nabto_connect* con, uint8_t event) {
    size_t length;
    uint8_t* ptr = insert_header(nabtoCommunicationBuffer, 0, con->spnsi, NP_PACKET_HDR_TYPE_STATS, false, 0, 0, 0);
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    


    ptr = insert_stats_payload(ptr, end, event);
    if (ptr == NULL) {
        return;
    }
    ptr = insert_version_payload(ptr, end);
    if (ptr == NULL) {
        return;
    }

#if NABTO_ENABLE_CLIENT_ID
    ptr = insert_cp_id_payload(ptr, end, con);
    if (ptr == NULL) {
        return;
    }
#endif

    ptr = insert_sp_id_payload(ptr, end);
    if (ptr == NULL) {
        return;
    }

    ptr = insert_ipx_payload(ptr, end);
    if (ptr == NULL) {
        return;
    }

    if (!con->noRendezvous) {
        ptr = insert_rendezvous_stats_payload(ptr, end, con);
        if (ptr == NULL) {
            return;
        }
    }

    ptr = insert_connection_info_payload(ptr, end, con);
    if (ptr == NULL) {
        return;
    }

    length = ptr - nabtoCommunicationBuffer;
    insert_length(nabtoCommunicationBuffer, length);
    send_to_basestation(nabtoCommunicationBuffer, length, &nmc.context.gsp);
}

void send_connection_ended_statistics(nabto_connect* con) {
    size_t length;
    uint8_t* ptr = insert_header(nabtoCommunicationBuffer, 0, con->spnsi, NP_PACKET_HDR_TYPE_STATS, false, 0, 0, 0);
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    ptr = insert_stats_payload(ptr, end, NP_PAYLOAD_STATS_TYPE_DEVICE_CONNECTION_CLOSE);
    if (ptr == NULL) {
        return;
    }

    ptr = insert_version_payload(ptr, end);
    if (ptr == NULL) {
        return;
    }

#if NABTO_ENABLE_CLIENT_ID
    ptr = insert_cp_id_payload(ptr, end, con);
    if (ptr == NULL) {
        return;
    }
#endif

    ptr = insert_sp_id_payload(ptr, end);
    if (ptr == NULL) {
        return;
    }
    ptr = insert_connection_info_payload(ptr, end, con);
    if (ptr == NULL) {
        return;
    }

    length = ptr - nabtoCommunicationBuffer;
    insert_length(nabtoCommunicationBuffer, length);
    
    send_to_basestation(nabtoCommunicationBuffer, length, &nmc.context.gsp);
}


#endif
