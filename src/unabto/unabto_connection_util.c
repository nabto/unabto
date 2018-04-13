#include <unabto/unabto_connection_util.h>
#include <unabto/unabto_packet_util.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_types.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_util.h>




// return true iff a client id was read
bool unabto_connection_util_read_client_id(const nabto_packet_header* header, nabto_connect* con)
{
    struct unabto_payload_packet cpIdPayload;
    
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_end(nabtoCommunicationBuffer, header);
    
    if (!unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_CP_ID, &cpIdPayload)) {
        return false;
    }

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
        } else {
            NABTO_LOG_WARN(("unknown cpid type"));
            return false;
        }
        return true;
    }

    return false;
}

// return true iff a fingerprint was read
bool unabto_connection_util_read_fingerprint(const nabto_packet_header* header, nabto_connect* con)
{
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_end(nabtoCommunicationBuffer, header);

    struct unabto_payload_packet fingerprintPayload;
    if (unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_FP, &fingerprintPayload)) {
        struct unabto_payload_typed_buffer fingerprint;
        if (unabto_payload_read_typed_buffer(&fingerprintPayload, &fingerprint)) {
            if (fingerprint.type == NP_PAYLOAD_FP_TYPE_SHA256_TRUNCATED) {
                if (fingerprint.dataLength  ==  NP_TRUNCATED_SHA256_LENGTH_BYTES) {
                    con->fingerprint.hasValue = true;
                    memcpy(con->fingerprint.value.fp, fingerprint.dataBegin, FINGERPRINT_LENGTH);
                    return true;
                } else {
                    NABTO_LOG_ERROR(("fingerprint has the wrong length %"PRIu16, fingerprint.dataLength));
                }
            } else {
                NABTO_LOG_TRACE(("cannot read fignerprint type: %"PRIu8, fingerprint.type));
            }
        }
    }
    return false;
}

// read unencrypted client nonce
bool unabto_connection_util_read_nonce_client(const nabto_packet_header* header, nabto_connect* connection)
{
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_end(nabtoCommunicationBuffer, header);

    struct unabto_payload_packet noncePayload;
    if (unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_NONCE, &noncePayload)) {
        if (noncePayload.dataLength != 32) {
            return false;
        }
        memcpy(connection->psk.handshakeData.initiatorNonce, noncePayload.dataBegin, 32);
        return true;
    }

    return false;
}

// read encrypted client random
bool unabto_connection_util_read_random_client(const uint8_t* payloadsBegin, const uint8_t* payloadsEnd, nabto_connect* connection)
{
    struct unabto_payload_packet randomPayload;
    if (unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_RANDOM, &randomPayload)) {
        if (randomPayload.dataLength != 32) {
            return false;
        }
        memcpy(connection->psk.handshakeData.initiatorRandom, randomPayload.dataBegin, 32);
        return true;
    }

    return false;
}

// read and validate nonce from client in packet
bool unabto_connection_util_read_and_validate_nonce_client(const uint8_t* payloadsBegin, const uint8_t* payloadsEnd, nabto_connect* connection)
{
    struct unabto_payload_packet noncePayload;
    if (unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_NONCE, &noncePayload)) {
        if (noncePayload.dataLength != 32) {
            return false;
        }
        if (memcmp(connection->psk.handshakeData.responderNonce, noncePayload.dataBegin, 32) == 0) {
            return true;
        }
    }

    return false;
}


// read key id
bool unabto_connection_util_read_key_id(const nabto_packet_header* header, nabto_connect* connection)
{
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_end(nabtoCommunicationBuffer, header);

    struct unabto_payload_packet keyIdPayload;
    if (unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_KEY_ID, &keyIdPayload)) {
        if (keyIdPayload.dataLength != 16) {
            return false;
        }
        memcpy(connection->psk.keyId.pskId, keyIdPayload.dataBegin, 16);
        return true;
    }

    return false;
}

// read capabilities
bool unabto_connection_util_read_capabilities(const nabto_packet_header* header, struct unabto_payload_capabilities_read* capabilities)
{
    struct unabto_payload_packet capabilitiesPayload;
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_end(nabtoCommunicationBuffer, header);
    
    if (unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_CAPABILITY, &capabilitiesPayload)) {
        return unabto_payload_read_capabilities(&capabilitiesPayload, capabilities);
    }
    return false;
}

bool unabto_connection_util_verify_capabilities(nabto_connect* connection, struct unabto_payload_capabilities_read* capabilities)
{
    uint32_t requiredCapabilities = PEER_CAP_TAG | PEER_CAP_FRCTRL | PEER_CAP_ASYNC;

    if ((capabilities->mask & requiredCapabilities) != requiredCapabilities) {
        NABTO_LOG_WARN(("client does not provide enough capabilities in its capability mask"));
        return false;
    }
    if ((capabilities->bits & requiredCapabilities) != requiredCapabilities) {
        NABTO_LOG_WARN(("client does not provide enough capabilities in its capability bits"));
        return false;
    }

    bool codeFound = false;
    uint16_t i;
    const uint8_t* ptr = capabilities->codesStart;
    for (i = 0; i < capabilities->codesLength; i++) {
        uint16_t code;
        READ_FORWARD_U16(code, ptr);
        if (code == CRYPT_W_AES_CBC_HMAC_SHA256) {
            codeFound = true;
            break;
        }
    }
    if (!codeFound) {
        NABTO_LOG_WARN(("no suitable encryption code is found in psk connect packet"));
        return false;
    }

    uint32_t supportedCapabilities = requiredCapabilities | PEER_CAP_FP;

    connection->psk.capabilities.type = 0;
    // limit the mask to capabilities we understand
    connection->psk.capabilities.mask = supportedCapabilities;

    // limit the proposed capabilities to capabilities we understand.
    connection->psk.capabilities.bits = (capabilities->bits & supportedCapabilities);

    return true;    
}

#if NABTO_ENABLE_LOCAL_PSK_CONNECTION
bool unabto_connection_util_psk_connect_init_key(nabto_connect* connection)
{
    struct unabto_psk psk;
    if (!unabto_local_psk_connection_get_key(&connection->psk.keyId, connection->clientId, &connection->fingerprint, &psk))
    {
        return false;
    }

    // init crypto context given a 128bit key.

    nabto_crypto_init_aes_128_hmac_sha256_psk_context(&connection->cryptoctx, psk.psk);
    return true;
}
#endif

bool unabto_psk_connection_util_verify_connect(const nabto_packet_header* header, nabto_connect* connection)
{
    // packet structure
    //U_CONNECT_PSK(Hdr, Capabilities, CpId, Fingerprint, KeyId, nonce_client, Enc())

    // find crypto payload
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_end(nabtoCommunicationBuffer, header);

    struct unabto_payload_packet cryptoPayload;
    struct unabto_payload_crypto crypto;
    uint16_t verifSize;
    if (!unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_CRYPTO, &cryptoPayload)) {
        return false;
    }

    if (!unabto_payload_read_crypto(&cryptoPayload, &crypto)) {
        return false;
    }

    // validate mac on packet
    return unabto_verify_integrity(&connection->cryptoctx, crypto.code, nabtoCommunicationBuffer, header->len, &verifSize);
}
