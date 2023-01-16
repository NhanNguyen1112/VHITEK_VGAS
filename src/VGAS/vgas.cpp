#include "vhitek.h"

namespace VHITEK
{
    namespace Vgas
    {
        uint32_t tienlamtron; 
        uint32_t litlamtron; 

        int16_t sumCalc_Total(VPETROTotalType data)
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

        bool write_total(VPETROTotalType total)
        {
            VPETROTotalType read_check_sum;
            byte check_dung=0;

            total.checkSum = sumCalc_Total(total); //tinh check sum cua hanh_dong
            accessI2C1Bus([&]{
                    myMem.put(1100, total); //luu vao eeprom             
            }, 100); 
            accessI2C1Bus([&]{ //Doc de lay ID cua tu 
                    myMem.get(1100, read_check_sum);        
            }, 100);                           

            for(int i=0; i<3; i++) //sao sanh 3 lan
            {
                if(total.checkSum == read_check_sum.checkSum) //neu Sum nhap voa va sum luu bang nhau
                {
                    check_dung ++;
                }
                else
                {
                    check_dung = 0;
                    break;
                } 
            }

            if(check_dung == 3) //neu dung
            {
                // VHITEK::Config::xem_eep_TranSac();
                // Serial.print("Check Dung: "); Serial.println(check_dung);
                // VHITEK::Config::xem_eeprom_tu_bat_ky(user_check.so_tu);
                // VHITEK::Config::xem_tung_o();

                return true;
            }
            else 
            {
                total.checkSum = sumCalc_Total(total); //tinh check sum cua hanh_dong
                accessI2C1Bus([&]{
                        myMem.put(1100, total); //luu vao eeprom             
                }, 100); 
                accessI2C1Bus([&]{ //Doc de lay ID cua tu 
                        myMem.get(1100, read_check_sum);        
                }, 100);      

                if(total.checkSum == read_check_sum.checkSum) //neu dung
                {
                    return true;
                }          
                else 
                {
                    // Serial.println(".........SAI...........");  
                    memset(&read_check_sum, 0, sizeof(read_check_sum));
                    accessI2C1Bus([&]{
                            myMem.put(1100, read_check_sum); //luu vao eeprom              
                    }, 100);        

                    return false;               
                }  
            }
        }
        VPETROTotalType read_total()
        {
            uint32_t Sum_cal;
            VPETROTotalType read;

            accessI2C1Bus([&]{
                    myMem.get(1100, read);       
            }, 100);           

            Sum_cal = sumCalc_Total(read); //tinh CHECK SUM vua doc 
            // Serial.print(" - SUM cal: "); Serial.println(Sum_cal);

            if(Sum_cal == read.checkSum) //Neu check SUM dung
            {
                // check_read_eeprom_2 = true;
                return read;
            }
            // else check_read_eeprom_2 = false;
        }

        void dungravoi();
        void cal_total_co();
        void STOP_PUM();

        void STOP_PUM() //DỪNG BƠM
        {
            detachInterrupt(PIN_S1);
            detachInterrupt(PIN_S2);
            digitalWrite(PIN_VAL1, LOW);
            digitalWrite(PIN_VAL2, LOW);
            digitalWrite(PIN_PUMP, LOW);
        }
        void clear_encoder()
        {
            VHITEK::newPosition = 0;
            VHITEK::Encoder_Value = 0;
        }

        bool totalco_tick()
        {
            // Serial.println("Total co dao trang thai.......1");
            digitalWrite(PIN_TOTAL, !digitalRead(PIN_TOTAL));
            return true;
        }
        void cal_total_co()
        {
            if(digitalRead(PIN_CO)==ON_PUM)
            {
                // Serial.printf("LITPUM: %d - TotalCo: %d - LastToTal: %d - hesodu: %d - TT: %d\n", LIT_PUM, TotalCo, Last_TotalCo, VConfig.lit_total_co, (LIT_PUM - Last_TotalCo));
                if(VHITEK::TotalCo - VHITEK::Last_TotalCo>=1000)
                {
                    if(totalco_tick()==true) VHITEK::tick_TotalCo = millis();
                    
                    if(VConfig.lit_total_co>0)
                    {
                        VHITEK::Last_TotalCo=VHITEK::LIT_PUM;
                        VConfig.lit_total_co=0;
                    }
                    else VHITEK::Last_TotalCo = VHITEK::TotalCo;
                } 

                if((uint32_t)millis()-VHITEK::tick_TotalCo>=30) 
                {
                    digitalWrite(PIN_TOTAL, LOW);
                    // Serial.println("Total co dao trang thai.......2");
                }
            }
        }

