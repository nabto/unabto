/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NSLP

#include "nslp_binary_transport.h"
#include "nslp_binary_link.h"
#include <unabto/unabto_util.h>
#include "nslp_communications_port.h"
#include <unabto/unabto_external_environment.h>

#ifndef NSLP_BINARY_TRANSPORT_TRANSMISSION_TIMEOUT
#define NSLP_BINARY_TRANSPORT_TRANSMISSION_TIMEOUT            1000
#endif

#ifndef NSLP_BINARY_TRANSPORT_MAXIMUM_TRANSMISSION_ATTEMPTS
#define NSLP_BINARY_TRANSPORT_MAXIMUM_TRANSMISSION_ATTEMPTS   5
#endif

#define PROTOCOL_VERSION                                      0

// the flags sent in the NSLP binary transport packet header
#define FLAG_NONE                                             0x00
//   the packet was sent from the server
#define FLAG_FROM_GATEWAY                                     0x01

static void tick_transmitter(void);
static void send_queued_packet(void);
static void release_queued_packet(void);
static uint8_t calculate_checksum(uint16_t startingIndex, uint16_t length);
static bool verify_integrity(uint8_t* buffer, uint16_t length);

// transmitter
//   general
#if UNABTO_PLATFORM_PIC18
#pragma udata big_mem
#endif

static uint8_t txQueueBuffer[NSLP_BINARY_TRANSPORT_TRANSMISSION_QUEUE_SIZE];

#if UNABTO_PLATFORM_PIC18
#pragma udata
#endif

static uint16_t txQueueHead;
static uint16_t txQueueTail;
static uint16_t txQueueFree;

//   stuff for the packet currently being constructed by the upper layer
static uint16_t txCurrentPacketSize;
static uint16_t txCurrentUsed;
static uint16_t txCurrentIndex;

//   packet currently being transmitted
static bool transmitterActive; // true if the transmitter is currently sending a packet
static bool txFailed; // true if a packet was dropped (latches high - cleared by reading its status using the function XXX)
static uint8_t txSequenceNumber; // sequence number of the packet currently being transmitted or the next sequence number to use if not currently transmitting
static nabto_stamp_t txTimeout; // time stamp indicating when the current transmission attempt will time out
static uint8_t txAttempts; // number of times the current transmission has timed out
static uint16_t transmitterPacketSize; // size of the packet currently being handled by the transmitter logic
static uint8_t transmitterPacketSequenceNumber; // sequence number of the packet currently being handled by the transmitter logic

// receiver
static uint8_t acknowledingSequenceNumber;

void nslp_binary_transport_initialize(void)
{
  nslp_binary_transport_reset();
}

void nslp_binary_transport_reset(void)
{
  txQueueHead = 0;
  txQueueTail = 0;
  txQueueFree = sizeof (txQueueBuffer);
  transmitterActive = false;
  txFailed = false;
  txSequenceNumber = 0; // first packet after reset is sent with seq# 0 to sync devices
  txCurrentPacketSize = 0;
  txCurrentUsed = 0;
  txCurrentIndex = 0;

  acknowledingSequenceNumber = 0;
}

void nslp_binary_transport_tick(void)
{
  tick_transmitter();
}

bool nslp_binary_transport_is_idle(void)
{
  return transmitterActive == false;
}

