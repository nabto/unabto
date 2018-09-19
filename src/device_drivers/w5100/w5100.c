/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_DEVICE_DRIVER

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_util.h>
#include "w5100.h"
#include "w5100_platform.h"
#include <string.h>

// <editor-fold defaultstate="collapsed" desc="Register, bit and constant definitions">

#define OPCODE_WRITE                                                0xf0
#define OPCODE_READ                                                 0x0f

#define REGISTER_MODE                                               0x0000
#define REGISTER_GATEWAY_ADDRESS                                    0x0001
#define REGISTER_SUBNET_MASK_ADDRESS                                0x0005
#define REGISTER_SOURCE_HARDWARE_ADDRESS                            0x0009
#define REGISTER_SOURCE_IP_ADDRESS                                  0x000f
#define REGISTER_INTERRUPT                                          0x0015
#define REGISTER_INTERRUPT_MASK                                     0x0016
#define REGISTER_RETRY_TIME                                         0x0017
#define REGISTER_RETRY_COUNT                                        0x0019
#define REGISTER_RX_MEMORY_SIZE                                     0x001a
#define REGISTER_TX_MEMORY_SIZE                                     0x001b
#define REGISTER_PPPOE_AUTHENTICATION_TYPE                          0x001c
#define REGISTER_PPP_LCP_REQUEST_TIMER                              0x0028
#define REGISTER_PPP_LCP_MAGIC_NUMBER                               0x0029
#define REGISTER_UNREACHABLE_IP_ADDRESS                             0x002a
#define REGISTER_UNREACHABLE_PORT                                   0x002e

#define REGISTER_SOCKET_MODE(index)                                 (0x0400 + (index * 0x0100))
#define REGISTER_SOCKET_COMMAND(index)                              (0x0401 + (index * 0x0100))
#define REGISTER_SOCKET_INTERRUPT(index)                            (0x0402 + (index * 0x0100))
#define REGISTER_SOCKET_STATUS(index)                               (0x0403 + (index * 0x0100))
#define REGISTER_SOCKET_SOURCE_PORT(index)                          (0x0404 + (index * 0x0100))
#define REGISTER_SOCKET_DESTINATION_HARDWARE_ADDRESS(index)         (0x0406 + (index * 0x0100))
#define REGISTER_SOCKET_DESTINATION_IP_ADDRESS(index)               (0x040c + (index * 0x0100))
#define REGISTER_SOCKET_DESTINATION_PORT(index)                     (0x0410 + (index * 0x0100))
#define REGISTER_SOCKET_MAXIMUM_SEGMENT_SIZE(index)                 (0x0412 + (index * 0x0100))
#define REGISTER_SOCKET_PROTOCOL(index)                             (0x0414 + (index * 0x0100))
#define REGISTER_SOCKET_IP_TOS(index)                               (0x0415 + (index * 0x0100))
#define REGISTER_SOCKET_IP_TTL(index)                               (0x0416 + (index * 0x0100))
#define REGISTER_SOCKET_TX_FREE_SIZE(index)                         (0x0420 + (index * 0x0100))
#define REGISTER_SOCKET_TX_READ_POINTER(index)                      (0x0422 + (index * 0x0100))
#define REGISTER_SOCKET_TX_WRITE_POINTER(index)                     (0x0424 + (index * 0x0100))
#define REGISTER_SOCKET_RX_RECEIVED_SIZE(index)                     (0x0426 + (index * 0x0100))
#define REGISTER_SOCKET_RX_READ_POINTER(index)                      (0x0428 + (index * 0x0100))


/* MODE register values */
#define MODE_RESET			0x80 /**< reset */
#define MODE_PING_BLOCK			0x10 /**< ping block */
#define MODE_PPPOE		0x08 /**< enable pppoe */
#define MODE_LITTLE_ENDIAN  		0x04 /**< little or big endian selector in indirect mode */
#define MODE_AUTO_INCREMENT			0x02 /**< auto-increment in indirect mode */
#define MODE_INDIRECT			0x01 /**< enable indirect mode */

/* IR register values */
#define IR_CONFLICT	0x80 /**< check ip confict */
#define IR_UNREACH	0x40 /**< get the destination unreachable message in UDP sending */
#define IR_PPPoE		0x20 /**< get the PPPoE close message */
#define IR_SOCK(ch)	(0x01 << ch) /**< check socket interrupt */