        void tinh_xung_calib()
        {
            VHITEK::xungcalib = 5000 - VConfig.calib_pulse;
            // Serial.println(VHITEK::xungcalib);
        }

        void butruxung()
        {
            //// 0:+ ; 1:-
            if(VHITEK::LIT >= 2000 && VHITEK::LIT < 5000) //bu cho 2L
            {
                if(VConfig.pt2 == 0) VHITEK::PULSE += VConfig.bu2;
                else VHITEK::PULSE -= VConfig.bu2;
            }
            else if(VHITEK::LIT >= 5000 && VHITEK::LIT < 10000) //bu cho 5L
            {
                if(VConfig.pt5 == 0) VHITEK::PULSE += VConfig.bu5;
                else VHITEK::PULSE -= VConfig.bu5;
            }
            else if(VHITEK::LIT >= 10000) //bu cho 10L
            {
                if(VConfig.pt10 == 0) VHITEK::PULSE += VConfig.bu10;
                else VHITEK::PULSE -= VConfig.bu10;
            }
        }
        
        void LT_TIEN_LIT() //làm tròn tiền lit
        {
            tinh_xung_calib();

            if(VHITEK::setLTtien==true && VHITEK::setLTLIT==false) //làm tròn tiên
            {
                tienlamtron = 1000-(VHITEK::TIEN_PUM%1000);
                VHITEK::TIEN = VHITEK::TIEN_PUM + tienlamtron;
                VHITEK::LIT = (float)((float)VHITEK::TIEN/VHITEK::VConfig.price)*1000;
                VHITEK::PULSE = (((float)VHITEK::LIT/1000)*(VConfig.pulse*8));
                // Serial.printf("TLT: %d - Tien: %d L: %d - xung: %d - new: %d\n", tienlamtron, VHITEK::TIEN, VHITEK::LIT, VHITEK::PULSE, VHITEK::newPosition);          
            }
            else if(VHITEK::setLTtien==false && VHITEK::setLTLIT==true) //lam tron LIT
            {
                litlamtron = 1000 - (VHITEK::LIT_PUM%1000);
                VHITEK::LIT = VHITEK::LIT_PUM + litlamtron;
                VHITEK::TIEN = ((float)VHITEK::LIT/1000)*VHITEK::VConfig.price;
                VHITEK::PULSE = (((float)VHITEK::LIT/1000)*(VConfig.pulse*8));
                VHITEK::hesodung = (float)((float)(VHITEK::LIT-VConfig.hesoravan)/1000 )*(VConfig.pulse*8);
                // butruxung();
            }

            VHITEK::tick_encoderA1 = micros();
            VHITEK::tick_encoderA2 = micros();
            VHITEK::tick_encoderB1 = micros();
            VHITEK::tick_encoderB2 = micros();
            // Serial.printf("LLT: %d - Tien: %d L: %d - xung: %d - new: %d\n", litlamtron, VHITEK::TIEN, VHITEK::LIT, VHITEK::PULSE, VHITEK::newPosition);
        }

        void dungravoi()
        {
            if(VHITEK::newPosition >= VHITEK::hesodung)
            {
                if(digitalRead(PIN_VAL2) == HIGH) //Val 2 dang mo
                {
                    digitalWrite(PIN_VAL2, LOW); //Tat van 2
                    // Serial.println(" - - - VAL 2 OFF - - -");
                } else {;;}
            } else {;;}          
        }
        
