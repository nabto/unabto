#include <unabto/unabto_connection_util.h>
#include <unabto/unabto_packet_util.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_types.h>
#include <unabto/unabto_app.h>




// return true iff a client id was read
bool unabto_connection_util_read_client_id(const nabto_packet_header* header, nabto_connect* con)
{
    struct unabto_payload_packet cpIdPayload;
    
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    
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
    uint8_t* payloadsEnd = unabto_payloads_begin(nabtoCommunicationBuffer, header);

    struct unabto_payload_packet fingerprintPayload;
    if (unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_FP, &fingerprintPayload)) {
        struct unabto_payload_typed_buffer fingerprint;
        if (unabto_payload_read_typed_buffer(&fingerprintPayload, &fingerprint)) {
            if (fingerprint.type == NP_PAYLOAD_FP_TYPE_SHA256_TRUNCATED) {
                if (fingerprint.dataLength  ==  NP_TRUNCATED_SHA256_LENGTH_BYTES) {
                    con->hasFingerprint = true;
                    memcpy(con->fingerprint, fingerprint.dataBegin, NP_TRUNCATED_SHA256_LENGTH_BYTES);
                    return true;
                } else {
                    NABTO_LOG_ERROR(("fingerprint has the wrong length %"PRIu16, fingerprint.dataLength));
                }
            } else {
                NABTO_LOG_TRACE(("cannot read fignerprint type: %"PRIu8, fingerprint.type));
            }
        }
    }
}

// read unencrypted client nonce
bool unabto_connection_util_read_nonce_client(const nabto_packet_header* header, nabto_connect* connection)
{
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_begin(nabtoCommunicationBuffer, header);

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

// read key id
bool unabto_connection_util_read_key_id(const nabto_packet_header* header, nabto_connect* connection)
{
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_begin(nabtoCommunicationBuffer, header);

    struct unabto_payload_packet keyIdPayload;
    if (unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_KEY_ID, &keyIdPayload)) {
        if (keyIdPayload.dataLength != 16) {
            return false;
        }
        memcpy(connection->psk.keyId, keyIdPayload.dataBegin, 16);
        return true;
    }

    return false;
}

// read capabilities
bool unabto_connection_util_read_capabilities(const nabto_packet_header* header, nabto_connect* connection)
{
    return false;
}


bool unabto_connection_util_psk_connect_init_key(const nabto_packet_header* header, nabto_connect* connection)
{
    unabto_psk psk;
    if (!unabto_local_psk_connection_get_key(connection->psk.keyId, connection->clientId, connection->fingerprint, psk))
    {
        return false;
    }

    // init crypto context given a 128bit key.

    nabto_crypto_init_aes_128_hmac_sha256_psk_context(&connection->cryptoctx, psk);
    return true;
}

bool unabto_psk_connection_util_verify_connect(const nabto_packet_header* header, nabto_connect* connection)
{
    // packet structure
    //U_CONNECT_PSK(Hdr, Capabilities, CpId, Fingerprint, KeyId, nonce_client, Enc())

    // find crypto payload
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_begin(nabtoCommunicationBuffer, header);

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
