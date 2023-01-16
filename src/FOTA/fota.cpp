#include "vhitek.h"

namespace VHITEK
{
    namespace FOTA
    {
        esp32FOTA FOTA("VGAS", FW_server, false);

        void FOTAbegin()
        {
            //http://159.223.48.4:8000/users/getversionmachine/{id}/{version}
            char url[200];
            sprintf(url,"http://159.223.48.4:85/locker/getversionmachine/%s/%s/%d",apSSID,FW_TYPE,FW_server);
            FOTA.checkURL=String(url);
            // Serial.println(FOTA.checkURL);
        }

        void Star_update()
        {
            // Serial.println("Kiem tra update");
            // FOTA.useDeviceID = true;
            bool updatedNeeded = FOTA.execHTTPcheck();
            // Serial.println(updatedNeeded);
            if (updatedNeeded)
            {
                check_update_FOTA  = true;
                Serial.println("DANG UPDATE");
                FOTA.execOTA();
                check_update_FOTA = false;
            }            
            else check_update_FOTA = false;
        }
    }
}