        bool PUMSETUP(int type)
        {
            if(type == 1) //Bom theo lit
            {
                VHITEK::PULSE = (float)(((float)VHITEK::LIT/1000)*(VConfig.pulse*8)) - VHITEK::xungcalib;
                VHITEK::TIEN = (float)((float)VHITEK::LIT/1000)*VHITEK::VConfig.price;

                butruxung();

                VHITEK::hesodung = (float)((float)(VHITEK::LIT-VConfig.hesoravan)/1000 )*(VConfig.pulse*8);
            }
            else if(type == 2) //bom theo tien
            {
                VHITEK::LIT = (float)((float)VHITEK::TIEN/VHITEK::VConfig.price)*1000;
                VHITEK::PULSE = (float) (((float)VHITEK::TIEN/VHITEK::VConfig.price)*(VConfig.pulse*8)) - VHITEK::xungcalib;

                butruxung();

                VHITEK::hesodung = (float)((float)(VHITEK::LIT-VConfig.hesoravan)/1000 )*(VConfig.pulse*8);
            }
            // Serial.printf("L: %d - Pulse: %d - TIEN: %d - Heso: %d\n", LIT, PULSE, TIEN, hesodung);
            return true;
        }

        void CalPUM()
        { 
            VHITEK::LIT_PUM = (float)((float)VHITEK::newPosition*((float)VHITEK::LIT/VHITEK::PULSE));
            VHITEK::TIEN_PUM = ((float)VHITEK::LIT_PUM/1000) * VHITEK::VConfig.price;

            VHITEK::TotalCo = VHITEK::LIT_PUM + VConfig.lit_total_co;
        }

        void setup()
        {
            // attachInterrupt(digitalPinToInterrupt(PIN_S1), ISRA, CHANGE);
            // attachInterrupt(digitalPinToInterrupt(PIN_S2), ISRB, CHANGE);

            accessI2C1Bus([&]{
                    myMem.get(1000, VConfig);       //Load Price, partID, Pulse, calib pulse, hesoravan, lit total co
            }, 100);

            // VConfig.pt2 = 0;
            // VConfig.pt5 = 0;
            // VConfig.pt10 = 0;
            // VConfig.pt50 = 0;

            // VConfig.bu2 = 1;
            // VConfig.bu5 = 1;
            // VConfig.bu10 = 2;
            VConfig.bu50 = 187;

            // VHITEK::VTotal.total_lit=123456;
            // VHITEK::VTotal.total_tien=753432;
            // VHITEK::VTotal.total_ca_lit=634542;
            // VHITEK::VTotal.total_ca_tien=234719;

            // if(write_total(VTotal)==true) 
            VTotal = read_total();

            // VConfig.price = 10000;
            // VConfig.pulse = 100;
            accessI2C1Bus([&]{
                    myMem.put(1000, VConfig);       //Load Price, partID
            }, 100);
            // accessI2C1Bus([&]{
            //         myMem.get(1000, VConfig);       //Load Price, partID
            // }, 100);

            // Serial.printf("EEPROM/ Gia: %d - PartID: %d - Pulse: %d - calib: %d - ravan: %d - litdu: %d\n", VHITEK::VConfig.price, VHITEK::VConfig.partId, VHITEK::VConfig.pulse, VHITEK::VConfig.calib_pulse, VConfig.hesoravan, VConfig.lit_total_co);
            // Serial.printf("ToLIT: %lld - ToTien: %lld - ToCa: %lld - Tienca: %lld\n", VHITEK::VTotal.total_lit, VHITEK::VTotal.total_tien, VHITEK::VTotal.total_ca_lit, VHITEK::VTotal.total_ca_tien);
        }
        