bool nslp_binary_transport_allocate_packet(uint16_t size)
{
  uint16_t totalSize = 2 + NSLP_BINARY_TRANSPORT_PACKET_OVERHEAD + size; // the total queue size required is: size of packet indicator (uint16) + packet header and checksum + requested payload size

  // abort if there is not enough free space in the transmission queue for the requested packet
  if(txQueueFree < totalSize)
  {
    return false;
  }

  // setup the current-packet variables
  txCurrentIndex = txQueueHead; // set the index of the next byte of the packet to the head of the transmission queue
  txCurrentPacketSize = totalSize - 2; // save the number of bytes in the newly allocated packet - not including the size specifier
  txCurrentUsed = 0; // no bytes used yet

  // consume the bytes from the transmission queue
  txQueueHead = (txQueueHead + totalSize) % sizeof (txQueueBuffer);
  txQueueFree -= totalSize;

  nslp_binary_transport_write_uint16(txCurrentPacketSize); // write size specifier to transmission queue
  txCurrentUsed = 0; // reset used counter as it should not include the size specifier

  // write NSLP binary transport layer header
  //   protocol version
  nslp_binary_transport_write_uint8(PROTOCOL_VERSION);
  //   flags
#if NSLP_GATEWAY
  nslp_binary_transport_write_uint16(FLAG_FROM_GATEWAY);
#else
  nslp_binary_transport_write_uint16(FLAG_NONE);
#endif
  //   acknowleding sequence number
  nslp_binary_transport_write_uint8(acknowledingSequenceNumber);
  //   sequence number
  nslp_binary_transport_write_uint8(txSequenceNumber);
  //   when sending payload consume a sequence number
  if(size > 0)
  {
    if(++txSequenceNumber == 0)
    {
      txSequenceNumber = 1;
    }
  }

  NABTO_LOG_INFO(("Allocated packet length=2+%u+%u", NSLP_BINARY_TRANSPORT_PACKET_OVERHEAD, size));

  return true;
}

void nslp_binary_transport_write_uint8(uint8_t value)
{
  if(txCurrentPacketSize == txCurrentUsed)
  {
    NABTO_LOG_ERROR(("Packet overrun!"));
    return;
  }

  txQueueBuffer[txCurrentIndex] = value;
  txCurrentIndex = (txCurrentIndex + 1) % sizeof (txQueueBuffer);

  txCurrentUsed++;
}

void nslp_binary_transport_write_uint16(uint16_t value)
{
  nslp_binary_transport_write_uint8((uint8_t) (value >> 8));
  nslp_binary_transport_write_uint8((uint8_t) value);
}

void nslp_binary_transport_write_uint32(uint32_t value)
{
  nslp_binary_transport_write_uint8((uint8_t) (value >> 24));
  nslp_binary_transport_write_uint8((uint8_t) (value >> 16));
  nslp_binary_transport_write_uint8((uint8_t) (value >> 8));
  nslp_binary_transport_write_uint8((uint8_t) value);
}

void nslp_binary_transport_write_buffer(const void* buffer, uint16_t length)
{
  uint8_t* p = (uint8_t*) buffer;

  while(length--)
  {
    nslp_binary_transport_write_uint8(*p++);
  }
}

// receives commands from remote node.