// Socket mode register
#define SOCKET_MODE_PROTOCOL_CLOSE		0x00		/**< unused socket */
#define SOCKET_MODE_PROTOCOL_TCP		0x01		/**< TCP */
#define SOCKET_MODE_PROTOCOL_UDP		0x02		/**< UDP */
#define SOCKET_MODE_PROTOCOL_IPRAW	0x03		/**< IP LAYER RAW SOCK */
#define SOCKET_MODE_PROTOCOL_MACRAW	0x04		/**< MAC LAYER RAW SOCK */
#define SOCKET_MODE_PROTOCOL_PPPOE		0x05		/**< PPPoE */
#define SOCKET_MODE_NO_DELAY		0x20		/**< No Delayed Ack(TCP) flag */
#define SOCKET_MODE_ENABLE_MULTICASTING		0x80		/**< support multicating */

#define SOCKET_COMMAND_OPEN		0x01		/**< initialize or open socket */
#define SOCKET_COMMAND_LISTEN		0x02		/**< wait connection request in tcp mode(Server mode) */
#define SOCKET_COMMAND_CONNECT	0x04		/**< send connection request in tcp mode(Client mode) */
#define SOCKET_COMMAND_DISCONNECT		0x08		/**< send closing reqeuset in tcp mode */
#define SOCKET_COMMAND_CLOSE		0x10		/**< close socket */
#define SOCKET_COMMAND_SEND		0x20		/**< updata txbuf pointer, send data */
#define SOCKET_COMMAND_SEND_MAC	0x21		/**< send data with MAC address, so without ARP process */
#define SOCKET_COMMAND_SEND_KEEP	0x22		/**<  send keep alive message */
#define SOCKET_COMMAND_RECEIVE		0x40		/**< update rxbuf pointer, recv data */


#define Sn_CR_PCON				0x23
#define Sn_CR_PDISCON			0x24
#define Sn_CR_PCR					0x25
#define Sn_CR_PCN					0x26
#define Sn_CR_PCJ					0x27

// Sn_IR values
#define Sn_IR_PRECV			0x80
#define Sn_IR_PFAIL			0x40
#define Sn_IR_PNEXT			0x20

#define Sn_IR_SEND_OK			0x10		/**< complete sending */
#define Sn_IR_TIMEOUT			0x08		/**< assert timeout */
#define Sn_IR_RECV				0x04		/**< receiving data */
#define Sn_IR_DISCON				0x02		/**< closed socket */
#define Sn_IR_CON					0x01		/**< established connection */

/* Sn_SR values */
#define SOCKET_STATUS_CLOSED				0x00		/**< closed */
#define SOCKET_STATUS_INIT 				0x13		/**< init state */
#define SOCKET_STATUS_LISTEN				0x14		/**< listen state */
#define SOCKET_STATUS_SYNSENT	   		0x15		/**< connection state */
#define SOCKET_STATUS_SYNRECV		   	0x16		/**< connection state */
#define SOCKET_STATUS_ESTABLISHED		0x17		/**< success to connect */
#define SOCKET_STATUS_FIN_WAIT			0x18		/**< closing state */
#define SOCKET_STATUS_CLOSING		   	0x1A		/**< closing state */
#define SOCKET_STATUS_TIME_WAIT			0x1B		/**< closing state */
#define SOCKET_STATUS_CLOSE_WAIT			0x1C		/**< closing state */
#define SOCKET_STATUS_LAST_ACK			0x1D		/**< closing state */
#define SOCKET_STATUS_UDP				   0x22		/**< udp socket */
#define SOCKET_STATUS_IPRAW			   0x32		/**< ip raw mode socket */
#define SOCKET_STATUS_MACRAW			   0x42		/**< mac raw mode socket */
#define SOCKET_STATUS_PPPOE				0x5F		/**< pppoe socket */

/* IP PROTOCOL */
#define IPPROTO_IP              0           /**< Dummy for IP */
#define IPPROTO_ICMP            1           /**< Control message protocol */
#define IPPROTO_IGMP            2           /**< Internet group management protocol */
#define IPPROTO_GGP             3           /**< Gateway^2 (deprecated) */
#define IPPROTO_TCP             6           /**< TCP */
#define IPPROTO_PUP             12          /**< PUP */
#define IPPROTO_UDP             17          /**< UDP */
#define IPPROTO_IDP             22          /**< XNS idp */
#define IPPROTO_ND              77          /**< UNOFFICIAL net disk protocol */
#define IPPROTO_RAW             255         /**< Raw IP packet */

// </editor-fold>

static uint16_t get_free_udp_port(void);

static uint8_t read_register(uint16_t address);
static void write_register(uint16_t address, uint8_t value);
static uint16_t read_register_16(uint16_t address);
static void write_register_16(uint16_t address, uint16_t value);
static void read_buffer(uint16_t address, uint8_t* buffer, uint16_t length);
static void write_buffer(uint16_t address, const uint8_t* buffer, uint16_t length);