        void Detec_default()
        {
            VHITEK::Transaction_read.TrangThaiCoBom = 1; //1 chưa gui
            VHITEK::Transaction_read.ID = last_transac.ID+1;
            VHITEK::Transaction_read.IdVoiBom = 1;
            VHITEK::Transaction_read.IdLanBom = last_transac.IdLanBom+1;
            VHITEK::Transaction_read.date = thoigian;
            VHITEK::Transaction_read.partId = last_transac.partId;
            VHITEK::Transaction_read.cardId = 0;
            VHITEK::Transaction_read.GiaXangDau = VConfig.price;
            VHITEK::Transaction_read.total = VTotal.total_lit;
            VHITEK::Transaction_read.LoaiXangDau = 1;
            VHITEK::Transaction_read.paymentTerm = 0;

            VHITEK::Transaction_read.SoLitBomDuoc = VHITEK::LIT_PUM;
            if(chucnang==1) //Bơm tự do
            {
                if(VHITEK::setLTtien==false && VHITEK::setLTLIT==false) VHITEK::Transaction_read.ThanhTien = VHITEK::TIEN_PUM;
                else VHITEK::Transaction_read.ThanhTien = VHITEK::TIEN;
            } 
            else VHITEK::Transaction_read.ThanhTien = VHITEK::TIEN; //các chế độ bơm khác
        }

        void Cal_total()
        {
            VConfig.lit_total_co = (uint32_t)(VHITEK::LIT_PUM - (uint32_t)VHITEK::Last_TotalCo);
            VHITEK::TotalCo = 0;
            VHITEK::Last_TotalCo = 0;
            VHITEK::step_totalco=0;

            Detec_default();
            VHITEK::Transactions::transac_save();
            // VHITEK::Transactions::xem_transac(VHITEK::Transaction_read);

            VTotal.total_lit += VHITEK::LIT_PUM;
            VTotal.total_tien += VHITEK::TIEN;
            VTotal.total_ca_lit += VHITEK::LIT_PUM;
            VTotal.total_ca_tien += VHITEK::TIEN;
            write_total(VTotal);

            accessI2C1Bus([&]{
                    myMem.put(1000, VConfig);       //Load Price, partID, Pulse, calib pulse, hesoravan, lit total co
            }, 100);

            // Serial.printf("### LITPUM: %d - TotalCo: %d - LastToTal: %d - hesodu: %d\n", LIT_PUM, TotalCo, Last_TotalCo, VConfig.lit_total_co);
            // Serial.printf("Tien: %lld - LIT:%lld - TienCa: %lld - LitCa: %lld\n", VTotal.total_tien, VTotal.total_lit, VTotal.total_ca_tien, VTotal.total_ca_lit);
            // delay(2);
        }

        void clearpum()
        {
            // memset(&VHITEK::HienThi, 0, sizeof(VHITEK::HienThi));
            VHITEK::TIEN=0; 
            VHITEK::TIEN_PUM=0; 
            VHITEK::LIT=0; 
            VHITEK::LIT_PUM=0; 
            VHITEK::PULSE=0; 
        }

        void clearkeypad()
        {
            VHITEK::Keypad::clearKeypad(keydata, 10);
            keyCount = 0;
        }

        void back_bom()
        {
            clearpum();
            clear_encoder();
            VHITEK::step = 0;
            VHITEK::setLTtien=false;
            VHITEK::setLTLIT=false;
            VHITEK::chucnang=0;
            clearkeypad();
        }

