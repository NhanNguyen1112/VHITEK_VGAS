#include "vhitek.h"

namespace VHITEK
{
    namespace Config
    {
        int16_t sumCalc_LED(ledshow data)
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
        void Json_led()
        {
            DynamicJsonDocument doc(10000);
            doc.clear();

            doc["tien"] = VHITEK::hienthiled.tien;
            doc["lit"] = VHITEK::hienthiled.lit;
            doc["gia"] = VHITEK::hienthiled.gia;
            doc["tl"] = VHITEK::hienthiled.tl; //11: Lit; 12: Tien; 0: clear
            doc["csum"] = VHITEK::hienthiled.checkSum;

            Serial2.write(0x7F);
            serializeJson(doc, Serial2);
            Serial2.write(0x7E);
        }
        String Json_transac_real_time(VgasTranactionType data) // Gui Real time
        {
            DynamicJsonDocument doc(10000);
            char rData[2048];
            char Time[25];

            sprintf(Time, "%04d-%02d-%02dT%02d:%02d:%02d",
                    data.date.year + 2000, data.date.month, data.date.day, data.date.hour, data.date.min, data.date.seconds);

            doc["time"] = Time;
            doc["ThanhTien"] = data.ThanhTien;
            doc["Total"] = data.total;
            doc["GiaXangDau"] = data.GiaXangDau;
            doc["SoLitBomDuoc"] = data.SoLitBomDuoc;
            doc["LoaiNhienLieu"] = data.LoaiXangDau;
            doc["pType"] = data.paymentTerm;
            doc["sessionId"] = data.partId;
            doc["pumpId"] = data.IdVoiBom;
            doc["cardId"] = data.cardId;
            doc["TrangThaiCoBom"] = data.TrangThaiCoBom;
            doc["IdLanBom"] = data.IdLanBom;
            doc["fw"] = FW;
            doc["wifi"] = Wifi_RSSI();
            doc["mId"] = apSSID;
            serializeJson(doc, rData);
            return String(rData);
        }
        String Json_transac(VgasTranactionType data) // lich su giao dich
        {
            DynamicJsonDocument doc(10000);
            char rData[2048];
            char Time[25];

            sprintf(Time, "%04d-%02d-%02dT%02d:%02d:%02d",
                    data.date.year + 2000, data.date.month, data.date.day, data.date.hour, data.date.min, data.date.seconds);

            doc["time"] = Time;
            doc["ThanhTien"] = data.ThanhTien;
            doc["Total"] = data.total;
            doc["GiaXangDau"] = data.GiaXangDau;
            doc["SoLitBomDuoc"] = data.SoLitBomDuoc;
            doc["LoaiNhienLieu"] = data.LoaiXangDau;
            doc["pType"] = data.paymentTerm;
            doc["sessionId"] = data.partId;
            doc["pumpId"] = data.IdVoiBom;
            doc["cardId"] = data.cardId;
            doc["pId"] = 0;
            doc["IdLanBom"] = data.IdLanBom;
            doc["mId"] = apSSID;
            serializeJson(doc, rData);
            return String(rData);
        }

        void cai_dat_mat_khau()
        {
            // VHITEK::Config::All_Clear_eeprom(1, 20); //XOA toan bo EEPROM
            accessI2C1Bus([&]
                    { myMem.get(0, Vpass); },
            100);
            // Serial.printf("\nMKNV: %d - MKKT: %d - MK_chutram: %d - MK_vhitek: %d\n", Vpass.NV, Vpass.KT, Vpass.chutram, Vpass.vhitek);

            if(Vpass.vhitek!=MK_vhitek)
            {
                Vpass.vhitek=MK_vhitek;

                accessI2C1Bus([&]
                    { myMem.put(0, Vpass); },
                100);

                accessI2C1Bus([&]
                    { myMem.get(0, Vpass); },
                100);
                // Serial.printf("\nMKNV: %d - MKKT: %d - MKMASS: %d - MKCalib: %d\n", Vpass.NV, Vpass.KT, Vpass.MASS, Vpass.calib);
            } //else Serial.printf("\nMKKT: %d - MKKT: %d - MKMASS: %d\n", Vpass.KH, Vpass.KT, Vpass.MASS);
        }

