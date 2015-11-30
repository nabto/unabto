#if NABTO_ENABLE_STREAM && !NABTO_ENABLE_CONNECTIONS
#error "Streaming only works with connection based communication which means either NABTO_ENABLE_LOCAL_CONNECTION and/or NABTO_ENABLE_REMOTE_ACCESS must be enabled."
#endif