#define MAXIMUM_NUMBER_OF_SOCKETS                                   4

typedef enum
{
  INTERNAL_SOCKET_STATE_FREE,
  INTERNAL_SOCKET_STATE_OPEN_UDP
} internal_socket_state;

typedef struct
{
  internal_socket_state state;
  uint8_t socketIndex;
  uint16_t localPort;
} internal_socket;

static internal_socket sockets[MAXIMUM_NUMBER_OF_SOCKETS];

#define BUFFER_SIZE                                           (2048)
#define BUFFER_POINTER_MASK                                   (BUFFER_SIZE - 1)
#define BUFFER_TX_BASE(index)                                 (0x4000 + (index * BUFFER_SIZE))
#define BUFFER_RX_BASE(index)                                 (0x6000 + (index * BUFFER_SIZE))

void w5100_initialize(void)
{
  write_register(REGISTER_MODE, MODE_RESET);

  memset(sockets, 0, sizeof (sockets));

  write_register(REGISTER_TX_MEMORY_SIZE, 0x55); // all tx buffers are 2k
  write_register(REGISTER_RX_MEMORY_SIZE, 0x55); // all rx buffers are 2k
}

void w5100_set_mac_address(const uint8_t* mac)
{
  write_buffer(REGISTER_SOURCE_HARDWARE_ADDRESS, mac, 6);
}

void w5100_set_local_address(uint32_t ip)
{
  uint8_t buffer[4];
  WRITE_U32(buffer, ip);
  write_buffer(REGISTER_SOURCE_IP_ADDRESS, buffer, 4);
}

void w5100_set_gateway_address(uint32_t ip)
{
  uint8_t buffer[4];
  WRITE_U32(buffer, ip);
  write_buffer(REGISTER_GATEWAY_ADDRESS, buffer, 4);
}

void w5100_set_netmask(uint32_t mask)
{
  uint8_t buffer[4];
  WRITE_U32(buffer, mask);
  write_buffer(REGISTER_SUBNET_MASK_ADDRESS, buffer, 4);
}

// UDP interface

bool w5100_udp_open(w5100_socket* socket, uint16_t* sourcePort)
{
  uint8_t i;

  for(i = 0; i < MAXIMUM_NUMBER_OF_SOCKETS; i++)
  {
    if(sockets[i].state == INTERNAL_SOCKET_STATE_FREE)
    {
      // if source port was set to 0 find and return an unused port
      if(*sourcePort == 0)
      {
        *sourcePort = get_free_udp_port();
      }

      write_register(REGISTER_SOCKET_MODE(i), SOCKET_MODE_PROTOCOL_UDP);
      write_register_16(REGISTER_SOCKET_SOURCE_PORT(i), *sourcePort);
      write_register(REGISTER_SOCKET_COMMAND(i), SOCKET_COMMAND_OPEN);

      if(read_register(REGISTER_SOCKET_STATUS(i)) != SOCKET_STATUS_UDP)
      {
        write_register(REGISTER_SOCKET_COMMAND(i), SOCKET_COMMAND_CLOSE);
        return false;
      }

      sockets[i].localPort = *sourcePort;
      sockets[i].state = INTERNAL_SOCKET_STATE_OPEN_UDP;

      *socket = (w5100_socket) i;

      return true;
    }
  }

  return false;
}

void w5100_udp_close(w5100_socket* socket)
{
  uint8_t i = (uint8_t) * socket;

  if(i < MAXIMUM_NUMBER_OF_SOCKETS && sockets[i].state == INTERNAL_SOCKET_STATE_OPEN_UDP)
  {
    write_register(REGISTER_SOCKET_COMMAND(i), SOCKET_COMMAND_CLOSE);
    sockets[i].state = INTERNAL_SOCKET_STATE_FREE;
  }
}