        int so_sanh_mk_menu(char *mk)
        {
            uint32_t MK = atoi(mk);

            if (MK==Vpass.vhitek) return 1;
            else if (MK==Vpass.chutram) return 2;
            else if (MK==Vpass.KT) return 3;
            else if (MK==Vpass.NV) return 4;
            else return 0;

            // Serial.println(MK);
        }

        void All_Clear_eeprom(int eep, uint16_t Max_diachi) // Xoa toan bo IC eeprom luu hanh dong mo cua
        {
            if (eep == 0) // xoa ca 2 eeprom
            {
                for (uint16_t i = 0; i <= Max_diachi; i++)
                {
                    myMem.write(i, 0);
                    myMem2.write(i, 0);
                }
            }
            else if (eep == 1) // xoa eeprom 1
            {
                for (uint16_t i = 0; i <= Max_diachi; i++)
                {
                    myMem.write(i, 0);
                }
            }
            else if (eep == 2) // xoa eeprom 2
            {
                for (uint16_t i = 0; i <= Max_diachi; i++)
                {
                    myMem2.write(i, 0);
                }
            }
            // Serial.println("Đã xóa EEPROM xong");
        }

        int Wifi_RSSI() // RSSI cua Wifi
        {
            if (2 * (WiFi.RSSI() + 100) >= 100) return 100;
            else return (2 * (WiFi.RSSI() + 100));
        }

        String loadChipID()
        {
            uint32_t chipId = 0;
            for (int i = 0; i < 17; i = i + 8)
            {
                chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
            }
            sprintf(apSSID, "VGAS%d", chipId);
            // Serial.printf("\r\n\r\n %s\r\n\r\n", apSSID);
            return String(apSSID);
        }

        void HT_QR(String QR)
        {
            QRCode qrcode;
            uint8_t LCDType = 1;
            uint8_t qrcodeData[qrcode_getBufferSize(11)];
            char wQR[2000];
            sprintf(wQR, "%s", QR.c_str());
            qrcode_initText(&qrcode, qrcodeData, 9, ECC_MEDIUM, wQR);

            u8g2.clearBuffer();
            if (LCDType)    u8g2.drawBox(0, 0, 64, 64);
            const uint8_t y0 = (64 - qrcode.size) / 2;
            const uint8_t x0 = (64 - qrcode.size) / 2;
            for (uint8_t y = 0; y < qrcode.size; y++)
            {
                for (uint8_t x = 0; x < qrcode.size; x++)
                {
                    if (qrcode_getModule(&qrcode, x, y))
                    {
                        if (LCDType == 0)
                            u8g2.setColorIndex(1);
                        else
                            u8g2.setColorIndex(0);
                    }
                    else
                    {
                        if (LCDType == 0)
                            u8g2.setColorIndex(0);
                        else
                            u8g2.setColorIndex(1);
                    }
                    u8g2.drawPixel(x0 + x, y0 + y);
                }
            }
            u8g2.setColorIndex(1);
        }

        void scan_i2c() // scan i2c
        {
            byte error, address;
            int nDevices;
            Serial.println("Scanning...");
            nDevices = 0;
            for (address = 1; address < 127; address++)
            {
                // The i2c_scanner uses the return value of
                // the Write.endTransmisstion to see if
                // a device did acknowledge to the address.
                Wire.beginTransmission(address);
                error = Wire.endTransmission();
                if (error == 0)
                {
                    Serial.print("I2C device found at address 0x");
                    if (address < 16)
                        Serial.print("0");
                    Serial.print(address, HEX);
                    Serial.println("  !");
                    nDevices++;
                }
                else if (error == 4)
                {
                    Serial.print("Unknown error at address 0x");
                    if (address < 16)
                        Serial.print("0");
                    Serial.println(address, HEX);
                }
            }
            if (nDevices == 0)
                Serial.println("No I2C devices found\n");
            else
                Serial.println("done\n");
            delay(1000); // wait 5 seconds for next scan
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