#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "TCP.h"
#include "Http.h"
#include"global.h"
#include "SSL.h"

#define ALLOW_ACCEPT 1
#define HTTP_KIND int
#define HTTP 0
#define HTTPS 1

int RunWebServer(char *ip, int port, int maxListener, HTTP_KIND flag);
int RunHttpServer(char *ip, int port, int maxListener);
int RunHttpsServer(char *ip, int port, int maxListener);

#endif