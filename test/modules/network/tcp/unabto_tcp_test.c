#include <modules/network/tcp/unabto_tcp.h>
#include <unabto/unabto_context.h>
#include <basetsd.h>


int main(int argc, char** argv)
{
	struct unabto_tcp_socket sock;
	nabto_endpoint ep;
	enum unabto_tcp_status ret;
	const char buf[] = { "Hello world\n" };
	char recBuf[65535];
	size_t written = 0;
	size_t read = 0;

	memset(recBuf, 0, 65535);

	recBuf[0] = 0;

	ret = unabto_tcp_open(&sock);
	NABTO_LOG_INFO(("open return value was: %d", ret));

	ep.addr = 2130706433;
	ep.port = 32006;
	ret = unabto_tcp_connect(&sock, &ep);
	NABTO_LOG_INFO(("connect return value was: %d", ret));

	while (unabto_tcp_connect_poll(&sock) == UTS_CONNECTING) {
		NABTO_LOG_INFO(("connect poll return value was: %d", ret));
	}

	ret = unabto_tcp_write(&sock, (void*)buf, strlen(buf), &written);
	NABTO_LOG_INFO(("write return value was: %d, and written: %d", ret, written));

	while (unabto_tcp_read(&sock, (void*)recBuf, 65535, &read) == UTS_WOULD_BLOCK) {
	}
	NABTO_LOG_INFO(("read got read: %d", read));
	if (read > 0) {
		NABTO_LOG_INFO(("Got response: %s", recBuf));
	}

    ret = unabto_tcp_shutdown(&sock);
    NABTO_LOG_INFO(("shutdown return value was: %d",ret));

    
    ret = unabto_tcp_close(&sock);
    NABTO_LOG_INFO(("close return value was: %d",ret));
    return 0;
}
