#include "vhitek.h"
#include <AsyncElegantOTA.h> 
#include <ESPConnect.h>

namespace VHITEK
{
    namespace OTA
    {
        AsyncWebServer server(80);   

        void OTAServerStop()
        {
            WiFi.softAPdisconnect(true);
            server.end();
            WiFi.mode(WIFI_MODE_STA);
            esp_wifi_set_ps(WIFI_PS_NONE);
        }

        void OTAServerLoop()
        {
            AsyncElegantOTA.loop();
        }

        void OTAServerInit()
        {
            ESPConnect.autoConnect(apSSID);
            AsyncElegantOTA.begin(&server); // Start ElegantOTA
            char OTAID[50];
            sprintf(OTAID, "%s|FW : %s", apSSID, FW);
            AsyncElegantOTA.setID(OTAID);
            if (ESPConnect.begin(&server))
            {
                // Serial.println("Connected to WiFi");
                // Serial.println("IPAddress: " + WiFi.localIP().toString());
            }
            else
            {
                // Serial.println("Failed to connect to WiFi");
            }
            esp_wifi_set_ps(WIFI_PS_NONE);
            WiFi.mode(WIFI_MODE_AP);
            esp_wifi_set_ps(WIFI_PS_NONE);
        }

        void WifiBegin()
        {
            ESPConnect.begin();
        }

        void OTAServerBegin()
        {
            OTAServerInit();
            server.begin();
            esp_wifi_set_ps(WIFI_PS_NONE);
        }        
    }
}
