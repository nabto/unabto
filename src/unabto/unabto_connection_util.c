#include "unabto_connection_util.h"

bool unabto_connection_util_read_client_id(const nabto_packet_header* header, nabto_connect* con)
{
    unabto_payload_packet cpIdPayload;
    
    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    
    if (!unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_CP_ID, &cpIdPayload)) {
        return;
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



bool unabto_psk_connection_util_verify_connect(const nabto_packet_header* header, )
{
    // packet structure
    //U_CONNECT_PSK(Hdr, Capabilities, CpId, Fingerprint, KeyId, nonce_client, Enc())

    // read key id

    uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* payloadsEnd = unabto_payloads_begin(nabtoCommunicationBuffer, header);

    unabto_payload_packet keyIdPayload;
    unabto_psk_id keyId;
    const char* clientId;
    if (!unabto_find_payload(payloadsBegin, payloadsEnd, NP_PAYLOAD_TYPE_KEY_ID, &keyIdPayload)) {
        NABTO_LOG_WARN(("missing key id payload"));
        return;
    }
    
    
    
    if (keyIdPayload->dataLength != 16) {
        NABTO_LOG_WARN(("invalid key id length"));
        return;
    }

    uint8_t psk[16];
    bool unabto_local_psk_connection_get_key(keyIdPayload->dataBegin, const char* clientId, unabto_public_key_fingerprint fingerprint, unabto_psk key);
    
    // get crypto key

    // initialize crypto context

    // validate packet

    
}
