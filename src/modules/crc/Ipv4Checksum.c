/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "Ipv4Checksum.h"

uint16_t Ipv4Checksum_Calculate(uint8_t* buffer, uint16_t offset, uint16_t length)
{
    uint16_t i;
    uint32_t sum = 0;
    uint16_t evenLength = length & ~1;

    for (i = offset; i < (offset + evenLength); i += 2)
    {
        sum += (buffer[i] << 8) + buffer[i + 1];
    }

    if (length != evenLength)
    {
        sum += (buffer[offset + evenLength] << 8);
    }

    while (sum > 0xffff)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return (uint16_t)~sum;
}