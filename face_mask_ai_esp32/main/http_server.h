#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H



#include "esp_https_server.h"
void http_server_start(void);
void start_mdns_service(void);

#endif // HTTP_SERVER_H