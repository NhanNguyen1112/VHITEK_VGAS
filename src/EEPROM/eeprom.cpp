#include "vhitek.h"

namespace VHITEK
{
    namespace EEPROM
    {
        void begin()
        {
            Wire1.begin(16, 17,400000); // IC EEPROM U10: 0x51, U8: 0x53
            if(myMem2.begin(0x53,Wire1)==false)
            {
                Serial.println("No memory detected. Freezing.");
                // while (1);      
            }
            if (myMem.begin(0x51, Wire1) == false)
            {
                Serial.println("No memory detected. Freezing.");
                // while (1);
            }            
        }

        int16_t sumCalc_eeprom_2(VgasTranactionType data)
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
        int16_t sumCalc_eeprom_1(VgasTranactionType data) //50_transaction
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

        bool write_eeprom_1(VgasTranactionType giaodich, uint16_t diachi) //50 transaction
        {
            VgasTranactionType read_check_sum;
            byte check_dung=0;

            giaodich.checkSum = sumCalc_eeprom_1(giaodich); //tinh check sum cua hanh_dong
            accessI2C1Bus([&]{
                    myMem.put(diachi, giaodich); //luu vao eeprom             
            }, 100); 
            accessI2C1Bus([&]{ //Doc de lay ID cua tu 
                    myMem.get(diachi, read_check_sum);        
            }, 100);                           

            for(int i=0; i<3; i++) //sao sanh 3 lan
            {
                if(giaodich.checkSum == read_check_sum.checkSum) //neu Sum nhap voa va sum luu bang nhau
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
                giaodich.checkSum = sumCalc_eeprom_1(giaodich); //tinh check sum cua hanh_dong
                accessI2C1Bus([&]{
                        myMem.put(diachi, giaodich); //luu vao eeprom             
                }, 100); 
                accessI2C1Bus([&]{ //Doc de lay ID cua tu 
                        myMem.get(diachi, read_check_sum);        
                }, 100);      

                if(giaodich.checkSum == read_check_sum.checkSum) //neu dung
                {
                    return true;
                }          
                else 
                {
                    // Serial.println(".........SAI...........");  
                    memset(&read_check_sum, 0, sizeof(read_check_sum));
                    accessI2C1Bus([&]{
                            myMem.put(diachi, read_check_sum); //luu vao eeprom              
                    }, 100);        

                    return false;               
                }  
            }
        }
        VgasTranactionType read_eeprom_1(uint16_t dia_chi) //50_transaction
        {
            uint16_t Sum_cal;
            VgasTranactionType read;

            accessI2C1Bus([&]{
                    myMem.get(dia_chi, read); //Doc Check SUM trong eeprom              
            }, 100);           

            Sum_cal = sumCalc_eeprom_1(read); //tinh CHECK SUM vua doc 
            // Serial.print(" - SUM cal: "); Serial.println(Sum_cal);

            if(Sum_cal == read.checkSum) //Neu check SUM dung
            {
                // check_read_eeprom_2 = true;
                return read;
            }
            // else check_read_eeprom_2 = false;
        }

        bool write_eeprom_2(VgasTranactionType giaodich, uint16_t diachi)
        {
            VgasTranactionType read_check_sum;
            byte check_dung=0;

            giaodich.checkSum = sumCalc_eeprom_2(giaodich); //tinh check sum cua hanh_dong
            accessI2C1Bus([&]{
                    myMem2.put(diachi, giaodich); //luu vao eeprom             
            }, 100); 
            accessI2C1Bus([&]{ //Doc de lay ID cua tu 
                    myMem2.get(diachi, read_check_sum);        
            }, 100);                           

            for(int i=0; i<3; i++) //sao sanh 3 lan
            {
                if(giaodich.checkSum == read_check_sum.checkSum) //neu Sum nhap voa va sum luu bang nhau
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
                giaodich.checkSum = sumCalc_eeprom_2(giaodich); //tinh check sum cua hanh_dong
                accessI2C1Bus([&]{
                        myMem2.put(diachi, giaodich); //luu vao eeprom             
                }, 100); 
                accessI2C1Bus([&]{ //Doc de lay ID cua tu 
                        myMem2.get(diachi, read_check_sum);        
                }, 100);      

                if(giaodich.checkSum == read_check_sum.checkSum) //neu dung
                {
                    return true;
                }          
                else 
                {
                    // Serial.println(".........SAI...........");  
                    memset(&read_check_sum, 0, sizeof(read_check_sum));
                    accessI2C1Bus([&]{
                            myMem2.put(diachi, read_check_sum); //luu vao eeprom              
                    }, 100);        

                    return false;               
                }  
            }
        }
        VgasTranactionType read_eeprom_2(uint16_t dia_chi)
        {
            uint16_t Sum_cal;
            VgasTranactionType read;

            accessI2C1Bus([&]{
                    myMem2.get(dia_chi, read); //Doc Check SUM trong eeprom              
            }, 100);           

            Sum_cal = sumCalc_eeprom_2(read); //tinh CHECK SUM vua doc 
            // Serial.print(" - SUM cal: "); Serial.println(Sum_cal);

            if(Sum_cal == read.checkSum) //Neu check SUM dung
            {
                // check_read_eeprom_2 = true;
                return read;
            }
            // else check_read_eeprom_2 = false;
        }
    }
}