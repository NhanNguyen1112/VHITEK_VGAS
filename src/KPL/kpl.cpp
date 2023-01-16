#include "vhitek.h"

namespace VHITEK
{
    namespace KPL
    {
        VgasTranactionType decodeKPL76Byte(char *rData)
        {
            VgasTranactionType decodedData;
            char dData[20];
            decodedData.TrangThaiCoBom = rData[2] - '0';
            decodedData.IdVoiBom = rData[3] - 10;
            int offset = 4;
            for (int i = 0; i < 6; i++)
            {
                dData[i] = rData[offset + i];
            }
            dData[6] = 0;
            decodedData.IdLanBom = atol(dData);

            offset = 10;
            for (int i = 0; i < 9; i++)
            {
                dData[i] = rData[offset + i];
            }
            dData[9] = 0;
            decodedData.SoLitBomDuoc = atol(dData);

            offset = 19;
            for (int i = 0; i < 6; i++)
            {
                dData[i] = rData[offset + i];
            }
            dData[6] = 0;
            decodedData.GiaXangDau = atol(dData);

            offset = 25;
            for (int i = 0; i < 9; i++)
            {
                dData[i] = rData[offset + i];
            }
            dData[9] = 0;
            decodedData.total = atol(dData);

            offset = 34;
            for (int i = 0; i < 9; i++)
            {
                dData[i] = rData[offset + i];
            }
            dData[9] = 0;
            decodedData.ThanhTien = atol(dData);
            return decodedData;
        }

        VgasTranactionType KPL_ReadData(int ID)
        {
            char rData[200];
            int rPtr = 0;
            delay(20);
            VgasTranactionType decodeData;
            decodeData.IdVoiBom = -1;

            while (Serial2.available())
            {
                Serial2.read();
                delay(5);
            }
            Serial2.write(5);
            Serial2.write(11);
            Serial2.write(6);
            uint32_t startTime = millis();
            while (1)
            {
                if ((uint32_t)(millis() - startTime) > 500)
                    return decodeData;
                if (Serial2.available())
                {
                    char data = Serial2.read();
                    if (rPtr < 200)
                    {
                        rData[rPtr] = data;
                        rPtr++;
                    }
                    else
                    {
                        break;
                    }
                    if (rPtr >= 76)
                        break;
                }
                delay(3);
            }
            if ((rData[rPtr - 1] == 4) && (rData[rPtr - 3] == 3))
            {
                char checksum = 0x5A;
                //Serial.printf("\r\nKPL_ReadData : ");
                for (int i = 2; i < rPtr - 3; i++)
                {
                    //Serial.printf("%02X ", rData[i]);
                    checksum = checksum ^ rData[i];
                }
                //Serial.printf("\r\n");
                //Serial.printf("Checksum : %02X %02X\r\n", checksum, rData[rPtr - 2]);

                if(checksum != rData[rPtr - 2]){
                    // Serial.printf("Checksum Error\r\n");
                    return decodeData;
                }
                
                if (rPtr == 76)
                {
                    decodeData = decodeKPL76Byte(rData);
                }

                // if (Transactions::getTotalOfflineData() < Transactions::maxTransactionEEPromIdx - 1)
                if(IDX_Synch < maxTransactionEEPromIdx-1 && day_bo_nho==false) //Cho phép bơm
                {
                    if (decodeData.TrangThaiCoBom == 0)
                    {
                        Serial2.write(7);
                        Serial2.write(10 + ID);
                        Serial2.write(8);
                    }
                }
            }
            
            // Serial.printf("IDX: %d - Part_ID: %d - TThái cò: %d - ID vòi: %d - ID lần bơm: %d - So lít bơm: %d - Giá xăng: %d - ", decodeData.ID, decodeData.partId, decodeData.TrangThaiCoBom, decodeData.IdVoiBom, decodeData.IdLanBom, decodeData.SoLitBomDuoc, decodeData.GiaXangDau);
            // Serial.printf("Thành tiền: %d - Total: %d - Payment: %d - Loại xăng: %d\n",decodeData.ThanhTien, decodeData.total, decodeData.paymentTerm, decodeData.LoaiXangDau);

            return decodeData;
        }

        bool setPrice(int ID, uint32_t price)
        {
            char priceData[50];
            char checksum = 0x5A;
            char outputData[50];
            delay(200);
            VgasTranactionType rData = KPL_ReadData(ID);
            if (rData.TrangThaiCoBom != 0)
                return false;
            sprintf(priceData, "%06d", price);
            if (strlen(priceData) > 6)
            {
                return false;
            }
            delay(200);
            outputData[0] = ID + 10;
            outputData[1] = 'P';
            for (int i = 0; i < 6; i++)
            {
                outputData[2 + i] = priceData[5 - i];
            }
            Serial2.write(9);
            for (int i = 0; i < 8; i++)
            {
                checksum = checksum ^ outputData[i];
                Serial2.write(outputData[i]);
            }
            Serial2.write(checksum);
            Serial2.write(10);
            delay(200);
            rData = KPL_ReadData(ID);
            if (rData.GiaXangDau == price)
            {
                Serial.printf("Set Gia Thanh Cong ID :%d,Price : %d \r\n", ID, price);
                //VHITEK::Display::updatePumpData(rData);
                //currentData = rData;
                // lastTranctionData = rData;
                return true;
            }
            return false;
        }

        VgasTranactionType getNewData(QueueHandle_t queuedata)
        {
            VgasTranactionType lReceivedValue;
            if (xQueueReceive(queuedata, &lReceivedValue, 100) == pdPASS)
            {
                return lReceivedValue;
            }
            lReceivedValue.ID = 0;
            lReceivedValue.GiaXangDau = 0;
            return lReceivedValue;
        }

        uint8_t countNewData(QueueHandle_t queuedata)
        {
            return uxQueueMessagesWaiting(queuedata);
        }
    }
}
        