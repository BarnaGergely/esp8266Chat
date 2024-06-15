#include <ESP8266WiFi.h>
#include <string.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>

/* ============= CHANGE WIFI SSID ============= */
const char *ssid = "Chat Server";
/* ============= ================ ============= */

// cache expire time for static files in seconds, 7 days
const char *catcheExpireTime = "max-age=604800";

DNSServer dnsServer;
AsyncWebServer server(80);
FSInfo fs_info;
File f;

// returns free space in the eeprom in bytes
size_t getFreeSpace() {
    // manually update info, because it wont update automatically
    LittleFS.info(fs_info);
    int freeSpace = fs_info.totalBytes - fs_info.usedBytes;
    // if something went wrong and freeSpace is negative, return 0
    return (freeSpace < 0) ? 0 : freeSpace;
}

void setup()
{
    // configure IP and netmask
    IPAddress apIP(192, 168, 0, 1);
    IPAddress netMask(255, 255, 255, 0);

    // initialize wifi-ap
    WiFi.mode(WIFI_STA);
    WiFi.softAPConfig(apIP, apIP, netMask);
    WiFi.softAP(ssid);

    // mount filesystem etc.
    EEPROM.begin(4096);
    LittleFS.begin();

    // add mdns (http://chat.local/)
    MDNS.addService("http", "tcp", 80);
    MDNS.begin("chat");

    // start dns server to redirect all sites
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());

    // create file for text messages if it doesn't exist yet
    f = LittleFS.open("/messages.txt", "a");
    f.close();

    // route traffic to index.html file
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html", "text/html");
        response->addHeader("Cache-Control", catcheExpireTime);
        request->send(response);
        //request->send(LittleFS, "/index.html", "text/html");
    });

    // route to /tools.html file
    server.on("/tools.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/tools.html", "text/html");
        response->addHeader("Cache-Control", catcheExpireTime);
        request->send(response);
        //request->send(LittleFS, "/tools.html", "text/html");
    });

        // route to /messages.html file
    server.on("/tools.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/messages.html", "text/html");
        response->addHeader("Cache-Control", catcheExpireTime);
        request->send(response);
        //request->send(LittleFS, "/tools.html", "text/html");
    });

    // route to /styles.css file
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/styles.css", "text/css");
        response->addHeader("Cache-Control", catcheExpireTime);
        request->send(response);
    });

    // route to /scripts.js file
    server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/scripts.js", "text/js");
        response->addHeader("Cache-Control", catcheExpireTime);
        //response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

    // route to get free space - Returns the free space in bytes
    server.on("/getFreeSpace", HTTP_GET, [](AsyncWebServerRequest *request) {
        LittleFS.info(fs_info);
        request->send(200, "text/plain", String(getFreeSpace()));
    });

    // route to send text
    server.on("/sendText", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params_count = request->params();

        char *name = NULL;
        char *text = NULL;

        for (int i = 0; i < params_count; i++)
        {
            AsyncWebParameter *p = request->getParam(i);

            char *paramName = (char *)p->name().c_str();
            char *paramValue = (char *)p->value().c_str();

            // replace some characters
            for (unsigned int i = 0; i < strlen(paramValue); i++)
            {
                // replace pipe with /
                // because we use pipe as separator between messages.
                // if anyone used the pipe symbol in their messages, it would
                // mess up everything.
                if (paramValue[i] == '|')
                {
                    paramValue[i] = '/';
                }
            }

            // test if input only has spaces
            bool onlySpaces = true;

            if (strlen(paramValue) == 0)
            {
                onlySpaces = false;
            }

            for (unsigned int i = 0; i < strlen(paramValue); i++)
            {
                if (paramValue[i] != ' ')
                {
                    onlySpaces = false;
                    break;
                }
            }

            if (strcmp(paramName, "nickname") == 0 && strlen(paramValue) > 0 && !onlySpaces)
            {
                name = paramValue;
            }
            else if (strcmp(paramName, "text") == 0 && strlen(paramValue) > 0 && !onlySpaces)
            {
                text = paramValue;
            }
        }

        if (name == NULL)
        {
            // default nickname is 'Anon'
            name = (char *)"Anon";
        }

        if (text == NULL)
        {
            // illegal input -> abort
            request->redirect("/");
            return;
        }

        // allocate memory for new string
        // breaks down into:
        // - size of both strings
        // - 3 extra characters
        // - 1 for null-byte '\0' at the end
        char *resultString = (char *)malloc(strlen(name) + strlen(text) + 4);

        // combine strings
        sprintf(resultString, "%s: %s|", name, text);

        // append string to file
        f = LittleFS.open("/messages.txt", "a");
        f.write(resultString, strlen(resultString));
        f.close();

        //redirect to index.html
        request->redirect("/");
    });

    // route to show message-file contents
    server.on("/showText", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/messages.txt", "text/html");
    });

    // route to show timestamp of last write
    server.on("/lastWrite", HTTP_GET, [](AsyncWebServerRequest *request) {
        // open file
        f = LittleFS.open("/messages.txt", "r");

        // get last write time
        time_t lastWriteTime = f.getLastWrite();

        // convert to string
        char *lastWriteString = ctime(&lastWriteTime);

        // close file
        f.close();

        // send response
        request->send(200, "text/plain", lastWriteString);
    });

    // route to clear message-file
    server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *request) {
        f = LittleFS.open("/messages.txt", "w");
        f.close();
        request->redirect("/");
    });

    // route rest of traffic to index (triggers captive portal popup)
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    // start the webserver
    server.begin();
}

void loop()
{
    dnsServer.processNextRequest();
}