uint16_t nslp_binary_transport_receive(uint8_t** packet)
{
  uint16_t frameLength;
  uint8_t* frame;
  uint8_t protocolVersion;
  uint8_t flags;
  uint8_t sSequenceNumber;
  uint8_t aSequenceNumber;
  uint16_t payloadLength;

  // check for incoming commands and acknowledges
  frameLength = nslp_binary_link_receive_frame(&frame); // poll link layer for a frame
  if(frameLength == 0) // nothing received
  {
    return 0;
  }

  NABTO_LOG_TRACE(("Verifying incoming packet..."));

  if(frameLength < NSLP_BINARY_TRANSPORT_PACKET_OVERHEAD) // a frame must contain at least the header and the checksum field to be valid
  {
    NABTO_LOG_INFO(("Packet ignored - too small!"));
    return 0;
  }

  // verify frame integrity
  if(verify_integrity(frame, frameLength) == false)
  {
    NABTO_LOG_INFO(("Packet ignored - bad checksum!"));
    return 0;
  }

  // process header

  //   protocol version
  READ_FORWARD_U8(protocolVersion, frame);
  if(protocolVersion != PROTOCOL_VERSION)
  {
    NABTO_LOG_INFO(("Packet ignored - unsupported protocol version!\n"));
    return 0;
  }

  //   flags
  READ_FORWARD_U16(flags, frame);

#if NSLP_GATEWAY
  if((flags & FLAG_FROM_GATEWAY) != 0) // ignore frames not from client
#else
  if((flags & FLAG_FROM_GATEWAY) == 0) // ignore frames not from server
#endif
  {
    NABTO_LOG_INFO(("Packet ignored - I sent it!"));
    return 0;
  }

  //   acknowledging sequence number
  READ_FORWARD_U8(aSequenceNumber, frame);
  NABTO_LOG_TRACE(("acknowleding sequence number=%u", aSequenceNumber));
  if(aSequenceNumber == 0)
  {
    NABTO_LOG_TRACE(("Remote device has not received a packet yet."));
  }
  else
  {
    if(aSequenceNumber == (transmitterPacketSequenceNumber + 1) || (aSequenceNumber == 1 && transmitterPacketSequenceNumber == 255)) // correct acknowledgement of current packet (remote node sends next expected sequence number)
    {
      if(transmitterActive)
      {
        release_queued_packet();
        transmitterActive = false;

        NABTO_LOG_INFO(("Acknowledge received (sequence number=%u).", (int) transmitterPacketSequenceNumber));
      }
    }
    else
    {
      NABTO_LOG_TRACE(("Not acknowledging anything (sequence numbers: expected=%u actual=%u).", (int) transmitterPacketSequenceNumber, (int) aSequenceNumber));
    }
  }

  //   senders sequence number
  READ_FORWARD_U8(sSequenceNumber, frame);
  NABTO_LOG_TRACE(("senders sequence number=%u", sSequenceNumber));

  payloadLength = frameLength - NSLP_BINARY_TRANSPORT_PACKET_OVERHEAD;
  if(payloadLength > 0)
  {
    if(sSequenceNumber == acknowledingSequenceNumber || sSequenceNumber == 0 || acknowledingSequenceNumber == 0) // new packet or the first packet after the remote node was started or the first packet after this node was started
    {
      NABTO_LOG_INFO(("Packet received: sequence number=%u length=%u", (int) sSequenceNumber, (int) payloadLength));

      acknowledingSequenceNumber = sSequenceNumber + 1; // step sequence number in a way that also handles 0 values
      // sequence number 0 is used for synchronization and can't be used during normal communication
      if(acknowledingSequenceNumber == 0)
      {
        acknowledingSequenceNumber++;
      }

      // TODO quick ack might not be necessary...
      NABTO_LOG_INFO(("Sending acknowledge."));
      nslp_binary_transport_allocate_packet(0); // send a packet with no payload to send acknowledge
      tick_transmitter();

      // return command part of the packet (header has been stripped at this point)
      *packet = frame;
      return payloadLength;
    }
    else // possible duplicate packet. sending an empty packet acknowledges this
    {
      NABTO_LOG_TRACE(("Possible duplicate packet received (sequence number=%u).", (int) sSequenceNumber));

      NABTO_LOG_TRACE(("Sending duplicate acknowledge."));
      nslp_binary_transport_allocate_packet(0); // send a packet with no payload to resend acknowledge

      return 0; // don't pass duplicate packets to application
    }
  }
  else // packet didn't contain any payload -> it was just an acknowledge
  {
    return 0;
  }
}

uint8_t nslp_binary_transport_poll_send_status(void)
{
  if(txFailed)
  {
    txFailed = false;
    return true;
  }
  else
  {
    return false;
  }
}