        void action(int type) //0:tudo; 1:lit; 2:tien
        {
            if(VHITEK::step==0)
            {
                tinh_xung_calib();
                VHITEK::checkPUM=false;
                VHITEK::buttonevent=false;
                detachInterrupt(PIN_S1);
                detachInterrupt(PIN_S2);
                VHITEK::step_ravan=0;
                VHITEK::step=1;
                // Serial.printf("step 0 LIT: %d - LITPUM: %d - TIEN: %d - TIENPUM: %d\n", LIT, LIT_PUM, TIEN, TIEN_PUM);
            }
            else if(VHITEK::step==1) //Nhap so lit
            {
                VHITEK::buttonevent=true;
                if(type==1)
                {
                    VHITEK::kytu_TL = 11;
                    VHITEK::HienThi.lit = VHITEK::LIT;
                } 
                else if(type==2)
                {
                    VHITEK::kytu_TL = 12;
                    VHITEK::HienThi.thanhtien = VHITEK::TIEN;
                } 
                if(digitalRead(PIN_CO) == ON_PUM && (VHITEK::LIT>0 or VHITEK::TIEN>0)) //Dở cò -> Bơm
                {
                    if(PUMSETUP(type)==true)
                    {
                        VHITEK::buttonevent=false;
                        clear_encoder();
                        VHITEK::HienThi.thanhtien = VHITEK::TIEN_PUM;
                        VHITEK::HienThi.lit = VHITEK::LIT_PUM;
                        VHITEK::pos_nhanh=0;
                        VHITEK::last_ONPUM=millis();
                        VHITEK::step=2;
                    }                                  
                }  
            }
            else if(VHITEK::step==2) //Mở Bơm, Van
            {
                // Serial.printf("step 2 LIT: %d - LITPUM: %d - TIEN: %d - TIENPUM: %d\n", LIT, LIT_PUM, TIEN, TIEN_PUM);
                if(VHITEK::LIT>0 or VHITEK::TIEN>0) //Nếu số lIT nhập vào > 0
                {
                    if(digitalRead(PIN_CO) != ON_PUM) //gac co
                    {
                        STOP_PUM();
                        back_bom();
                    }
                    else 
                    {
                        VHITEK::HienThi.thanhtien = 0;
                        VHITEK::HienThi.lit = 0;

                        digitalWrite(PIN_PUMP, HIGH);
                        delay(500);                                               
                        digitalWrite(PIN_VAL2, HIGH);
                        delay(200);
                        digitalWrite(PIN_VAL1, HIGH); 
                        delay(200);

                        VHITEK::newPosition=0;
                        VHITEK::NewPos_nguoc1=0;
                        VHITEK::NewPos_nguoc2=0;

                        VHITEK::bandau_A = digitalRead(PIN_S1);
                        VHITEK::bandau_B = digitalRead(PIN_S2);
                        attachInterrupt(digitalPinToInterrupt(PIN_S1), VHITEK::ISRA, CHANGE);
                        attachInterrupt(digitalPinToInterrupt(PIN_S2), VHITEK::ISRB, CHANGE);
                        VHITEK::tick_KTnguoc1 = micros();
                        VHITEK::tick_KTnguoc2 = micros();
                        VHITEK::tick_encoderA1 = micros();
                        VHITEK::tick_encoderA2 = micros();
                        VHITEK::tick_encoderB1 = micros();
                        VHITEK::tick_encoderB2 = micros();
                  
                        VHITEK::step=3;
                    }
                }
                else  //Nếu số lIT nhập vào <= 0
                {
                    if(digitalRead(PIN_CO) == ON_PUM) STOP_PUM(); //dở cò -> Không cho bơm
                    else back_bom();
                }
            }
            else if(VHITEK::step==3) //Thực hiện BƠM
            {
                if(digitalRead(PIN_CO) == ON_PUM) //Dơ cò bơm
                {    
                    if(VHITEK::checkPUM == false) //chưa bơm xong
                    {  
                        if(VHITEK::LIT_PUM>=VConfig.hscangong && VHITEK::LIT_PUM<LIT && VHITEK::newPosition<VHITEK::PULSE)
                        {
                            VHITEK::HienThi.thanhtien = VHITEK::TIEN_PUM;
                            VHITEK::HienThi.lit = VHITEK::LIT_PUM;
                        }  
                        else if(VHITEK::LIT_PUM>=VConfig.hscangong && VHITEK::LIT_PUM>=LIT && VHITEK::newPosition>=VHITEK::PULSE)
                        {
                            VHITEK::HienThi.thanhtien = VHITEK::TIEN;
                            VHITEK::HienThi.lit = VHITEK::LIT;
                        }

                        dungravoi();
                        CalPUM();        
                    }
                } 
                else //gác cò
                {
                    STOP_PUM();
                    if(VHITEK::TIEN_PUM>0 && VHITEK::LIT_PUM>0) //đã bơm
                    {
                        if(VHITEK::checkPUM==false) //đã bơm nhưng chưa bơm xong
                        {
                            VConfig.lastTien = VHITEK::TIEN_PUM;
                            VConfig.lastLit = VHITEK::LIT_PUM; 
                            Cal_total();
                        }                 
                        back_bom();
                    }
                    else back_bom(); //chưa bơm               
                } 
                // Serial.printf("Total Co: %d - LITPUM: %d - LastToTal: %d\n", totalco, LIT_PUM, last_totalco);
            }
            else if(VHITEK::step==4) //Bơm xong, chờ chọn Bơm Nhanh hoặc thoát
            {
                if(digitalRead(PIN_CO) == ON_PUM) //Dơ cò bơm
                {
                    if(VHITEK::checkPUM == true) //Nếu đã bơm xong
                    {
                        if(VHITEK::chucnang==4 or VHITEK::chucnang==2)
                        {
                            if(VHITEK::pos_nhanh==0)
                            {
                                detachInterrupt(PIN_S1);
                                detachInterrupt(PIN_S2);                 
                                VHITEK::HienThi.thanhtien = VConfig.lastTien = VHITEK::TIEN;
                                VHITEK::HienThi.lit = VConfig.lastLit = VHITEK::LIT;                      
                                Cal_total();
                                VHITEK::Vgas::clearkeypad();
                                VHITEK::buttonevent=true;
                                VHITEK::pos_nhanh=1;
                            }
                            else
                            {                            
                                if(VHITEK::key)
                                {
                                    if(VHITEK::key=='#' or VHITEK::key=='*' or VHITEK::key=='A' or VHITEK::key=='B' or VHITEK::key=='C' or VHITEK::key=='D') clearkeypad();
                                    else 
                                    {
                                        clearpum();
                                        VHITEK::newPosition=0;
                                        VHITEK::step=0;                                        
                                    }
                                }
                            }
                        }
                        else 
                        {
                            VHITEK::HienThi.thanhtien = VConfig.lastTien = TIEN;
                            VHITEK::HienThi.lit = VConfig.lastLit = LIT; 
                        }
                    }
                }
                else 
                {
                    STOP_PUM();               
                    back_bom();
                }
            }
        }