bool w5100_udp_send(w5100_socket* socket, const uint8_t* buffer, uint16_t length, uint32_t ip, uint16_t port)
{
  uint8_t transferBuffer[4];
  uint8_t i = (uint8_t) * socket;
  uint16_t writePointer;
  uint16_t offset;
  uint16_t absoluteAddress;

  // valid socket, open socket, enough free buffer space to send packet?
  if(i >= MAXIMUM_NUMBER_OF_SOCKETS || sockets[i].state != INTERNAL_SOCKET_STATE_OPEN_UDP || read_register_16(REGISTER_SOCKET_TX_FREE_SIZE(i)) < length)
  {
    return false;
  }

  WRITE_U32(transferBuffer, ip);
  write_buffer(REGISTER_SOCKET_DESTINATION_IP_ADDRESS(i), transferBuffer, 4);
  write_register_16(REGISTER_SOCKET_DESTINATION_PORT(i), port);

  writePointer = read_register_16(REGISTER_SOCKET_TX_WRITE_POINTER(i));
  offset = writePointer & BUFFER_POINTER_MASK;
  absoluteAddress = BUFFER_TX_BASE(i) + offset;

  // transfer the packet to the W5100 tx buffer
  if((offset + length) >= BUFFER_SIZE) // does the packet cause a buffer wrap around -> write packet in two parts
  {
    uint16_t l1 = BUFFER_SIZE - offset;
    write_buffer(absoluteAddress, buffer, l1);
    write_buffer(BUFFER_TX_BASE(i), buffer + l1, length - l1);
  }
  else // packet can be written as one block
  {
    write_buffer(absoluteAddress, buffer, length);
  }

  write_register_16(REGISTER_SOCKET_TX_WRITE_POINTER(i), writePointer + length); // increment the write pointer

  write_register(REGISTER_SOCKET_COMMAND(i), SOCKET_COMMAND_SEND); // inform the W5100 that a packet is ready for transmission
  return true;
}

uint16_t w5100_udp_receive(w5100_socket* socket, uint8_t* buffer, uint16_t maximumLength, uint32_t* sourceIp, uint16_t* sourcePort)
{
  uint8_t i = (uint8_t) * socket;
  uint16_t readPointer;
  uint16_t offset;
  uint16_t absoluteAddress;
  uint8_t header[8];
  uint16_t length;

  // valid socket, open socket, anything in the W5100 receive buffer?
  if(i >= MAXIMUM_NUMBER_OF_SOCKETS || sockets[i].state != INTERNAL_SOCKET_STATE_OPEN_UDP || read_register_16(REGISTER_SOCKET_RX_RECEIVED_SIZE(i)) == 0)
  {
    return 0;
  }

  readPointer = read_register_16(REGISTER_SOCKET_RX_READ_POINTER(i));
  offset = readPointer & BUFFER_POINTER_MASK;
  absoluteAddress = BUFFER_RX_BASE(i) + offset;

  NABTO_LOG_TRACE(("rp=%u, offset=%u, abs=%u", (int)readPointer, (int)offset, (int)absoluteAddress));

  // read packet header
  if((offset + sizeof (header)) >= BUFFER_SIZE)
  {
    uint16_t l1 = BUFFER_SIZE - offset; // the number of bytes to read from the top of the buffer
    offset = sizeof (header) - l1; // the number of bytes to read from the bottom of the buffer - also updates the read offset
    read_buffer(absoluteAddress, header, l1);
    read_buffer(BUFFER_RX_BASE(i), header + l1, offset);
  }
  else
  {
    read_buffer(absoluteAddress, header, sizeof (header));
    offset += sizeof (header);
  }

  // parse header
  READ_U32(*sourceIp, header + 0);
  READ_U16(*sourcePort, header + 4);
  READ_U16(length, header + 6);

  // copy packet payload from W5100 to memory
  if(length <= maximumLength) // can the application buffer hold the received packet?
  {
    absoluteAddress = BUFFER_RX_BASE(i) + offset;
    if((offset + length) >= BUFFER_SIZE)
    {
      uint16_t l1 = BUFFER_SIZE - offset;
      read_buffer(absoluteAddress, buffer, l1);
      read_buffer(BUFFER_RX_BASE(i), buffer + l1, length - l1);
    }
    else
    {
      read_buffer(absoluteAddress, buffer, length);
    }
  }

  readPointer += sizeof(header) + length; // increment the read pointer

  NABTO_LOG_TRACE(("rp=%u", (int)readPointer));

  write_register_16(REGISTER_SOCKET_RX_READ_POINTER(i), readPointer); // write new read pointer

  write_register(REGISTER_SOCKET_COMMAND(i), SOCKET_COMMAND_RECEIVE); // inform the W5100 that a packet has been received

  if(length <= maximumLength)
  {
    NABTO_LOG_TRACE(("Received packet: source=" PRIip ":%" PRIu16 ", length=%" PRIu16, MAKE_IP_PRINTABLE(*sourceIp), *sourcePort, length));
    return length;
  }
  else
  {
    NABTO_LOG_TRACE(("Dropped oversize packet: source=" PRIip ":%" PRIu16 ", length=%" PRIu16, MAKE_IP_PRINTABLE(*sourceIp), *sourcePort, length));
    return 0;
  }
}

// Implement uNabto interface

