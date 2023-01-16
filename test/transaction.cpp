#include "vhitek.h"
namespace VHITEK
{
    namespace Transactions
    {
        ExternalEEPROM vmTransactionEeprom;
        VPETROConfigType settings;
        uint32_t TransactionID = 0;
        uint32_t lastTransactionEEPromIdx = 0;
        uint32_t maxTransactionEEPromIdx = 64000 / sizeof(VgasTranactionType) - 10;
        uint32_t offlineDataCount = 0;

        int getCurrenTransactionID()
        {
            return TransactionID;
        }



        uint32_t getSessionID()
        {
            return settings.partId;
        }


        uint32_t getTotalOfflineData()
        {
            return offlineDataCount;
        }
        

        void updateTotalOfflineData(uint32_t x)
        {
            offlineDataCount = x;
        }


        bool addTransaction(VgasTranactionType data)
        {
            uint32_t tick = millis();
            bool success = false;
            accessI2C1Bus([&]
                          {
                             if (writeToEpprom(data, lastTransactionEEPromIdx + 1))
                             {
                                 lastTransactionEEPromIdx += 1;
                                 if(lastTransactionEEPromIdx >= (maxTransactionEEPromIdx-1))
                                    lastTransactionEEPromIdx=0;
                                 success = true;
                                 TransactionID+=1;
                                 offlineDataCount++;
                             }
                             else
                             {
                                 success = false;
                             } },
                          1000);
            Serial.printf("addTransaction done : %s- driff : %ld \r\n", success ? "true" : "false", millis() - tick);
            return success;
        }


        bool writeToEpprom(VgasTranactionType data, uint16_t idx)
        {
            uint32_t tick = millis();
            uint16_t eepromPtr = idx * sizeof(VgasTranactionType);
            uint16_t sum = sumCalc(data);
            data.checkSum = sum;
            vmTransactionEeprom.put(eepromPtr, data);
            // Serial.printf("vmTransactionEeprom.put driff : %d \r\n", (millis() - tick));
            if ((millis() - tick) >= 5000)
            {
                vmTransactionEeprom.setMemorySize(512000 / 8);
                vmTransactionEeprom.setPageSize(64);
            }
            tick = millis();
            bool dataVaid;
            readEeprom(idx, &dataVaid);
            // Serial.printf("vmTransactionEeprom.readEeprom vaild : %s - driff : %d \r\n", dataVaid ? "true" : "false", (millis() - tick));
            return dataVaid;
        }



        int16_t sumCalc(VgasTranactionType data)
        {
            int len = sizeof(data);
            uint8_t buffer[50];
            uint8_t *ptr = (uint8_t *)&data;
            uint8_t dataSize = 0;
            for (int i = 0; i < len; i++)
            {
                if ((uint32_t)&data.checkSum <= (uint32_t)&ptr[i])
                {
                    break;
                }
                buffer[i] = ptr[i];
                dataSize++;
            }
            uint16_t sum = cal_crc_loop_CCITT_A(dataSize, buffer);
            return sum;
        }


        VgasTranactionType readEeprom(uint16_t idx, bool *dataVaid)
        {
            uint16_t eepromPtr = idx * sizeof(VgasTranactionType);
            VgasTranactionType tmpData;
            vmTransactionEeprom.get(eepromPtr, tmpData);
            uint16_t sum = sumCalc(tmpData);
            *dataVaid = false;
            if (tmpData.TrangThaiCoBom > 1)
            {
                *dataVaid = false;
                return tmpData;
            }

            if (tmpData.SoLitBomDuoc > 1000000)
            {
                *dataVaid = false;
                return tmpData;
            }

            if (sum == tmpData.checkSum)
                *dataVaid = true;
            return tmpData;
        }



        String toJson(VgasTranactionType data)
        {
            DynamicJsonDocument doc(10000);
            char rData[2048];
            char dateBuf[25];
            sprintf(dateBuf, "%04d-%02d-%02d %02d:%02d:%02d",
                    data.date.year + 1900, data.date.month, data.date.day,
                    data.date.hour, data.date.min, data.date.seconds);
            doc["mId"] = apSSID;
            doc["pumpId"] = data.IdVoiBom;
            doc["ID"] = data.ID;
            doc["sessionId"] = data.partId;
            doc["time"] = dateBuf;
            doc["TrangThaiCoBom"] = data.TrangThaiCoBom;
            doc["IdLanBom"] = data.IdLanBom;
            doc["SoLitBomDuoc"] = data.SoLitBomDuoc;
            doc["GiaXangDau"] = data.GiaXangDau;
            doc["LoaiNhienLieu"] = data.LoaiXangDau;
            doc["pType"] = data.paymentTerm;
            doc["ThanhTien"] = data.ThanhTien;
            doc["Total"] = data.total;
            serializeJson(doc, rData);
            return String(rData);
        }


        void begin()
        {
            bool vaid = false;
            uint32_t lastIdx = 0;
            VgasTranactionType tmpData;
            // sizeof(tmpData);
            u8g2.clearBuffer();                   // clear the internal memory
            u8g2.setFont(u8g2_font_profont10_mf); // Courier New Bold 10,12,14,18,24
            u8g2.setCursor(1, 10);
            u8g2.printf("LOADING DATA ...");
            u8g2.sendBuffer();
            if (vmTransactionEeprom.begin(0x51, Wire1) == false)
            {

                u8g2.setCursor(1, 20);
                u8g2.printf("vmTransactionEeprom not found");
                u8g2.sendBuffer();
                Serial.println(F("vmTransactionEeprom not found"));
                while (1)
                {
                    delay(100);
                }
            }
            vmTransactionEeprom.setMemorySize(65536);
            vmTransactionEeprom.setPageSize(64);
            Serial.println(sizeof(VgasTranactionType));
            uint32_t startTick = millis();
            bool inited = false;
            for (uint32_t i = 0; i < maxTransactionEEPromIdx; i++)
            {
                if ((uint32_t)(millis() - startTick) > 200)
                {
                    u8g2.setCursor(1, 30);
                    u8g2.printf("Loading %.2f ", (float)i / maxTransactionEEPromIdx * 100);
                    u8g2.sendBuffer();
                    startTick = millis();
                }
                tmpData = readEeprom(i, &vaid);
                if (vaid)
                {
                    if (tmpData.ID > 1000000000)
                    {
                        continue;
                    }

                    if (tmpData.TrangThaiCoBom == 0)
                    {
                        offlineDataCount++;
                    }

                    // Serial.println(VHITEK::VGAS::toJson(tmpData));
                    if (lastIdx < tmpData.ID)
                    {
                        lastIdx = tmpData.ID;
                        TransactionID = tmpData.ID;
                        lastTransactionEEPromIdx = i;
                        VPETRO::lastPumpData = tmpData;
                    }
                }
            }
            TransactionID = TransactionID + 1;
            // Serial.printf("lastIdx = %d lastTransactionEEPromIdx = %d\n", TransactionID, lastTransactionEEPromIdx);
        }
    }
}