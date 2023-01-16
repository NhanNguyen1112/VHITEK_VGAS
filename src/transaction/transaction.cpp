#include "vhitek.h"

namespace VHITEK
{
    uint32_t maxTransactionEEPromIdx = 64000/sizeof(VgasTranactionType);
    namespace Transactions
    {
        int step=0;

        void VD_full_eeprom2()
        {
            int idx=0;
            VgasTranactionType Data;
            Data.TrangThaiCoBom=0;
            Data.GiaXangDau=20000;
            Data.IdVoiBom=1;
            Data.LoaiXangDau = 119;
            Data.paymentTerm = 0;
            Data.SoLitBomDuoc = 1234;
            Data.total = 955066577;
            Data.partId = 166538123;
            Data.ThanhTien = 23000;
            for(int32_t i=0; i<5; i++)
            {
                Data.ID=idx;
                idx+=7;
                VHITEK::EEPROM::write_eeprom_2(Data, i*sizeof(VgasTranactionType));
            }
        }

        void VD_TranSac_1()
        {
            int idx=0;
            VgasTranactionType Data;
            Data.TrangThaiCoBom=0;
            Data.GiaXangDau=20000;
            Data.IdVoiBom=1;
            Data.LoaiXangDau = 119;
            Data.IdLanBom = 123;
            Data.paymentTerm = 0;
            Data.SoLitBomDuoc = 1234;
            Data.total = 955066577;
            Data.partId = 166538123;
            Data.ThanhTien = 23000;

            for(uint32_t i=2000; i<= 4800; i+=sizeof(VgasTranactionType))
            {
                Data.ID=idx;
                idx++;
                VHITEK::EEPROM::write_eeprom_1(Data, i);
            }
        }

        void clear_trans_1()
        {
            for (uint16_t i = 2000; i <= 5000; i++)
            {
                myMem.write(i, 0);
            }
            // Serial.println("Xoa TranSacTion EEPROM 1");
        }

        void xem_transac(VgasTranactionType giaodich)
        {
            Serial.printf("ID: %d - Part_ID: %d - TThái cò: %d - ID vòi: %d - ID lần bơm: %d - So lít bơm: %d - Giá xăng: %d - ", giaodich.ID, giaodich.partId, giaodich.TrangThaiCoBom, giaodich.IdVoiBom, giaodich.IdLanBom, giaodich.SoLitBomDuoc, giaodich.GiaXangDau);
            Serial.printf(" %d-%d-%d %d:%d:%d - ", giaodich.date.year, giaodich.date.month, giaodich.date.day, giaodich.date.hour, giaodich.date.min, giaodich.date.seconds);
            Serial.printf("Thành tiền: %d - Total: %d - Payment: %d - Loại xăng: %d\n",giaodich.ThanhTien, giaodich.total, giaodich.paymentTerm, giaodich.LoaiXangDau);
        }

        void load_du_lieu() //V2
        {    
            VgasTranactionType data;
            VgasTranactionType read_50_data;
            IDX_add_50 = 2000;
            uint32_t LastID=0;
            uint32_t IDXlast=0;

            static float phantram;     
            uint32_t startTick = millis();     

            for(uint32_t i=2000; i<= 4968; i+=sizeof(VgasTranactionType)) //doc 50 lan bom gan nhat
            {
                accessI2C1Bus([&]{
                        myMem.get(i, read_50_data);       
                }, 100);  

                if(last_transac_50.ID>read_50_data.ID)
                {
                    IDX_add_50 = i;
                    break;
                }
                else
                {
                    last_transac_50 = read_50_data;
                } 
            }
            Serial.printf("IDX 50: %d\n", IDX_add_50);
            xem_transac(last_transac_50);

            for(uint32_t i=0; i<= maxTransactionEEPromIdx-1; i++) //i là IDX
            {
                if((uint32_t)(millis() - startTick) > 200) //Hiển thị %
                {
                    phantram = (float)i/maxTransactionEEPromIdx*100;
                    u8g2.clearBuffer();
                    u8g2.drawFrame(0, 0, 128, 64);
                    u8g2.setFont(u8g2_font_resoledbold_tr);
                    u8g2.setCursor(15,25);
                    u8g2.printf("DANG LOAD DU LIEU"); 
                    u8g2.setFont(u8g2_font_ncenB10_tr);
                    u8g2.setCursor(85,45);
                    u8g2.print("%");                 
                    u8g2.setFont(u8g2_font_ncenB10_tr);
                    u8g2.setCursor(45,45);
                    u8g2.printf("%0.2f", phantram);               
                    u8g2.sendBuffer();   
                    startTick = millis();
                }     

                accessI2C1Bus([&]{
                        myMem2.get(i*sizeof(VgasTranactionType), last_transac);       
                }, 100);  

                if(last_transac.TrangThaiCoBom==1)
                {
                    IDX_Synch++;
                    if(IDX_Synch>= maxTransactionEEPromIdx-1) day_bo_nho=true;
                } 
                
                if(LastID > last_transac.ID)
                {
                    // ReadEEProm;
                    last_transac = data;
                    IDX=IDXlast;
                    IDX_add = IDX*sizeof(VgasTranactionType);
                }
                else 
                {
                    IDXlast = i;
                    data = last_transac;
                    LastID = last_transac.ID;
                }
            }

            last_transac.partId = VConfig.partId;

            Trans_realtime = last_transac;

            VHITEK::HienThi.thanhtien = VConfig.lastTien;
            VHITEK::HienThi.lit = VConfig.lastLit; 

            Serial.printf("EEP2-IDX: %d - IDX_add: %d - IDX_Synch: %d\n", IDX, IDX_add, IDX_Synch);
            xem_transac(last_transac);
        }
        