bool w5100_nabto_init_socket(uint16_t* localPort, nabto_socket_t* socket)
{
  bool result;

  result = w5100_udp_open(socket, localPort);

  NABTO_LOG_TRACE(("w5100_nabto_udp_open socket=%u localPort=%u", (int) *socket, *localPort));

  return result;
}

void w5100_nabto_close_socket(nabto_socket_t* socket)
{
  NABTO_LOG_TRACE(("w5100_nabto_udp_close"));

  w5100_udp_close(socket);
}

ssize_t w5100_nabto_write(nabto_socket_t socket, const uint8_t* buf, size_t len, uint32_t addr, uint16_t port)
{
  NABTO_LOG_TRACE(("w5100_nabto_udp_write socket=%u length=%u", (int) socket, (int) len));

  if(w5100_udp_send(&socket, (uint8_t*) buf, len, addr, port))
  {
    return len;
  }
  else
  {
    return -1;
  }
}

ssize_t w5100_nabto_read(nabto_socket_t socket, uint8_t* buf, size_t len, struct nabto_ip_address* addr, uint16_t* port)
{
  if (addr->type != NABTO_IP_V4) {
    return 0;
  }
  {
    uint16_t length = w5100_udp_receive(&socket, buf, len, &addr->addr.ipv4, port);
  

    if(length > 0)
    {
      NABTO_LOG_TRACE(("w5100_nabto_udp_read socket=%u length=%u", (int) socket, (int) length));
    }

    return length;
  }
}

// Helpers

#define LOWEST_UDP_PORT                                             51000

static uint16_t get_free_udp_port(void)
{
  static uint16_t lastPort = LOWEST_UDP_PORT - 1;
  uint8_t i = 0;

  lastPort++;

  while(i < MAXIMUM_NUMBER_OF_SOCKETS)
  {
    if(sockets[i].state == INTERNAL_SOCKET_STATE_OPEN_UDP && sockets[i].localPort == lastPort)
    {
      if(++lastPort == 0)
      {
        lastPort = LOWEST_UDP_PORT;
      }
      i = 0; // start over the is-port-in-use test
    }
    else
    {

      i++;
    }
  }

  return lastPort;
}

// <editor-fold defaultstate="collapsed" desc="w5100 register and buffer IO">

static uint8_t read_register(uint16_t address)
{
  uint8_t registerTransferBuffer[4];

  registerTransferBuffer[0] = OPCODE_READ;
  registerTransferBuffer[1] = (uint8_t) (address >> 8);
  registerTransferBuffer[2] = (uint8_t) address;
  registerTransferBuffer[3] = 0;

  w5100_spi_transfer_buffer(registerTransferBuffer, sizeof (registerTransferBuffer));

  return registerTransferBuffer[3];
}

static void write_register(uint16_t address, uint8_t value)
{
  uint8_t registerTransferBuffer[4];

  registerTransferBuffer[0] = OPCODE_WRITE;
  registerTransferBuffer[1] = (uint8_t) (address >> 8);
  registerTransferBuffer[2] = (uint8_t) address;
  registerTransferBuffer[3] = value;
  w5100_spi_transfer_buffer(registerTransferBuffer, sizeof (registerTransferBuffer));
}

static uint16_t read_register_16(uint16_t address)
{
  uint16_t value = read_register(address);
  value <<= 8;
  value |= read_register(address + 1);

  return value;
}

static void write_register_16(uint16_t address, uint16_t value)
{
  write_register(address, (uint8_t) (value >> 8));
  write_register(address + 1, (uint8_t) value);
}

static void read_buffer(uint16_t address, uint8_t* buffer, uint16_t length)
{
  uint8_t registerTransferBuffer[4];

  while(length-- > 0)
  {
    registerTransferBuffer[0] = OPCODE_READ;
    registerTransferBuffer[1] = (uint8_t) (address >> 8);
    registerTransferBuffer[2] = (uint8_t) address;
    registerTransferBuffer[3] = 0x00;
    w5100_spi_transfer_buffer(registerTransferBuffer, sizeof (registerTransferBuffer));

    *buffer++ = registerTransferBuffer[3];
    address++;
  }
}

static void write_buffer(uint16_t address, const uint8_t* buffer, uint16_t length)
{
  uint8_t registerTransferBuffer[4];

  while(length-- > 0)
  {
    registerTransferBuffer[0] = OPCODE_WRITE;
    registerTransferBuffer[1] = (uint8_t) (address >> 8);
    registerTransferBuffer[2] = (uint8_t) address;
    registerTransferBuffer[3] = *buffer++;

    w5100_spi_transfer_buffer(registerTransferBuffer, sizeof (registerTransferBuffer));

    address++;
  }
}

// </editor-fold>
