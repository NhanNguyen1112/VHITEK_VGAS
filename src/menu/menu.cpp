#include "vhitek.h"
namespace VHITEK
{
    namespace Menu
    {
        typedef std::function<void()> MenuFuncTion;
        typedef struct
        {
            MenuFuncTion fn;
            char menuName[25];
            char userLevel;
            char menuIdx;
            bool enabled;
        } MenuType;        
        MenuType menuELM[so_luong_menu];
        MenuType mainMenuELM[so_luong_menu];
        MenuFuncTion subMenuFunc;
        int MenuID = 0, totalMenuPage, menuPage = 0;
        bool reInit = true;
        int userLevel = 0; 
        char num_data[20];

        void gotoMenu(int menuId)
        {
            if (menuId)
            {
                for (int i = 0; i < so_luong_menu; i++)
                {
                    if (menuELM[i].menuIdx == menuId)
                    {
                        subMenuFunc = menuELM[i].fn;
                        break;
                    }
                }
            }
            MenuID = menuId;
            reInit = true;
            VHITEK::Keypad::clearKeypad();
        }
        void add_menu(const char *name, uint8_t idx, uint8_t level, MenuFuncTion fn)
        {
            for (int i = 0; i < so_luong_menu; i++) 
            {
                if (menuELM[i].menuIdx == idx)
                {
                    // Serial.println("Trung");
                    return;
                }
            }
            for (int i = 0; i < so_luong_menu; i++)
            {
                if (menuELM[i].menuIdx == 0) // kiem tra id co chua
                {
                    sprintf(menuELM[i].menuName, "%s", name);
                    // Serial.printf("%s", name);
                    menuELM[i].menuIdx = idx;
                    menuELM[i].userLevel = level;
                    menuELM[i].fn = fn;
                    // Serial.println("da luu");
                    return;
                }
            }
        }
        void menu_page_up()
        {
            char idx = menuPage + 1;
            if (idx > totalMenuPage)
            {
                menuPage = 0;
                return;
            }
            menuPage++;
        }
        void menu_page_down()
        {
            if (menuPage == 0)
            {
                menuPage = totalMenuPage;
                return;
            }
            menuPage--;
        }
        void end()
        {
            userLevel = 0;
            MenuID = 0;
            VHITEK::Keypad::clearKeypad();
        }
        void mainMenuReinit()
        {
            int totalMainMenuELMCount = 0;
            for (int i = 0; i < 30; i++)
            {
                mainMenuELM[i].enabled = false;
                if ((menuELM[i].userLevel <= userLevel) && (strlen(menuELM[i].menuName)))
                {
                    mainMenuELM[totalMainMenuELMCount] = menuELM[i];
                    mainMenuELM[totalMainMenuELMCount].enabled = true;
                    // Serial.printf("totalMainMenuELMCount = %d %s\n", totalMainMenuELMCount, mainMenuELM[totalMainMenuELMCount].menuName);
                    totalMainMenuELMCount++;
                }
            }

            totalMenuPage = (totalMainMenuELMCount / 7);
            if ((totalMainMenuELMCount - (totalMenuPage * 7)) > 0)
            {
                totalMenuPage++;
            }

            if (totalMenuPage > 0)
            {
                totalMenuPage--;
            }
        }
        void nhap_pass() //nhap pass de vao menu
        {
            char key = Keypad::getKey();

            static char keypadData[6];
            static uint8_t keypadCount = 0;
            if (reInit)
            {
                Keypad::clearKeypad(keypadData, 6);
                Keypad::clearKeypad();
                keypadCount = 0;
                reInit = false;
            }
            if (key)
            {
                if (keypadCount <= 6)
                {
                    keypadData[keypadCount] = key;
                    keypadCount++;
                }
            }

            u8g2.clearBuffer();
            u8g2.drawFrame(0, 0, 128, 64);
            u8g2.enableUTF8Print();
            u8g2.setFontDirection(0);
            u8g2.setFont(u8g2_font_resoledbold_tr);            
            u8g2.setCursor(15,20);
            u8g2.printf("MOI NHAP PASSWORD");
            u8g2.setCursor(35,60);
            u8g2.printf("VHITEK.VN");          

            u8g2.setCursor(30,40);
            u8g2.printf("%c %c %c %c %c %c",
                        (keypadData[0]) ? '*' : '_',
                        (keypadData[1]) ? '*' : '_',
                        (keypadData[2]) ? '*' : '_',
                        (keypadData[3]) ? '*' : '_',
                        (keypadData[4]) ? '*' : '_',
                        (keypadData[5]) ? '*' : '_');
            u8g2.sendBuffer();

            if(keypadCount >= 6)
            {
                if(VHITEK::Config::so_sanh_mk_menu(keypadData) == 4) 
                {
                    // Serial.println("MENU KH");
                    //Menu khach hang
                    userLevel = 1;
                    mainMenuReinit();
                    keypadCount = 0;
                    VHITEK::Keypad::clearKeypad(keypadData, 6);                    
                }
                else if(VHITEK::Config::so_sanh_mk_menu(keypadData) == 1 or VHITEK::Config::so_sanh_mk_menu(keypadData) == 2 or VHITEK::Config::so_sanh_mk_menu(keypadData) == 3)
                {
                    // Serial.println("MENU KT");
                    // Menu cho ky thuat

                    userLevel = 2;
                    mainMenuReinit();
                    keypadCount = 0;
                    VHITEK::Keypad::clearKeypad(keypadData, 6);      
                } 
                else
                {
                    VHITEK::currentMode=1;
                    end();
                }
            }
            // Serial.printf("%c, %c, %c, %c, %c, %c\n", keypadData[0], keypadData[1], keypadData[2], keypadData[3], keypadData[4], keypadData[5]);
        }
        void mainMenu()
        {
            if (reInit == true)
            {
                mainMenuReinit();
                menuPage = 0;
                reInit = false;
            }

            u8g2.refreshDisplay();
            u8g2.setBitmapMode(0);
            u8g2.clearBuffer(); // clear the internal memory
            u8g2.drawFrame(0, 0, 128, 64);
            u8g2.setCursor(110, 10);
            u8g2.setFont(u8g2_font_profont10_mf); // Courier New Bold 10,12,14,18,24
            u8g2.printf("%d/%d", menuPage + 1, totalMenuPage + 1);
            u8g2.setFont(u8g2_font_resoledbold_tr); // Courier New Bold 10,12,14,18,24

            int cursorX = 10;
            uint8_t currentMenu[7];
            uint8_t activeMenuCount = 0;
            for (int i = 0; i < 7; i++)
            {
                uint8_t menuPtr = (menuPage * 7) + i;
                if (mainMenuELM[menuPtr].enabled)
                {
                    u8g2.setCursor(2, cursorX);
                    u8g2.printf("%d.%s", i + 1, mainMenuELM[menuPtr].menuName);
                    currentMenu[activeMenuCount] = mainMenuELM[menuPtr].menuIdx;
                    cursorX += 8;
                    activeMenuCount++;
                }
            }
            u8g2.sendBuffer();

            char key = Keypad::getKey();
            if (key)
            {
                if (key == '8')
                {
                    menu_page_down();
                    return;
                }

                if (key == '0')
                {
                    menu_page_up();
                    return;
                }

                key = key - '0';
                if (key == 0)
                {
                    return;
                }
                if (key <= activeMenuCount)
                {
                    gotoMenu(currentMenu[key - 1]);
                }
            }

            // Serial.printf("MenuID: %d\n", MenuID);
        }
        void loop()
        {
            if (userLevel == 0) nhap_pass(); //nhap password
            else if (userLevel == 1) { //Menu khach hang
                if(MenuID == 0)
                    mainMenu();
                else subMenuFunc();        
            } 
            else if (userLevel == 2) { //Menu ky thuat
                if(MenuID == 0)
                    mainMenu();
                else subMenuFunc();             
            } 

            // Serial.printf("userLevel: %d, MenuID: %d, MenuPage: %d, TotalMenuPage: %d \n",userLevel, MenuID, menuPage, totalMenuPage);
        }
        