        void transac_save() //V2
        {
            // auto Detec_default = [&](){
            //     Transaction_read.ID = last_transac.ID;
            //     Transaction_read.IdVoiBom = 1;
            //     Transaction_read.date = thoigian;
            //     Transaction_read.partId = last_transac.partId;
            //     Transaction_read.cardId = 0;
            //     Transaction_read.GiaXangDau = VConfig.price;
            //     Transaction_read.total = VTotal.total_lit;
            //     Transaction_read.LoaiXangDau = 1;
            //     Transaction_read.paymentTerm = 1;
            // };

            if(IDX_Synch < maxTransactionEEPromIdx-1 && day_bo_nho==false)
            {
                if(digitalRead(PIN_CO)!=ON_PUM or VHITEK::checkPUM==true or VHITEK::setLTtien==true or VHITEK::setLTLIT==true) //gac co hoặc Nhấn làm tròn tiền khi bơm tự do
                {
                    // Serial.println("Gac co");
                    // Detec_default();

                    if(last_transac.IdLanBom>VHITEK::Transaction_read.IdLanBom or last_transac.GiaXangDau!=VHITEK::Transaction_read.GiaXangDau or last_transac.total>VHITEK::Transaction_read.total or price_update == true)
                    {
                        VHITEK::VConfig.partId = VHITEK::Ds1307::ToTimeStamp(thoigian);                           
                        last_transac.partId = VHITEK::VConfig.partId;
                        last_transac.GiaXangDau = VHITEK::VConfig.price;
                        VHITEK::Transaction_read.partId = last_transac.partId;
                        accessI2C1Bus([&]{
                            myMem.put(1000, VHITEK::VConfig);  
                            price_update = false;     
                        }, 100);      
                        // Serial.printf("Save/ Gia: %d - PartID: %d\n", VHITEK::VConfig.price, VHITEK::VConfig.partId);     
                    }

                    if(VHITEK::Transaction_read.IdLanBom!=last_transac.IdLanBom && VHITEK::Transaction_read.total!=last_transac.total && VHITEK::Transaction_read.ThanhTien>0)
                    {
                        // Transaction_read.ID += 1;

                        xQueueSend(TranctionDataQueue, &VHITEK::Transaction_read, 100);

                        // Transac_read_50 = Transaction_read;

                        if(VHITEK::EEPROM::write_eeprom_1( VHITEK::Transaction_read, IDX_add_50 ))
                        {
                            last_transac_50 = VHITEK::Transaction_read;
                            IDX_add_50+=sizeof(VgasTranactionType);
                            if(IDX_add_50>=4968) IDX_add_50=2000;

                            // xem_transac(last_transac_50);
                            // Serial.printf("NEW IDX 50: %d\n", IDX_add_50);
                        }

                        if(VHITEK::EEPROM::write_eeprom_2(VHITEK::Transaction_read, IDX*sizeof(VgasTranactionType)))
                        {
                            xem_transac(VHITEK::Transaction_read);
                            
                            last_transac = VHITEK::Transaction_read;
                            // memset(&Transaction_read, 0, sizeof(Transaction_read));
                                                
                            IDX++;
                            if(IDX >= maxTransactionEEPromIdx-1)
                            // if(IDX >= 10)
                            {
                                if(IDX_Synch==0)
                                {
                                    day_bo_nho=false;
                                    IDX=0;
                                }  
                                else day_bo_nho=true;
                            } 
                            IDX_Synch++;

                            // Serial.printf("EE2-TT | IDX: %d - IDX_Synch: %d\n", IDX, IDX_Synch);
                        }     
                    }
                    // xem_transac(Transaction_read);
                }
                else  DangBom=0;
            }
        }

    } //transaction
} //vhitek