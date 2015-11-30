#ifdef WIN32
DWORD WINAPI test_webserver(void*);
#else
void* test_webserver(void*);
#endif