static void tick_transmitter(void)
{
  // verify that the application has not left an invalid packet in the transmission queue
  if(txCurrentPacketSize > 0) // was a packet allocated?
  {
    if(txCurrentPacketSize != (txCurrentUsed + 1)) // has fewer or more bytes than expected been written (excluding the checksum which will be added below)?
    {
      NABTO_LOG_ERROR(("Application left invalid packet in tx queue!"));
    }
  }

  if(transmitterActive == false) // not currently transmitting a packet
  {
    if(txQueueFree < sizeof (txQueueBuffer)) // if at this point some data has been allocated from the transmission buffer there is a full and valid packet ready to be sent
    {
      uint16_t index;
      uint8_t checksum;

      // read size of packet and advance past size specifier
      transmitterPacketSize = txQueueBuffer[txQueueTail];
      txQueueTail = (txQueueTail + 1) % sizeof (txQueueBuffer);
      transmitterPacketSize <<= 8;
      transmitterPacketSize |= txQueueBuffer[txQueueTail];
      txQueueTail = (txQueueTail + 1) % sizeof (txQueueBuffer);

      txQueueFree += 2; // size specifier has now been removed from the transmission queue

      // read sequence number (offset 3) without advancing
      index = (txQueueTail + 4) % sizeof (txQueueBuffer);
      transmitterPacketSequenceNumber = txQueueBuffer[index];

      // calculate and append checksum
      checksum = calculate_checksum(txQueueTail, transmitterPacketSize - 1); // don't include the checksum field when calculating the checksum
      index = (txQueueTail + transmitterPacketSize - 1) % sizeof (txQueueBuffer);
      txQueueBuffer[index] = checksum;

      send_queued_packet();

      if(transmitterPacketSize == NSLP_BINARY_TRANSPORT_PACKET_OVERHEAD)
      {
        NABTO_LOG_INFO(("Transmitting acknowledge packet."));

        release_queued_packet();
        transmitterActive = false;
      }
      else
      {
        NABTO_LOG_INFO(("Transmitting payload packet."));

        txAttempts = 0;
        nabtoSetFutureStamp(&txTimeout, NSLP_BINARY_TRANSPORT_TRANSMISSION_TIMEOUT);
        transmitterActive = true;
      }
    }
  }
  else // currently transmitting a packet
  {
    if(nabtoIsStampPassed(&txTimeout)) // timeout occured?
    {
      if(++txAttempts >= NSLP_BINARY_TRANSPORT_MAXIMUM_TRANSMISSION_ATTEMPTS) // to many attempts?
      {
        release_queued_packet();

        transmitterActive = false;
        txFailed = true;

        NABTO_LOG_INFO(("Packet dropped - too many retransmissions!"));
      }
      else // retransmit
      {
        send_queued_packet();

        nabtoSetFutureStamp(&txTimeout, NSLP_BINARY_TRANSPORT_TRANSMISSION_TIMEOUT);

        NABTO_LOG_INFO(("Retransmitting packet!"));
      }
    }
  }
}

static void send_queued_packet(void)
{
  uint16_t size = transmitterPacketSize;
  uint16_t index = txQueueTail;

  // construct the frame
  nslp_binary_link_start_frame();

  while(size--)
  {
    nslp_binary_link_write_to_frame(txQueueBuffer[index]);
    index = (index + 1) % sizeof (txQueueBuffer);
  }

  nslp_binary_link_end_frame();
}

static void release_queued_packet(void)
{
  txQueueTail = (txQueueTail + transmitterPacketSize) % sizeof (txQueueBuffer); // advance the tail index to the next packet.
  txQueueFree += transmitterPacketSize; // release memory allocated for current packet

  NABTO_LOG_TRACE(("Current packet released."));
}

static uint8_t calculate_checksum(uint16_t startingIndex, uint16_t length)
{
  uint8_t checksum = 0;

  while(length--)
  {
    checksum += txQueueBuffer[startingIndex];
    startingIndex = (startingIndex + 1) % sizeof (txQueueBuffer);
  }

  return (checksum ^ 0xff) + 1;
}

static bool verify_integrity(uint8_t* buffer, uint16_t length)
{
  uint8_t checksum = 0;

  while(length--)
  {
    checksum += *buffer++;
  }

  return checksum == 0;
}