        void bomtudo() //Bơm tự do
        {
            if(VHITEK::step==0)
            {
                VHITEK::kytu_TL = 0;
                tinh_xung_calib();
                clearpum();
                clear_encoder();
                clearkeypad(); 
                VHITEK::LIT = 1000000;
                VHITEK::PULSE = 400000;
                VHITEK::step=1;
            }
            else if(VHITEK::step==1) //mở Bơm, Van
            {
                if(digitalRead(PIN_CO) != ON_PUM) //gac co
                {
                    STOP_PUM();
                    back_bom();
                }
                else 
                {
                    VHITEK::HienThi.thanhtien = 0;
                    VHITEK::HienThi.lit = 0;

                    digitalWrite(PIN_PUMP, HIGH);
                    delay(500);                                               
                    digitalWrite(PIN_VAL2, HIGH);
                    delay(200);
                    digitalWrite(PIN_VAL1, HIGH); 
                    delay(200);

                    VHITEK::newPosition=0;
                    VHITEK::NewPos_nguoc1=0;
                    VHITEK::NewPos_nguoc2=0;

                    VHITEK::bandau_A = digitalRead(PIN_S1);
                    VHITEK::bandau_B = digitalRead(PIN_S2);
                    attachInterrupt(digitalPinToInterrupt(PIN_S1), VHITEK::Atudo, CHANGE);
                    attachInterrupt(digitalPinToInterrupt(PIN_S2), VHITEK::Btudo, CHANGE);
                    VHITEK::tick_KTnguoc1 = micros();
                    VHITEK::tick_KTnguoc2 = micros();
                    VHITEK::tick_encoderA1 = micros();
                    VHITEK::tick_encoderA2 = micros();
                    VHITEK::tick_encoderB1 = micros();
                    VHITEK::tick_encoderB2 = micros();
             
                    VHITEK::step=2;
                }
            }
            else if(VHITEK::step == 2) //thực hiện bơm
            {              
                if(digitalRead(PIN_CO) == ON_PUM) //bom
                {
                    if(VHITEK::setLTtien==true or VHITEK::setLTLIT==true) //Nhấn làm tròn tiền or LIT
                    {
                        if(VHITEK::newPosition>=VHITEK::PULSE - 3)
                        {
                            STOP_PUM();
                            Cal_total();
                            VHITEK::step=3;
                        } 
                        else  if(VHITEK::setLTLIT==true) dungravoi();
                    }

                    CalPUM();

                    if(VHITEK::LIT_PUM>=VConfig.hscangong)
                    {
                        if(VHITEK::setLTtien==true or VHITEK::setLTLIT==true)
                        {
                            if(VHITEK::LIT_PUM>=LIT && VHITEK::newPosition>=VHITEK::PULSE)
                            {
                                VHITEK::HienThi.thanhtien = VHITEK::TIEN;
                                VHITEK::HienThi.lit = VHITEK::LIT;
                            }
                            else 
                            {
                                VHITEK::HienThi.thanhtien = VHITEK::TIEN_PUM;
                                VHITEK::HienThi.lit = VHITEK::LIT_PUM;
                            }                             
                        }
                        else //khong nhan lam tron Lit/tien
                        {
                            VHITEK::HienThi.thanhtien = VHITEK::TIEN_PUM;
                            VHITEK::HienThi.lit = VHITEK::LIT_PUM;
                        }
                    }  
                } 
                else //gác cò
                {
                    STOP_PUM();
                    if(VHITEK::TIEN_PUM>1 && VHITEK::LIT_PUM>1)
                    {
                        VConfig.lastTien = VHITEK::TIEN_PUM;
                        VConfig.lastLit = VHITEK::LIT_PUM;
                        Cal_total();
                        back_bom();
                    } else back_bom();
                }    
            }
            else if(VHITEK::step==3) //Dừng bơm, lưu transaction
            {
                VHITEK::HienThi.thanhtien = VHITEK::TIEN;
                VHITEK::HienThi.lit = VHITEK::LIT; 
                if(digitalRead(PIN_CO) == ON_PUM) //còn dở cò
                {
                    if(VHITEK::TIEN_PUM>0 && VHITEK::LIT_PUM>0)
                    {
                        VConfig.lastTien = VHITEK::TIEN;
                        VConfig.lastLit = VHITEK::LIT;
                    }
                }
                else //gác cò 
                {
                    back_bom();
                }
            }
        }

        void loop()
        {
            // if(pos1!=Encoder_Value)
            // {
            //     Serial.printf("Pos 1: %d - Pos2: %d\n", pos1, pos2);
            //     Encoder_Value = pos1;
            // }

            // adc = analogRead(35);
            // vol = map(adc, 0, 4095, 0, 5);
            // Serial.printf("Vol: %f\n", vol);

            if(day_bo_nho == false)
            {
                if(VHITEK::chucnang==1) bomtudo();
                else if(VHITEK::chucnang==2 or VHITEK::chucnang==3) action(2); //2: tien dat truoc; 3: tien bat ky
                else if(VHITEK::chucnang==4 or VHITEK::chucnang==5) action(1); //4: lit dat truoc; 5: lit bat ky
                else //if(chucnang==0)
                {
                    last_ONPUM=millis();
                    last_ONVAN=millis();
                    VHITEK::kytu_TL = 0;
                    VHITEK::HienThi.thanhtien = VConfig.lastTien;
                    VHITEK::HienThi.lit = VConfig.lastLit; 
                    if(digitalRead(PIN_CO)==ON_PUM) VHITEK::chucnang=1;
                }

                cal_total_co();
            }

            // Serial.println(VHITEK::chucnang);
        }
    }
}