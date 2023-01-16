#include "vhitek.h"
#include "logo.h"
namespace VHITEK
{
    U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R2, /* CS=*/LCD_CS, /* reset=*/LCD_RESET);
    namespace Display
    {
        void TB_update_FOTA() //man hinh thong bao update Fimware
        {
            u8g2.clearBuffer();
            u8g2.drawFrame(0, 0, 128, 64);
            u8g2.enableUTF8Print();
            u8g2.setFont(u8g2_font_resoledbold_tr);
            u8g2.setFontDirection(0);
            u8g2.setCursor(35, 15);
            u8g2.printf("DANG UPDATE");
            u8g2.setCursor(40, 30);
            u8g2.printf("Vui long");   
            u8g2.setCursor(30, 40);
            u8g2.printf("Cho 3-5 phut");                              
            u8g2.setFont(u8g2_font_ncenB10_tr);
            u8g2.setCursor(20, 60);
            u8g2.printf("VHITEK.VN");                 
            u8g2.sendBuffer();
        }

        void manhinh()
        {
            u8g2.clearBuffer();
            u8g2.drawFrame(0, 0, 128, 64);
            u8g2.setFont(u8g2_font_profont10_mf);
            u8g2.setCursor(5,10);
            u8g2.printf("%d/%d", IDX_Synch, maxTransactionEEPromIdx);
            u8g2.setCursor(95,10);
            u8g2.print(VHITEK::LIT_PUM);
            // u8g2.printf("W: %d", VHITEK::Config::Wifi_RSSI());

            u8g2.setFont(u8g2_font_resoledbold_tr);
            u8g2.setCursor(5,25);
            u8g2.printf("T.Tien: %sVND", VHITEK::Display::ultoa(VHITEK::HienThi.thanhtien, num_data));
            u8g2.setCursor(5,35);
            u8g2.printf("Lit: %s.%03dL",  VHITEK::Display::ultoa(VHITEK::HienThi.lit/1000, num_data),VHITEK::HienThi.lit%1000);
            u8g2.setCursor(5,45);
            u8g2.printf("Gia: %sVND", VHITEK::Display::ultoa(VHITEK::VConfig.price, num_data));

            u8g2.setFont(u8g2_font_resoledbold_tr);
            u8g2.setCursor(60, 10);
            u8g2.print(newPosition);
            
            auto hienthi_bom = [](){
                // u8g2.setFont(u8g2_font_resoledbold_tr);
                // u8g2.setCursor(60, 10);
                // u8g2.print("BOM");
            };
            
            if(VHITEK::chucnang==0)
            {
                u8g2.setCursor(5,60);
                u8g2.printf("%02d:%02d:%02d - %02d/%02d/%02d", thoigian.hour, thoigian.min, thoigian.seconds, thoigian.day, thoigian.month, thoigian.year);
                u8g2.sendBuffer();
                // VHITEK::Vgas::STOP_PUM();
            }
            else if(VHITEK::chucnang == 1) //Bom tu do
            {
                u8g2.setCursor(5,60); 
                if(VHITEK::setLTtien==true && VHITEK::setLTLIT==false) u8g2.printf("Lam tron $: %d", VHITEK::TIEN);
                else if(VHITEK::setLTtien==false && VHITEK::setLTLIT==true) u8g2.printf("Lam tron L: %d", VHITEK::LIT);
                else u8g2.printf("%02d:%02d:%02d - %02d/%02d/%02d", thoigian.hour, thoigian.min, thoigian.seconds, thoigian.day, thoigian.month, thoigian.year);

                if(digitalRead(PIN_CO)==ON_PUM) hienthi_bom(); //Bom
                // else VHITEK::Vgas::STOP_PUM();
                u8g2.sendBuffer();
            }
            else if(VHITEK::chucnang==3) //bom theo tien bat ky
            {
                if(digitalRead(PIN_CO)==ON_PUM) hienthi_bom(); //Bom
                // else VHITEK::Vgas::STOP_PUM();
                u8g2.setCursor(5, 60);
                u8g2.printf("Tien: %c%c%c%c%c%c%c%cVND", 
                                    (keydata[0]) ? keydata[0] : '_',
                                    (keydata[1]) ? keydata[1] : '_',
                                    (keydata[2]) ? keydata[2] : '_',
                                    (keydata[3]) ? keydata[3] : '_',
                                    (keydata[4]) ? keydata[4] : '_',
                                    (keydata[5]) ? keydata[5] : '_',
                                    (keydata[6]) ? keydata[6] : '_',
                                    (keydata[7]) ? keydata[7] : '_');
                u8g2.sendBuffer();
                VHITEK::TIEN = atoi(keydata);
            }
            else if(VHITEK::chucnang==2) //bom theo tien dat truoc
            {
                if(digitalRead(PIN_CO)==ON_PUM && VHITEK::checkPUM==false) hienthi_bom(); //bom
                // else VHITEK::Vgas::STOP_PUM(); 
                u8g2.setCursor(10, 60);
                u8g2.printf("F1 -> Tien: %c0K VND", (keydata[0]) ? keydata[0] : '_');           
                u8g2.sendBuffer();
                VHITEK::TIEN = atoi(keydata)*10000;
            }
            else if(VHITEK::chucnang==4) //bom theo lit dat truoc
            {
                if(digitalRead(PIN_CO)==ON_PUM && VHITEK::checkPUM==false) hienthi_bom(); //bom
                // else VHITEK::Vgas::STOP_PUM();
                u8g2.setCursor(25, 60);
                u8g2.printf("F2 -> Lit: %cL", (keydata[0]) ? keydata[0] : '_');            
                u8g2.sendBuffer();
                VHITEK::LIT = atoi(keydata)*1000;
            }
            else if(VHITEK::chucnang==5) //bom theo lit bat ky
            {
                if(digitalRead(PIN_CO)==ON_PUM) hienthi_bom(); //bom
                // else VHITEK::Vgas::STOP_PUM();
                u8g2.setCursor(20, 60); 
                u8g2.printf("Lit: %c%c%c%c%c%c%c%cL", 
                                (keydata[0]) ? keydata[0] : '_',
                                (keydata[1]) ? keydata[1] : '_',
                                (keydata[2]) ? keydata[2] : '_',
                                (keydata[3]) ? keydata[3] : '_',
                                (keydata[4]) ? keydata[4] : '_',
                                (keydata[5]) ? keydata[5] : '_',
                                (keydata[6]) ? keydata[6] : '_',
                                (keydata[7]) ? keydata[7] : '_');
                u8g2.sendBuffer();               
                VHITEK::LIT = atoi(keydata);
            }
        }

        void hienthi_50lb(uint8_t stt, VgasTranactionType data1, VgasTranactionType data2, VgasTranactionType data3)
        {
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_trixel_square_tf);

            u8g2.setCursor(1,10);
            u8g2.printf("%d.  %d/%d - %02d:%02d", stt, data1.date.day, data1.date.month, data1.date.hour, data1.date.min);
            u8g2.setCursor(10,20);
            u8g2.printf("$: %s - L: %.3f", VHITEK::Display::ultoa(data1.ThanhTien, num_data), (float)data1.SoLitBomDuoc/1000);
            u8g2.setCursor(1,30);
            u8g2.printf("%d.  %d/%d - %02d:%02d", stt+1, data2.date.day, data2.date.month, data2.date.hour, data2.date.min);
            u8g2.setCursor(10,40);
            u8g2.printf("$: %s - L: %.3f", VHITEK::Display::ultoa(data2.ThanhTien, num_data), (float)data2.SoLitBomDuoc/1000);
            
            if(stt!=52)
            {
                u8g2.setCursor(1,50);
                u8g2.printf("%d.  %d/%d - %02d:%02d", stt+2, data3.date.day, data3.date.month, data3.date.hour, data3.date.min);
                u8g2.setCursor(10,60);
                u8g2.printf("$: %s - L: %.3f", VHITEK::Display::ultoa(data3.ThanhTien, num_data), (float)data3.SoLitBomDuoc/1000);
            }
            u8g2.sendBuffer(); 
        }

        void man_hinh_chinh(VgasTranactionType Data)
        {
            u8g2.clearBuffer();
            u8g2.drawFrame(0, 0, 128, 64);
            u8g2.setFont(u8g2_font_resoledbold_tr);
            u8g2.setCursor(5,10);
            u8g2.printf("%d/%d", IDX_Synch, maxTransactionEEPromIdx);
            u8g2.setCursor(80,10);
            u8g2.printf("W: %d", VHITEK::Config::Wifi_RSSI());
            u8g2.setCursor(5,30);
            u8g2.printf("Tien: %.3f", (float)Data.ThanhTien/1000);
            u8g2.setCursor(5,40);
            u8g2.printf("Lit: %.3f", (float)Data.SoLitBomDuoc/1000);
            u8g2.setCursor(5,50);
            u8g2.printf("Gia: %.3f", (float)Data.GiaXangDau/1000);
            u8g2.setCursor(5,60);
            u8g2.printf("%02d:%02d - %02d/%02d/%02d", thoigian.hour, thoigian.min, thoigian.day, thoigian.month, thoigian.year);
            u8g2.sendBuffer();
        }

        char *ultoa(unsigned long val, char *s)
        {
            char *p = s + 13;
            *p = '\0';
            do
            {
                if ((p - s) % 4 == 2)
                    *--p = ',';
                *--p = '0' + val % 10;
                val /= 10;
            } while (val);
            return p;
        }

        void setup()
        {
            SPI.begin(LCD_CLOCK, -1, LCD_DATA, LCD_CS);

            u8g2.setBusClock(700000);   //Chỉ định tốc độ đồng hồ bus (tần số) cho I2C và SPI. 
            //                             Giá trị mặc định sẽ được sử dụng nếu hàm này không được gọi. 
            //                             Lệnh này phải được đặt trước lệnh gọi đầu tiên tới u8g2.begin () hoặc u8g2.initDisplay () .
            u8g2.begin();

            u8g2.enableUTF8Print();
            u8g2.setFont(u8g2_font_lubBI14_tn);
            u8g2.setFontDirection(0);   //xác định hướng vẽ của tất cả các chuỗi; 
                                        // 0: trái->phải; 1: từ trên xuống; 2: phải->trái; 3: xuống đầu trang

            u8g2.clearBuffer();
            u8g2.drawXBM(0, 0, 128, 64, logo_vhitek_bits); //vẽ XBM Bitmap: x, y, w, h, bitmap
            delay(1000);
            u8g2.sendBuffer();
            // Serial.println("Display Module Initialized");
        }

        void loop()
        {
            if(check_update_FOTA  == false)
            {
                if(day_bo_nho == true)
                {
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setFont(u8g2_font_resoledbold_tr);
                    u8g2.setCursor(5,10);
                    u8g2.printf("%d/%d", IDX_Synch, maxTransactionEEPromIdx);
                    u8g2.setCursor(80,10);
                    u8g2.printf("W: %d", VHITEK::Config::Wifi_RSSI());
                    u8g2.setCursor(32,30);
                    u8g2.printf("HET BO NHO!");
                    u8g2.setCursor(30,45);
                    u8g2.printf("KET NOI MANG");
                    u8g2.setCursor(10,55);
                    u8g2.printf("DE DONG BO DU LIEU");
                    u8g2.sendBuffer();
                }
                else accessSPIBus([&]{manhinh();}, 100);
            }
            
            if(VHITEK::key && VHITEK::chucnang!=1) //Các chức năng bơm khác BƠM TỰ DO
            {
                if(VHITEK::chucnang == 0 && key != '*') //bơm theo tiền bất kỳ
                {
                    memset(&VHITEK::HienThi, 0, sizeof(VHITEK::HienThi));
                    VHITEK::chucnang=3; 
                }

                if(key == 'A' && digitalRead(PIN_CO) != ON_PUM) //Bơm theo tiền đặt trước
                {
                    memset(&VHITEK::HienThi, 0, sizeof(VHITEK::HienThi));
                    VHITEK::chucnang=2;
                }
                else if(key == 'B' && digitalRead(PIN_CO) != ON_PUM) //Bơm theo Lit đặt trước
                {
                    memset(&VHITEK::HienThi, 0, sizeof(VHITEK::HienThi));
                    VHITEK::chucnang=4;
                }
                else if(key == 'C' && digitalRead(PIN_CO) != ON_PUM) //Bơm theo Lit bat ky
                {
                    memset(&VHITEK::HienThi, 0, sizeof(VHITEK::HienThi));
                    VHITEK::chucnang=5;
                }
                else if(key == '#' && digitalRead(PIN_CO) != ON_PUM) //Nhan Cancel
                {
                    if(digitalRead(PIN_CO) != ON_PUM)
                    {
                        VHITEK::Vgas::clear_encoder();
                        VHITEK::Vgas::clearpum();
                        VHITEK::Vgas::clearkeypad();
                        VHITEK::Vgas::back_bom();
                        VHITEK::chucnang=0;
                    }                           
                }
                else 
                {
                    if((VHITEK::chucnang == 2 or VHITEK::chucnang == 4) && VHITEK::buttonevent==true) //lit đặt trước, tiền đặt trước
                    {
                        if (keyCount < 1)
                        {
                            keydata[keyCount] = key;
                            keyCount++;
                        }
                        else VHITEK::Vgas::clearkeypad();
                    }
                    else if((VHITEK::chucnang == 3 or VHITEK::chucnang == 5) && digitalRead(PIN_CO) != ON_PUM) //tien bat ky, lit bat ky
                    {
                        if (keyCount < 8)
                        {
                            keydata[keyCount] = key;
                            keyCount++;
                        }
                        else VHITEK::Vgas::clearkeypad();
                    }
                }
            }
            else //BƠM TỰ DO 
            {
                if(digitalRead(PIN_CO) == ON_PUM) //Đang dở cò BƠM
                {
                    if(VHITEK::setLTtien==false && VHITEK::setLTLIT==false)
                    {
                        if(key)
                        {
                            if(key == 'A') VHITEK::setLTtien=true; //Nhấn nút tiền
                            else if(key == 'B') VHITEK::setLTLIT=true; //Nhấn nút LIT   
                            VHITEK::Vgas::LT_TIEN_LIT();
                        }      
                    }
                }
            }
        }

        void ShowWelcome()
        {
            time_t now = time(nullptr);
            u8g2.clearBuffer(); // clear the internal memory
            struct tm *p_tm = localtime(&now);
            u8g2.clearBuffer(); // clear the internal memory
            u8g2.drawFrame(0, 0, 128, 64);
            u8g2.setFont(u8g2_font_timB24_tf); // Courier New Bold 10,12,14,18,24
            u8g2.setCursor(27, 41);
            u8g2.printf("%02d:%02d", p_tm->tm_hour, p_tm->tm_min);
            u8g2.setFont(u8g2_font_ncenB10_tr); // Courier New Bold 10,12,14,18,24
            u8g2.setCursor(20, 63);
            u8g2.printf("VHITEK.VN");
            u8g2.setCursor(27, 14);
            u8g2.printf("%02d/%02d/%02d", p_tm->tm_mday, p_tm->tm_mon + 1, p_tm->tm_year + 1900);
            u8g2.sendBuffer();
        }
    }
}