        void menu_cai_dat_ngay_gio()
        {
            static struct_vmSaleDate ngay_gio_moi;
            char key = Keypad::getKey();
            static char keydata[2];
            static uint8_t keyCount = 0; 

            static char keydata_Mas[6];
            static uint8_t keyCount_Mas = 0; 

            static uint8_t step=0;

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 2);
                // step = 0;
                keyCount = 0;
                reInit = false;
            }    
            
            if(step == 0) //Nhap Pass MAS
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata_Mas[0]) ? '*' : '_',
                            (keydata_Mas[1]) ? '*' : '_',
                            (keydata_Mas[2]) ? '*' : '_',
                            (keydata_Mas[3]) ? '*' : '_',
                            (keydata_Mas[4]) ? '*' : '_',
                            (keydata_Mas[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount_Mas>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata_Mas) == 1 or VHITEK::Config::so_sanh_mk_menu(keydata_Mas) == 2 or VHITEK::Config::so_sanh_mk_menu(keydata_Mas) == 3) //chu, KT, vhitek
                    { 
                        Keypad::clearKeypad(keydata_Mas, 6);
                        keyCount_Mas = 0;
                        step=1;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata_Mas, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step == 1) //cai dat ngay
            {
                u8g2.clearBuffer();
                u8g2.setCursor(45,10);
                u8g2.printf("CAI DAT");
                u8g2.setCursor(25,20);
                u8g2.printf("NGAY/THANG/NAM");            
                u8g2.setCursor(5,35);
                u8g2.printf("HIEN TAI: %02d/%02d/%02d", thoigian.day, thoigian.month,thoigian.year);             
                u8g2.setCursor(5,50);
                u8g2.printf("NGAY: %c%c",  
                            (keydata[0]) ? keydata[0] : '_',
                            (keydata[1]) ? keydata[1] : '_');                          
                u8g2.setCursor(70,60);
                u8g2.printf("(Enter)->");                               
                u8g2.sendBuffer();   
                ngay_gio_moi.day=atoi(keydata);                               
            }
            else if(step == 2) //cai dat thang
            {
                u8g2.clearBuffer();
                u8g2.setCursor(45,10);
                u8g2.printf("CAI DAT");
                u8g2.setCursor(25,20);
                u8g2.printf("NGAY/THANG/NAM");  
                u8g2.setCursor(5,50);
                u8g2.printf("THANG: %c%c",  
                            (keydata[0]) ? keydata[0] : '_',
                            (keydata[1]) ? keydata[1] : '_');                          
                u8g2.setCursor(70,60);
                u8g2.printf("(Enter)->"); 
                ngay_gio_moi.month=atoi(keydata);

                if(ngay_gio_moi.day>31) //ngay > 31
                {
                    VHITEK::Keypad::clearKeypad(keydata, 2);
                    step = 1;
                }
                else if(ngay_gio_moi.day == 0)
                {
                    ngay_gio_moi.day=thoigian.day;
                    u8g2.setCursor(5,35);
                    u8g2.printf("HIEN TAI: %02d/%02d/%02d", ngay_gio_moi.day, thoigian.month,thoigian.year);             
                    u8g2.sendBuffer();
                }
                else 
                {          
                    u8g2.setCursor(5,35);
                    u8g2.printf("HIEN TAI: %02d/%02d/%02d", ngay_gio_moi.day, thoigian.month,thoigian.year);                                         
                    u8g2.sendBuffer();                    
                }
            }
            else if(step == 3) //cai dat nam
            {
                u8g2.clearBuffer();
                u8g2.setCursor(45,10);
                u8g2.printf("CAI DAT");
                u8g2.setCursor(25,20);
                u8g2.printf("NGAY/THANG/NAM");     
                u8g2.setCursor(5,50);
                u8g2.printf("NAM: %c%c",  
                            (keydata[0]) ? keydata[0] : '_',
                            (keydata[1]) ? keydata[1] : '_');                          
                u8g2.setCursor(70,60);
                u8g2.printf("(Enter)->");   
                ngay_gio_moi.year=atoi(keydata);

                if(ngay_gio_moi.month>12) //thang > 31
                {
                    VHITEK::Keypad::clearKeypad(keydata, 2);
                    step = 2;
                }
                else if(ngay_gio_moi.month==0)
                {
                    ngay_gio_moi.month=thoigian.month;
                    u8g2.setCursor(5,35);
                    u8g2.printf("HIEN TAI: %02d/%02d/%02d", ngay_gio_moi.day, ngay_gio_moi.month, ngay_gio_moi.year);             
                    u8g2.sendBuffer();
                }
                else 
                {
                    if(ngay_gio_moi.year==0)
                    {
                        ngay_gio_moi.year = thoigian.year;
                        u8g2.setCursor(5,35);
                        u8g2.printf("HIEN TAI: %02d/%02d/%02d", ngay_gio_moi.day, ngay_gio_moi.month, ngay_gio_moi.year);             
                        u8g2.sendBuffer();  
                    }
                    else 
                    {
                        ngay_gio_moi.year+=2000;
                        u8g2.setCursor(5,35);
                        u8g2.printf("HIEN TAI: %02d/%02d/%02d", ngay_gio_moi.day, ngay_gio_moi.month, ngay_gio_moi.year);             
                        u8g2.sendBuffer();                         
                    }
                }
            }
            //luu ngay/thang/nam -> hoi xem co muon tiep tuc cai gio/phut/giay
            else if(step == 4) 
            {
                rtc.adjust(DateTime(ngay_gio_moi.year, ngay_gio_moi.month, ngay_gio_moi.day, thoigian.hour, thoigian.min, thoigian.seconds)); //nam, thang, ngay, gio, phut, giay

                u8g2.clearBuffer();
                u8g2.setCursor(45,10);
                u8g2.printf("DA LUU");
                u8g2.setCursor(25,20);
                u8g2.printf("NGAY/THANG/NAM"); 
                u8g2.setCursor(30,35);
                u8g2.printf("%02d:%02d:%02d", ngay_gio_moi.day, ngay_gio_moi.month, ngay_gio_moi.year);     
                u8g2.setCursor(5,50);                         
                u8g2.printf("Enter-> Setup Gio");
                u8g2.setCursor(5,60);
                u8g2.printf("Cancel-> THOAT");
                u8g2.sendBuffer();
            }
            else if(step == 5) //cai dat GIO
            {
                u8g2.clearBuffer();
                u8g2.setCursor(45,10);
                u8g2.printf("CAI DAT");
                u8g2.setCursor(25,20);
                u8g2.printf("GIO:PHUT:GIAY");            
                u8g2.setCursor(5,50);
                u8g2.printf("GIO: %c%c",  
                            (keydata[0]) ? keydata[0] : '_',
                            (keydata[1]) ? keydata[1] : '_');                          
                u8g2.setCursor(70,60);
                u8g2.printf("(Enter)->");                                
                u8g2.setCursor(5,35);
                u8g2.printf("HIEN TAI: %02d:%02d", ngay_gio_moi.hour, ngay_gio_moi.min);  
                u8g2.sendBuffer();            
                ngay_gio_moi.hour = atoi(keydata);      
            }
            else if(step == 6) //cai dat PHUT
            {
                u8g2.clearBuffer();
                u8g2.setCursor(45,10);
                u8g2.printf("CAI DAT");
                u8g2.setCursor(25,20);
                u8g2.printf("GIO:PHUT:GIAY");            
                u8g2.setCursor(5,50);
                u8g2.printf("PHUT: %c%c",  
                            (keydata[0]) ? keydata[0] : '_',
                            (keydata[1]) ? keydata[1] : '_');                          
                u8g2.setCursor(70,60);
                u8g2.printf("(Enter)->");                               
                ngay_gio_moi.min = atoi(keydata);                 

                if(ngay_gio_moi.hour > 23) //co nhan nut NHUNG gio lon hon 23 
                {
                    VHITEK::Keypad::clearKeypad(keydata, 2);
                    step = 5;
                }
                else
                {
                    u8g2.setCursor(5,35);
                    u8g2.printf("HIEN TAI: %02d:%02d", ngay_gio_moi.hour, ngay_gio_moi.min);      
                    u8g2.sendBuffer();       
                }
            }
            else if(step == 7) //KIEM TRA VA LUU PHUT
            {
                if(ngay_gio_moi.min > 59) //co nhan NUT NHUNG phut lon hon 59
                {
                    VHITEK::Keypad::clearKeypad(keydata, 2);
                    step = 6;
                }
                else
                {
                    step = 8;
                }
            }
            else if(step == 8) //Luu Gio:Phut:Giay
            {
                rtc.adjust(DateTime(thoigian.year, thoigian.month, thoigian.day, ngay_gio_moi.hour, ngay_gio_moi.min, ngay_gio_moi.seconds)); //nam, thang, ngay, gio, phut, giay
                u8g2.clearBuffer();
                u8g2.setCursor(45,20);
                u8g2.printf("DA LUU");
                u8g2.setCursor(25,30);
                u8g2.printf("GIO:PHUT:GIAY");  
                u8g2.setCursor(35,45);
                u8g2.printf("%02d:%02d:%02d", ngay_gio_moi.hour, ngay_gio_moi.min, ngay_gio_moi.seconds);                 
                u8g2.sendBuffer();  
                delay(3000);
                gotoMenu(menuMainId);
                reInit = true;
                VHITEK::Keypad::clearKeypad(keydata, 2);
                step = 0; keyCount = 0;                            
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    reInit = true;
                    VHITEK::Keypad::clearKeypad(keydata, 2);
                    step = 0; keyCount = 0;
                    return;
                }
                else if(key == '*' && step!=0) //nhan enter 
                {
                    reInit = true;
                    step++;
                    // VHITEK::Keypad::clearKeypad(keydata, 2);
                }
                else
                {
                    if(step==0) //Nhap Pass Mas
                    {
                        if (keyCount_Mas < 6)
                        {
                            keydata_Mas[keyCount_Mas] = key;
                            keyCount_Mas++;
                        }   
                        else {
                            VHITEK::Keypad::clearKeypad(keydata_Mas, 6);
                            keyCount_Mas = 0;
                        }
                    }
                    else 
                    {
                        if (keyCount < 2)
                        {
                            keydata[keyCount] = key;
                            keyCount++;
                        }   
                        else {
                            VHITEK::Keypad::clearKeypad(keydata, 2);
                            keyCount = 0;
                        } 
                    }
                }            
            }                            
        } 
        void xem_lan_bom() //Xem 50 lần bơm gần nhất
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t keyCount = 0; 
            static uint8_t step=0;
            static int pos=0;
            static int stt=1;

            static int j=0;
            VgasTranactionType tran50[60];
            VgasTranactionType tg;
            
            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 

            if(step==0)
            {
                for(uint16_t i=2000; i<=4968; i+=sizeof(VgasTranactionType))
                {
                    accessI2C1Bus([&]{
                            myMem.get(i, tran50[j]);       
                    }, 100);  
                    j++;
                    delay(1);
                }
                // Serial.printf("j: %d\n",j);
                j=0;
                step=1;
            }
            else if(step==1)
            {
                for(int i = 0; i<=52; i++)
                {
                    for(int j = i + 1; j <= 52; j++)
                    {
                        if(tran50[i].ID < tran50[j].ID)
                        {
                            // Hoan vi 2 so a[i] va a[j]
                            tg = tran50[i];
                            tran50[i] = tran50[j];
                            tran50[j] = tg;        
                        }
                    }
                }

                // for(int i=0; i<=52; i++)
                // {
                //     VHITEK::Transactions::xem_transac(tran50[i]);
                // }
                step=2;
            }
            else if(step==2)
            {
                VHITEK::Display::hienthi_50lb(stt, tran50[pos], tran50[pos+1], tran50[pos+2]);
            }

            if(key)   
            {
                if(step==2)
                {
                    if(key == '8')
                    {
                        stt-=3;
                        pos-=3;
                        if(pos<0)
                        {
                            stt=52;
                            pos=51;
                        } 
                    }   
                    else if(key == '0')
                    {              
                        stt+=3; 
                        pos+=3;
                        if(pos>52)
                        {
                            stt=1;
                            pos=0;
                        } 
                    }
                    // Serial.printf("pos: %d - stt:%d\n", pos, stt);
                }

                if(key == '#') //nhan cancel
                {
                    VHITEK::Keypad::clearKeypad(keydata, 6);
                    keyCount = 0;
                    step=0;
                    pos=0;
                    stt=1;
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*')
                {
                    step++;
                }
                else 
                {
                    if (keyCount < 6)
                    {
                        keydata[keyCount] = key;
                        keyCount++;
                    }   
                    else {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                    }    
                }
            } 
        }
        void Set_price() //cai dat gia
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t keyCount = 0; 
            static uint8_t step=0; 
            static uint32_t price=0;

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 

            if(step==0) //Nhap Pass: chu tram, vhitek
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata[0]) ? '*' : '_',
                            (keydata[1]) ? '*' : '_',
                            (keydata[2]) ? '*' : '_',
                            (keydata[3]) ? '*' : '_',
                            (keydata[4]) ? '*' : '_',
                            (keydata[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata) == 1 or VHITEK::Config::so_sanh_mk_menu(keydata) == 2) //MK: chu tram, vhitek
                    { 
                        Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                        step=1;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step==1)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(30,15);
                u8g2.printf("CAI DAT GIA");
                u8g2.setCursor(5,35);
                u8g2.printf("Gia HT: %d", VConfig.price);
                u8g2.setCursor(5,45);
                u8g2.printf("Gia MOI: %c%c%c%c%c",  
                                (keydata[0]) ? keydata[0] : '_',
                                (keydata[1]) ? keydata[1] : '_',
                                (keydata[2]) ? keydata[2] : '_',
                                (keydata[3]) ? keydata[3] : '_',
                                (keydata[4]) ? keydata[4] : '_');   
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->LUU");
                u8g2.sendBuffer();
                price = atoi(keydata);
            }
            else if(step==2)
            {
                if(price>0)
                {
                    VConfig.price = price;
                    step=3;
                }
                else 
                {
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setFont(u8g2_font_resoledbold_tr);
                    u8g2.setCursor(35,30);
                    u8g2.printf("GIA CAI DAT");
                    u8g2.setCursor(25,40);
                    u8g2.printf("PHAI LON HON 0");
                    u8g2.sendBuffer();

                    delay(2000);
                    reInit = true;
                    gotoMenu(menuMainId);
                    return;
                }              
            }
            else if(step==3)
            {
                accessI2C1Bus([&]{
                    myMem.put(1000, VConfig);       //Load Price, partID
                }, 100);
                // Serial.printf("EEPROM/ Gia: %d - PartID: %d - Pulse: %d\n", VHITEK::VConfig.price, VHITEK::VConfig.partId, VHITEK::VConfig.pulse);

                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(15,35);
                u8g2.printf("DANG LUU GIA MOI");
                u8g2.sendBuffer();

                delay(2000);
                reInit = true;
                gotoMenu(menuMainId);
                return;
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*' && step!=0)
                {
                    step++;
                }
                else 
                {
                    if (keyCount < 6)
                    {
                        keydata[keyCount] = key;
                        keyCount++;
                    }   
                    else {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                    }    
                }
            } 
        }
        void Set_Pulse() //cai dat xung
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t keyCount = 0; 
            static uint8_t step=0; 
            static uint32_t pulse=0;

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 

            if(step==0) //Nhap pass
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata[0]) ? '*' : '_',
                            (keydata[1]) ? '*' : '_',
                            (keydata[2]) ? '*' : '_',
                            (keydata[3]) ? '*' : '_',
                            (keydata[4]) ? '*' : '_',
                            (keydata[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata) == 1) //MK: vhitek
                    { 
                        Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                        step=1;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step==1)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(30,15);
                u8g2.printf("Cai Dat XUNG");
                u8g2.setCursor(5,35);
                u8g2.printf("Xung HT: %d", VConfig.pulse);
                u8g2.setCursor(5,45);
                u8g2.printf("Xung MOI: %c%c%c%c%c%c",  
                                (keydata[0]) ? keydata[0] : '_',
                                (keydata[1]) ? keydata[1] : '_',
                                (keydata[2]) ? keydata[2] : '_',
                                (keydata[3]) ? keydata[3] : '_',
                                (keydata[4]) ? keydata[4] : '_',
                                (keydata[5]) ? keydata[5] : '_');   
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->LUU");
                u8g2.sendBuffer();
                pulse = atoi(keydata);
            }
            else if(step==2)
            {
                if(pulse>0)
                {
                    VConfig.pulse = pulse;
                    step=3;
                }
                else 
                {
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setFont(u8g2_font_resoledbold_tr);
                    u8g2.setCursor(35,30);
                    u8g2.printf("XUNG CAI DAT");
                    u8g2.setCursor(25,40);
                    u8g2.printf("PHAI LON HON 0");
                    u8g2.sendBuffer();

                    delay(2000);
                    reInit = true;
                    gotoMenu(menuMainId);
                    return;
                }              
            }
            else if(step==3)
            {
                accessI2C1Bus([&]{
                    myMem.put(1000, VConfig);       //Load Price, partID
                }, 100);
                // Serial.printf("EEPROM/ Gia: %d - PartID: %d - Pulse: %d\n", VHITEK::VConfig.price, VHITEK::VConfig.partId, VHITEK::VConfig.pulse);

                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(15,35);
                u8g2.printf("DANG LUU XUNG MOI");
                u8g2.sendBuffer();

                delay(2000);
                reInit = true;
                gotoMenu(menuMainId);
                return;
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*' && step!=0)
                {
                    step++;
                }
                else 
                {
                    if (keyCount < 6)
                    {
                        keydata[keyCount] = key;
                        keyCount++;
                    }   
                    else {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                    }    
                }
            } 
        }
        void Set_Pulse_Calib() //Cài đặt xung calib
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t keyCount = 0; 
            static uint8_t step=0; 
            static uint32_t pulse_calib=0;

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 

            if(step==0) //nhap pass
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata[0]) ? '*' : '_',
                            (keydata[1]) ? '*' : '_',
                            (keydata[2]) ? '*' : '_',
                            (keydata[3]) ? '*' : '_',
                            (keydata[4]) ? '*' : '_',
                            (keydata[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata) == 1) //MK: vhitek
                    { 
                        Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                        step=1;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step==1)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(10,15);
                u8g2.printf("Cai Dat Xung Calib");
                u8g2.setCursor(5,35);
                u8g2.printf("Xung HT: %d", VConfig.calib_pulse);
                u8g2.setCursor(5,45);
                u8g2.printf("Xung MOI: %c%c%c%c%c%c",  
                                (keydata[0]) ? keydata[0] : '_',
                                (keydata[1]) ? keydata[1] : '_',
                                (keydata[2]) ? keydata[2] : '_',
                                (keydata[3]) ? keydata[3] : '_',
                                (keydata[4]) ? keydata[4] : '_',
                                (keydata[5]) ? keydata[5] : '_');   
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->LUU");
                u8g2.sendBuffer();
                pulse_calib = atoi(keydata);
            }
            else if(step==2)
            {
                if(pulse_calib>0)
                {
                    VConfig.calib_pulse = pulse_calib;
                    step=3;
                }
                else 
                {
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setFont(u8g2_font_resoledbold_tr);
                    u8g2.setCursor(35,30);
                    u8g2.printf("XUNG CAI DAT");
                    u8g2.setCursor(25,40);
                    u8g2.printf("PHAI LON HON 0");
                    u8g2.sendBuffer();

                    delay(2000);
                    reInit = true;
                    gotoMenu(menuMainId);
                    return;
                }              
            }
            else if(step==3)
            {
                accessI2C1Bus([&]{
                    myMem.put(1000, VConfig);       //Load Price, partID
                }, 100);
                // Serial.printf("EEPROM/ Gia: %d - PartID: %d - Pulse: %d\n", VHITEK::VConfig.price, VHITEK::VConfig.partId, VHITEK::VConfig.pulse);

                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(15,35);
                u8g2.printf("DANG LUU XUNG MOI");
                u8g2.sendBuffer();

                delay(2000);
                reInit = true;
                gotoMenu(menuMainId);
                return;
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*' && step!=0)
                {
                    step++;
                }
                else 
                {
                    if (keyCount < 6)
                    {
                        keydata[keyCount] = key;
                        keyCount++;
                    }   
                    else {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                    }    
                }
            } 
        }
        void HeSoRaVan() //Cài hệ số ra van
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t keyCount = 0; 
            static uint8_t step=0; 
            static uint32_t HeSo=0;

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 

            if(step==0)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(30,15);
                u8g2.printf("He So Ra Van");
                u8g2.setCursor(5,35);
                u8g2.printf("He So HT: %d", VConfig.hesoravan);
                u8g2.setCursor(5,45);
                u8g2.printf("He So MOI: %c%c%c%c%c%c",  
                                (keydata[0]) ? keydata[0] : '_',
                                (keydata[1]) ? keydata[1] : '_',
                                (keydata[2]) ? keydata[2] : '_',
                                (keydata[3]) ? keydata[3] : '_',
                                (keydata[4]) ? keydata[4] : '_',
                                (keydata[5]) ? keydata[5] : '_');   
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->LUU");
                u8g2.sendBuffer();
                HeSo = atoi(keydata);
            }
            else if(step==1)
            {
                if(HeSo>=0 && HeSo<=1000)
                {
                    VConfig.hesoravan = HeSo;
                    step=2;
                }
                else 
                {
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setFont(u8g2_font_resoledbold_tr);
                    u8g2.setCursor(30,30);
                    u8g2.printf("XUNG CAI DAT");
                    u8g2.setCursor(20,43);
                    u8g2.printf("PHAI TU 0->1000");
                    u8g2.sendBuffer();

                    delay(3000);
                    reInit = true;
                    gotoMenu(menuMainId);
                    return;
                }              
            }
            else if(step==2)
            {
                accessI2C1Bus([&]{
                    myMem.put(1000, VConfig);       //Load Price, partID
                }, 100);
                // Serial.printf("EEPROM/ Gia: %d - PartID: %d - Pulse: %d\n", VHITEK::VConfig.price, VHITEK::VConfig.partId, VHITEK::VConfig.pulse);

                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(10,35);
                u8g2.printf("DANG LUU HE SO MOI");
                u8g2.sendBuffer();

                delay(2000);
                reInit = true;
                gotoMenu(menuMainId);
                return;
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*')
                {
                    step++;
                }
                else 
                {
                    if (keyCount < 6)
                    {
                        keydata[keyCount] = key;
                        keyCount++;
                    }   
                    else {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                    }    
                }
            } 
        }
        void Doi_Pass()
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t keyCount = 0; 
            static uint8_t step=0; 
            static uint32_t mode=0;
            static uint32_t PASS=0;

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 

            if(step==0)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata[0]) ? '*' : '_',
                            (keydata[1]) ? '*' : '_',
                            (keydata[2]) ? '*' : '_',
                            (keydata[3]) ? '*' : '_',
                            (keydata[4]) ? '*' : '_',
                            (keydata[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata) == 1 or VHITEK::Config::so_sanh_mk_menu(keydata) == 2) //MK: vhitek, chu tram
                    { 
                        Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                        step=1;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step==1)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(40,15);
                u8g2.printf("DOI PASS");
                u8g2.setCursor(5,35);
                u8g2.printf("1. Chu tram");
                u8g2.setCursor(5,45);
                u8g2.printf("2. Nhan vien");
                u8g2.setCursor(5,55);
                u8g2.printf("3. Ky thuat/Quan ly");
                u8g2.sendBuffer();
            }
            else if(step==2)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                if(mode==1)
                {
                    u8g2.setCursor(15,15);
                    u8g2.printf("DOI PASS CHU TRAM");
                    u8g2.setCursor(5,35);
                    u8g2.printf("Pass cu: %d", Vpass.chutram);
                }
                else if(mode==2)
                {
                    u8g2.setCursor(10,15);
                    u8g2.printf("DOI PASS NHAN VIEN");
                    u8g2.setCursor(5,35);
                    u8g2.printf("Pass cu: %d", Vpass.NV);
                }
                else if(mode==3)
                {
                    u8g2.setCursor(25,15);
                    u8g2.printf("DOI PASS KT/QL");
                    u8g2.setCursor(5,35);
                    u8g2.printf("Pass cu: %d", Vpass.KT);
                }
                u8g2.setCursor(5,45);
                u8g2.printf("Pass MOI: %c%c%c%c%c%c",  
                                (keydata[0]) ? '*' : '_',
                                (keydata[1]) ? '*' : '_',
                                (keydata[2]) ? '*' : '_',
                                (keydata[3]) ? '*' : '_',
                                (keydata[4]) ? '*' : '_',
                                (keydata[5]) ? '*' : '_');   
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->LUU");
                u8g2.sendBuffer();
                PASS = atoi(keydata);
            }
            else if(step==3)
            {
                if(PASS>0)
                {
                    if(mode==1) Vpass.chutram = PASS;
                    else if(mode==2) Vpass.NV = PASS;
                    else if(mode==3) Vpass.KT = PASS;
                    step=4;
                }
                else 
                {
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setFont(u8g2_font_resoledbold_tr);
                    u8g2.setCursor(40,30);
                    u8g2.printf("PASS MOI");
                    u8g2.setCursor(30,40);
                    u8g2.printf("PHAI KHAC 0");
                    u8g2.sendBuffer();

                    delay(2000);
                    reInit = true;
                    gotoMenu(menuMainId);
                    return;
                }              
            }
            else if(step==4)
            {
                accessI2C1Bus([&]{
                    myMem.put(0, Vpass);       //Load Price, partID
                }, 100);
                // Serial.printf("EEPROM/ Gia: %d - PartID: %d - Pulse: %d\n", VHITEK::VConfig.price, VHITEK::VConfig.partId, VHITEK::VConfig.pulse);

                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(15,35);
                u8g2.printf("DANG LUU PASS MOI");
                u8g2.sendBuffer();

                delay(2000);
                reInit = true;
                gotoMenu(menuMainId);
                return;
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    mode = 0;
                    step=0;
                    PASS=0;
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*' && step!=0 && step!=1)
                {
                    step++;
                }
                else 
                {
                    if(step==1)
                    {
                        if(key == '1') mode = 1;
                        else if(key == '2') mode = 2;
                        else if(key == '3') mode = 3;
                        else 
                        {
                            mode = 0;
                            step=0;
                            PASS=0;
                            gotoMenu(menuMainId);
                            return;
                        }
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                        step++;
                    }
                    else 
                    {
                        if (keyCount < 6)
                        {
                            keydata[keyCount] = key;
                            keyCount++;
                        }   
                        else {
                            VHITEK::Keypad::clearKeypad(keydata, 6);
                            keyCount = 0;
                        }   
                    }
                }
            }
        }
        void thong_tin_may()
        {
            String ssid(apSSID);
            char key = Keypad::getKey();

            VHITEK::Config::HT_QR(ssid);

            u8g2.setColorIndex(1);
            u8g2.setFont(u8g2_font_profont10_mf); // Courier New Bold 10,12,14,18,24
            u8g2.setCursor(67,10);
            u8g2.printf("MACHINE INFO"); 
            u8g2.setCursor(65,25);
            u8g2.printf("Firmware:"); 
            u8g2.setCursor(80,35);
            u8g2.print(FW);            
            u8g2.setCursor(65,50);
            u8g2.printf("SSID:");                                     
            u8g2.setCursor(65, 60);
            u8g2.printf("%s", apSSID);
            u8g2.sendBuffer();           

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }             
            }    
        }
        void xem_total_tong()
        {
            char key = Keypad::getKey();
            static char keydata[6];

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                reInit = false;
            } 

            u8g2.clearBuffer();
            u8g2.drawFrame(0, 0, 128, 64);
            u8g2.setCursor(35,15);
            u8g2.printf("TOTAL TONG");            
            u8g2.setCursor(5,30);
            u8g2.printf("L: %s", VHITEK::Display::ultoa(VTotal.total_lit, num_data));   
            u8g2.setCursor(5,45);
            u8g2.printf("$: %s", VHITEK::Display::ultoa(VTotal.total_tien, num_data)); 
            u8g2.setCursor(60,60);
            u8g2.printf("'C'->Thoat"); 
            u8g2.sendBuffer();  

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
            } 
        }
        void xoa_total_tong()
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t step=0;
            static uint8_t keyCount = 0; 

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 
            
            if (step==0)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setCursor(25,15);
                u8g2.printf("XOA TOTAL TONG");            
                u8g2.setCursor(5,30);
                u8g2.printf("L: %s", VHITEK::Display::ultoa(VTotal.total_lit, num_data));   
                u8g2.setCursor(5,45);
                u8g2.printf("$: %s", VHITEK::Display::ultoa(VTotal.total_tien, num_data)); 
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->XOA");
                u8g2.sendBuffer();  
            }
            else if(step==1) //Nhap pass
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata[0]) ? '*' : '_',
                            (keydata[1]) ? '*' : '_',
                            (keydata[2]) ? '*' : '_',
                            (keydata[3]) ? '*' : '_',
                            (keydata[4]) ? '*' : '_',
                            (keydata[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata) == 1) //MK: vhitek
                    {
                        VTotal.total_lit = 0;
                        VTotal.total_tien = 0;
                        // Serial.printf("ToTong: %lld - ToTien: %lld - ToCa: %lld - Tienca: %lld\n", VHITEK::VTotal.total_lit, VHITEK::VTotal.total_tien, VHITEK::VTotal.total_ca_lit, VHITEK::VTotal.total_ca_tien);
                        step=2;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step==2)
            {
                if(VHITEK::Vgas::write_total(VTotal)==true)
                {
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setCursor(10,35);
                    u8g2.printf("DANG XOA TOTAL TONG");
                    u8g2.sendBuffer();   

                    delay(2000);
                    reInit = true;
                    gotoMenu(menuMainId);
                    return;
                }
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*')
                {
                    step++;
                }
                else 
                {
                    if (keyCount < 6)
                    {
                        keydata[keyCount] = key;
                        keyCount++;
                    }   
                    else {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                    }    
                }
            }
        }
        void xem_total_ca()
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t step=0;
            static uint8_t keyCount = 0; 

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 

            u8g2.clearBuffer();
            u8g2.drawFrame(0, 0, 128, 64);
            u8g2.setCursor(40,15);
            u8g2.printf("TOTAL CA");            
            u8g2.setCursor(5,30);
            u8g2.printf("L: %s", VHITEK::Display::ultoa(VTotal.total_ca_lit, num_data));   
            u8g2.setCursor(5,45);
            u8g2.printf("$: %s", VHITEK::Display::ultoa(VTotal.total_ca_tien, num_data)); 
            u8g2.setCursor(60,60);
            u8g2.printf("'C'->Thoat"); 
            u8g2.sendBuffer();  

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    keyCount = 0;
                    step=0;
                    gotoMenu(menuMainId);
                    return;
                }
            } 
        }
        void xoa_total_ca()
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t step=0;
            static uint8_t keyCount = 0; 

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 
            
            if (step==0)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setCursor(30,15);
                u8g2.printf("XOA TOTAL CA");            
                u8g2.setCursor(5,30);
                u8g2.printf("L: %s", VHITEK::Display::ultoa(VTotal.total_ca_lit, num_data));   
                u8g2.setCursor(5,45);
                u8g2.printf("$: %s", VHITEK::Display::ultoa(VTotal.total_ca_tien, num_data)); 
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->XOA");
                u8g2.sendBuffer();  
            }
            else if(step==1) //Nhap pass
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata[0]) ? '*' : '_',
                            (keydata[1]) ? '*' : '_',
                            (keydata[2]) ? '*' : '_',
                            (keydata[3]) ? '*' : '_',
                            (keydata[4]) ? '*' : '_',
                            (keydata[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata) == 1 or VHITEK::Config::so_sanh_mk_menu(keydata) == 2 or VHITEK::Config::so_sanh_mk_menu(keydata) == 3) //MK: vhitek, chu tram, KT
                    {
                        VTotal.total_ca_lit = 0;
                        VTotal.total_ca_tien = 0;
                        // Serial.printf("ToTong: %lld - ToTien: %lld - ToCa: %lld - Tienca: %lld\n", VHITEK::VTotal.total_lit, VHITEK::VTotal.total_tien, VHITEK::VTotal.total_ca_lit, VHITEK::VTotal.total_ca_tien);
                        step=2;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step==2)
            {
                if(VHITEK::Vgas::write_total(VTotal)==true)
                {
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setCursor(15,35);
                    u8g2.printf("DANG XOA TOTAL CA");
                    u8g2.sendBuffer();   

                    delay(2000);
                    reInit = true;
                    gotoMenu(menuMainId);
                    return;
                }
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*' && step!=1)
                {
                    step++;
                }
                else 
                {
                    if (keyCount < 6)
                    {
                        keydata[keyCount] = key;
                        keyCount++;
                    }   
                    else {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                    }    
                }
            } 
        }
        void xoa_lich_su()
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t step=0;
            static uint8_t keyCount = 0; 

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 
            
            if (step==0)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setCursor(35,25);
                u8g2.printf("XOA BO NHO");
                u8g2.setCursor(15,35);
                u8g2.printf("LICH SU GIAO DICH");            
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->XOA");
                u8g2.sendBuffer();  
            }
            else if(step==1) //nhap pass
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata[0]) ? '*' : '_',
                            (keydata[1]) ? '*' : '_',
                            (keydata[2]) ? '*' : '_',
                            (keydata[3]) ? '*' : '_',
                            (keydata[4]) ? '*' : '_',
                            (keydata[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata) == 1) //MK: 1
                    {
                        step=2;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step==2)
            {           
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setCursor(15,35);
                u8g2.printf("DANG XOA LICH SU");
                u8g2.sendBuffer();   

                VHITEK::Config::All_Clear_eeprom(2, 64000);
                VHITEK::Transactions::clear_trans_1();

                delay(1000);
                reInit = true;
                gotoMenu(menuMainId);
                return;
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*' && step!=1)
                {
                    step++;
                }
                else 
                {
                    if (keyCount < 6)
                    {
                        keydata[keyCount] = key;
                        keyCount++;
                    }   
                    else {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                    }    
                }
            }
        }
        void he_so_cang_ong() //cai he so cang ong
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t keyCount = 0; 
            static uint8_t step=0; 
            static uint32_t cangong=0;

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 

            if(step==0) //Nhap Pass: chu tram, KT, vhitek
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata[0]) ? '*' : '_',
                            (keydata[1]) ? '*' : '_',
                            (keydata[2]) ? '*' : '_',
                            (keydata[3]) ? '*' : '_',
                            (keydata[4]) ? '*' : '_',
                            (keydata[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata) == 1 or VHITEK::Config::so_sanh_mk_menu(keydata) == 2 or VHITEK::Config::so_sanh_mk_menu(keydata) == 3) //MK: chu tram, KT, vhitek
                    { 
                        Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                        step=1;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step==1)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(25,15);
                u8g2.printf("HE SO CANG ONG");
                u8g2.setCursor(5,35);
                u8g2.printf("HS CU: %d", VConfig.hscangong);
                u8g2.setCursor(5,45);
                u8g2.printf("HS MOI: %c%c%c",  
                                (keydata[0]) ? keydata[0] : '_',
                                (keydata[1]) ? keydata[1] : '_',
                                (keydata[2]) ? keydata[2] : '_');   
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->LUU");
                u8g2.sendBuffer();
                cangong = atoi(keydata);
            }
            else if(step==2)
            {
                if(cangong>0)
                {
                    VConfig.hscangong = cangong;
                    step=3;
                }
                else 
                {
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setFont(u8g2_font_resoledbold_tr);
                    u8g2.setCursor(35,30);
                    u8g2.printf("HS CAI DAT");
                    u8g2.setCursor(25,40);
                    u8g2.printf("PHAI LON HON 0");
                    u8g2.sendBuffer();

                    delay(2000);
                    reInit = true;
                    gotoMenu(menuMainId);
                    return;
                }  
            }
            else if(step==3)
            {
                accessI2C1Bus([&]{
                    myMem.put(1000, VConfig);       //Load Price, partID
                }, 100);
                // Serial.printf("EEPROM/ Gia: %d - PartID: %d - Pulse: %d\n", VHITEK::VConfig.price, VHITEK::VConfig.partId, VHITEK::VConfig.pulse);

                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(20,35);
                u8g2.printf("DANG LUU HS MOI");
                u8g2.sendBuffer();

                delay(2000);
                reInit = true;
                gotoMenu(menuMainId);
                return;
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*' && step!=0)
                {
                    step++;
                }
                else 
                {
                    if(step==0)
                    {
                        if (keyCount < 6)
                        {
                            keydata[keyCount] = key;
                            keyCount++;
                        }   
                        else {
                            VHITEK::Keypad::clearKeypad(keydata, 6);
                            keyCount = 0;
                        }
                    }
                    else 
                    {
                        if (keyCount < 3)
                        {
                            keydata[keyCount] = key;
                            keyCount++;
                        }   
                        else {
                            VHITEK::Keypad::clearKeypad(keydata, 6);
                            keyCount = 0;
                        }
                    } 
                }
            }
        }
        void he_so_bu() //cai he so bu tru
        {
            char key = Keypad::getKey();
            static char keydata[6];
            static uint8_t keyCount = 0; 
            static uint8_t step=0; 

            static String pthuc[2] = {"+", "-"}; //0:+ ; 1:-

            static uint8_t  pt2=VConfig.pt2;
            static uint8_t  bu2=0;

            static uint8_t  pt5=VConfig.pt5;
            static uint8_t  bu5=0;

            static uint8_t  pt10=VConfig.pt10;
            static uint8_t  bu10=0;

            static uint8_t  pt50=VConfig.pt50;
            static uint8_t  bu50=0; //Số này không để làm gì

            if (reInit == true)
            {
                Keypad::clearKeypad(keydata, 6);
                step = 0;
                keyCount = 0;
                reInit = false;
            } 

            if(step==0) //Nhap Pass: chu tram, KT, vhitek
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.enableUTF8Print();
                u8g2.setFontDirection(0);
                u8g2.setFont(u8g2_font_resoledbold_tr);            
                u8g2.setCursor(15,20);
                u8g2.printf("NHAP LAI PASSWORD");
                u8g2.setCursor(35,60);
                u8g2.printf("VHITEK.VN");          

                u8g2.setCursor(30,40);
                u8g2.printf("%c %c %c %c %c %c",
                            (keydata[0]) ? '*' : '_',
                            (keydata[1]) ? '*' : '_',
                            (keydata[2]) ? '*' : '_',
                            (keydata[3]) ? '*' : '_',
                            (keydata[4]) ? '*' : '_',
                            (keydata[5]) ? '*' : '_');
                u8g2.sendBuffer();

                if(keyCount>=6)
                {
                    if(VHITEK::Config::so_sanh_mk_menu(keydata) == 1 or VHITEK::Config::so_sanh_mk_menu(keydata) == 2 or VHITEK::Config::so_sanh_mk_menu(keydata) == 3) //MK: chu tram, KT, vhitek
                    { 
                        Keypad::clearKeypad(keydata, 6);
                        keyCount = 0;
                        step=1;
                    }
                    else 
                    {
                        VHITEK::Keypad::clearKeypad(keydata, 6);
                        reInit = true;
                        step=0;
                        u8g2.clearBuffer();
                        u8g2.setCursor(25,35);
                        u8g2.printf("SAI PASSWORD!");
                        u8g2.sendBuffer();   
                        delay(2000);  
                        gotoMenu(menuMainId);                   
                    }
                }
            }
            else if(step==1)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(25,10);
                u8g2.printf("MA TRAN BU TRU");
                u8g2.setCursor(5,25);
                u8g2.printf("1/5: %d", VConfig.bu2);
                u8g2.setCursor(70,25);
                u8g2.printf("2/5: %d", VConfig.bu5);
                u8g2.setCursor(5,45);
                u8g2.printf("3/5: %d", VConfig.bu10);
                u8g2.setCursor(55,45);
                u8g2.printf("5/7/2'4: %d", VConfig.bu50);
                u8g2.setCursor(1,60);
                u8g2.printf("'C'->Thoat 'E'->Setup");
                u8g2.sendBuffer();
            }
            else if(step==2) //bu 2L
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(25,10);
                u8g2.printf("MA TRAN BU TRU");
                u8g2.setCursor(5,25);
                u8g2.printf("HS CU: 1/5 = %d", VConfig.bu2);
                u8g2.setCursor(5,35);
                u8g2.printf("P.Thuc: 1/5 = %s", pthuc[pt2]);
                u8g2.setCursor(5,45);
                u8g2.printf("HS MOI: 1/5 = %c%c",  
                                (keydata[0]) ? keydata[0] : '_',
                                (keydata[1]) ? keydata[1] : '_');  
                u8g2.setFont(u8g2_font_profont10_mf);
                u8g2.setCursor(2,60);
                u8g2.printf("'F1'Chon P.Thuc  'E'T.Tuc");
                u8g2.sendBuffer();  
                bu2 = atoi(keydata);                              
            }
            else if(step==3) //bu 5L
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(25,10);
                u8g2.printf("MA TRAN BU TRU");
                u8g2.setCursor(5,25);
                u8g2.printf("HS CU: 2/5 = %d", VConfig.bu5);
                u8g2.setCursor(5,35);
                u8g2.printf("P.Thuc: 2/5 = %s", pthuc[pt5]);
                u8g2.setCursor(5,45);
                u8g2.printf("HS MOI: 2/5 = %c%c",  
                                (keydata[0]) ? keydata[0] : '_',
                                (keydata[1]) ? keydata[1] : '_');  
                u8g2.setFont(u8g2_font_profont10_mf);
                u8g2.setCursor(2,60);
                u8g2.printf("'F1'Chon P.Thuc  'E'T.Tuc");
                u8g2.sendBuffer();  
                bu5 = atoi(keydata);                                
            }
            else if(step==4) //bu 10L
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(25,10);
                u8g2.printf("MA TRAN BU TRU");
                u8g2.setCursor(5,25);
                u8g2.printf("HS CU: 3/5 = %d", VConfig.bu10);
                u8g2.setCursor(5,35);
                u8g2.printf("P.Thuc: 3/5 = %s", pthuc[pt10]);
                u8g2.setCursor(5,45);
                u8g2.printf("HS MOI: 3/5 = %c%c",  
                                (keydata[0]) ? keydata[0] : '_',
                                (keydata[1]) ? keydata[1] : '_');  
                u8g2.setFont(u8g2_font_profont10_mf);
                u8g2.setCursor(2,60);
                u8g2.printf("'F1'Chon P.Thuc  'E'T.Tuc");
                u8g2.sendBuffer();  
                bu10 = atoi(keydata);                                 
            }
            else if(step==5)
            {
                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(13,15);
                u8g2.printf("Tinh ti le ma tran");
                u8g2.setCursor(23,25);
                u8g2.printf("He so mac dinh");
                u8g2.setCursor(5,43);
                u8g2.printf("HS: 5/7/2'4 = %d", VConfig.bu50);
                u8g2.setCursor(5,60);
                u8g2.printf("'C'->Thoat  'E'->LUU");
                u8g2.sendBuffer();                               
            }
            else if(step==6)
            {
                VConfig.pt2 = pt2;
                VConfig.pt5 = pt5;
                VConfig.pt10 = pt10;

                VConfig.bu2 = bu2;
                VConfig.bu5 = bu5;
                VConfig.bu10 = bu10;
                VConfig.bu50 = 187;
                step=7;
            }
            else if(step==7)
            {
                accessI2C1Bus([&]{
                    myMem.put(1000, VConfig);       //Load Price, partID
                }, 100);
                // Serial.printf("EEPROM/ Gia: %d - PartID: %d - Pulse: %d\n", VHITEK::VConfig.price, VHITEK::VConfig.partId, VHITEK::VConfig.pulse);

                u8g2.clearBuffer();
                u8g2.drawFrame(0, 0, 128, 64);
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(5,35);
                u8g2.printf("DANG LUU MA TRAN MOI");
                u8g2.sendBuffer();

                delay(2000);
                reInit = true;
                gotoMenu(menuMainId);
                return;
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    gotoMenu(menuMainId);
                    return;
                }
                else if(key == '*' && step!=0)
                {
                    VHITEK::Keypad::clearKeypad(keydata, 6);
                    keyCount = 0;
                    step++;
                }
                else 
                {
                    if(step==0)
                    {
                        if (keyCount < 6)
                        {
                            keydata[keyCount] = key;
                            keyCount++;
                        }   
                        else {
                            VHITEK::Keypad::clearKeypad(keydata, 6);
                            keyCount = 0;
                        }
                    }
                    else 
                    {
                        if(key=='A' or key=='B' or key=='C' or key=='D')
                        {
                            if(step==2) //bu 2L
                            {                                
                                if(key == 'A')
                                {
                                    pt2++;
                                    if(pt2>1) pt2=0;
                                }
                            }
                            else if(step==3) //bu 5L
                            {                                
                                if(key == 'A')
                                {
                                    pt5++;
                                    if(pt5>1) pt5=0;
                                }
                            }
                            else if(step==4) //bu 10L
                            {                                
                                if(key == 'A')
                                {
                                    pt10++;
                                    if(pt10>1) pt10=0;
                                }
                            }
                            else
                            {
                                VHITEK::Keypad::clearKeypad(keydata, 6);
                                keyCount = 0;
                            }   
                        }
                        else 
                        {
                            if (keyCount < 2)
                            {
                                keydata[keyCount] = key;
                                keyCount++;
                            }   
                            else {
                                VHITEK::Keypad::clearKeypad(keydata, 6);
                                keyCount = 0;
                            }
                        }
                    } 
                }
            }
        }
        void KT_RTC() //Kiem tra hoat dong mach thoi gian DS1307
        {
            char key = Keypad::getKey();
            static bool status;
            static bool pos;

            if(pos == 0)
            {
                if (! rtc.begin(&Wire)) status = 1; //loi
                pos = 1;
            }
            else
            {
                u8g2.clearBuffer();
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(12,15);
                u8g2.printf("Kiem tra hoat dong");
                u8g2.setCursor(8,25);
                u8g2.printf("mach thoi gian thuc");       
                u8g2.setCursor(2,50);
                if (status==1) {
                    u8g2.printf("Trang thai: LOI...!");
                } else u8g2.printf("Trang thai: OK");
                u8g2.sendBuffer();
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    status=0;
                    pos=0;
                    gotoMenu(menuMainId);
                    return;
                }
            }            
        }
        void KT_bo_nho() //Kiem tra hoat dong 2 IC eeprom
        {
            char key = Keypad::getKey();

            static bool status_1;
            static bool status_2;
            static bool pos;

            if(pos == 0)
            {
                if(myMem2.begin(0x53,Wire1)==false) status_2 = 1; //EEPROM 2 loi
                if(myMem.begin(0x51, Wire1) == false) status_1 = 1; //EEPROM 1 loi
                pos = 1;
            }
            else
            {
                u8g2.clearBuffer();
                u8g2.setFont(u8g2_font_resoledbold_tr);
                u8g2.setCursor(12,15);
                u8g2.printf("Kiem tra hoat dong");
                u8g2.setCursor(12,25);
                u8g2.printf("bo nho board mach");       
                u8g2.setCursor(2,45);
                if (status_1==1) {
                    u8g2.printf("Bo nho 1: LOI...!");
                } else u8g2.printf("Bo nho 1: OK");
                u8g2.setCursor(2,55);
                if (status_2==1) {
                    u8g2.printf("Bo nho 2: LOI...!");
                } else u8g2.printf("Bo nho 2: OK");                                 
                u8g2.sendBuffer();
            }

            if(key)   
            {
                if(key == '#') //nhan cancel
                {
                    status_1=0;
                    status_2=0;
                    pos=0;
                    gotoMenu(menuMainId);
                    return;
                }
            }                 
        }        
        void menuExit()
        {
            VHITEK::OTA::OTAServerStop();
            VHITEK::currentMode=1;
            end();
        }

        void begin()
        {
            //Menu cua khach hang
            add_menu("Cai Dat Gia", priceset, 1, Set_price);
            add_menu("Cai Dat Xung", pulseset, 2, Set_Pulse);
            add_menu("Cai Dat Xung Calib", pulsecalib, 2, Set_Pulse_Calib);
            add_menu("He So Ra Van", hesoravan, 1, HeSoRaVan);
            add_menu("Cai Dat Ngay Gio", caidatngay, 1, menu_cai_dat_ngay_gio);
            add_menu("Xem 50 Lan Bom", xemlanbom, 1, xem_lan_bom);
            add_menu("Xem Total Tong", xemTTong, 1, xem_total_tong);
            add_menu("Xem Total Ca", xemTCa, 1, xem_total_ca);
            add_menu("Xoa Total Ca", xoaTCa, 1, xoa_total_ca);
            add_menu("Doi Pass", doipass, 1, Doi_Pass);
            add_menu("Thong tin may", thongtinmay, 1, thong_tin_may);
            
            //Menu cua ky thuat
            add_menu("He So Cang Ong", cangong, 2, he_so_cang_ong);
            add_menu("Ma Tran Bu Tru", butru, 2, he_so_bu);
            add_menu("Xoa Total Tong", xoaTTong, 2, xoa_total_tong);
            add_menu("Xoa Lich Su", xoaEEPROM2, 2, xoa_lich_su);
            add_menu("KT RTC", kt_rtc, 2, KT_RTC);
            add_menu("KT EEPROM", kt_bonho, 2, KT_bo_nho);
            add_menu("THOAT", menuExitId, 1, menuExit);

            userLevel = 0;
            MenuID = 0; reInit=true;
            VHITEK::Keypad::clearKeypad();
        }
    } // namespace menu
} // namespace vhitek