#include "vhitek.h"

namespace VHITEK
{
    namespace Ds1307
    {
        UnixTime stamp(7);

        void begin()
        {          
            // rtc.begin();
            if (! rtc.begin(&Wire)) {
                // Serial.println("KHONG TIM THAY RTC");
                Serial.flush();
                //abort();
            }            
            if (! rtc.isrunning()) {
                // Serial.println("RTC KHONG CHAY, CAI DAT LAI THOI GIAN");
                rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
            }         

            // rtc.adjust(DateTime(2022, 10, 12, 9, 39, 0)); //nam, thang, ngay, gio, phut, giay
        }

        uint64_t ToTimeStamp(struct_vmSaleDate thoigian)
        {
            stamp.setDateTime(thoigian.year+2000, thoigian.month, thoigian.day, thoigian.hour, thoigian.min, thoigian.seconds);

            // Serial.println(stamp.getUnix());

            return stamp.getUnix();
        }

        void loop()
        {
            DateTime now = rtc.now();

            thoigian.day = now.day();
            thoigian.month = now.month();
            thoigian.year = now.year()-2000;
            thoigian.hour = now.hour();
            thoigian.min = now.minute();
            thoigian.seconds = now.second();

            // ToTimeStamp(thoigian);

            // Serial.print(thoigian.year);
            // Serial.print('/');
            // Serial.print(thoigian.month);
            // Serial.print('/');
            // Serial.print(thoigian.day);
            // Serial.print(" - ");
            // // Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
            // // Serial.print(") ");
            // Serial.print(thoigian.hour);
            // Serial.print(':');
            // Serial.print(thoigian.min);
            // Serial.print(':');
            // Serial.print(thoigian.seconds);
            // Serial.println();            
        }
    }
}