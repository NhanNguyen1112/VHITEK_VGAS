#include "vhitek.h"
#include "RFID.h"

namespace VHITEK
{
  namespace RFID
  {
    char encodeStr[7]; // holds string representation of encoding
    void begin()
    {
      pinMode(36, INPUT);
      rfidSetup(false, encodeStr, 0, 36, false);
      // Serial.printf("Recognise tag type: %s\n", encodeStr);      
    }

    void loop()
    {
      if (captureReady(50))
      {
        uint64_t currentTag = getTag();
        if (currentTag)
        {
          ID = currentTag;
          // testID = atoi(currentTag);
          // Serial.printf("%010ld \n",ID);
        }
        resetCapture();
      }   
    }
  }
}