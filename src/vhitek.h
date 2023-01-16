
#include <Arduino.h>
#include <SPI.h>
#include "ArduinoJson.h"
#include "RFID.h"
#include "WiFi.h"
#include "SPI.h"
#include <stdlib.h>
#include <qrcode.h>
#include <IoAbstraction.h>
#include <TaskManagerIO.h>
#include <KeyboardManager.h>
#include <IoAbstractionWire.h>
#include <U8g2lib.h>
#include "SparkFun_External_EEPROM.h"
#include "RTClib.h"
#include "time.h"
#include <sys/time.h>
#include "WiFi.h"
#include "AsyncJson.h"
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <esp32fota.h>
#include <esp_pm.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <esp_wifi_types.h>
#include <SPIFFS.h>
#include <FS.h>
#include <SPIFFS.h>
#include <UnixTime.h>

#define MK_chutram 123456 //MK chủ trạm
#define MK_NV 123456 //Setup MK nhân viên
#define MK_KT 123456 //Setup MK cho ky thuat vien

#define MK_vhitek 231223 //MK Ky Thuat Vhitek

#define LCD_RESET 4  // RST on LCD
#define LCD_CS 2     // RS on LCD
#define LCD_CLOCK 19 // E on LCD
#define LCD_DATA 21  // R/W on LCD

#define U8_Width 128
#define U8_Height 64

#define PIN_PUMP 12
#define PIN_VAL1 27
#define PIN_VAL2 14
#define PIN_TOTAL 32

#define PIN_S1 33
#define PIN_S2 34

#define PIN_CO 15 //0: bom; 1: Gac co
#define ON_PUM 0

#define PIN_LEVEL_SHIFTER_ENABLE 13

#define domain "159.223.48.4:85"
#define so_luong_menu 30 //tong so menu
#define FW_TYPE  "VGAS"
#define FW "VGA0.0.1"
#define FW_server 1

namespace VHITEK
{
    typedef struct //Lưu từ ô 0
    {
        uint32_t chutram;
        uint32_t NV;
        uint32_t KT;
        uint32_t vhitek;
    } password;
    extern password Vpass;
    typedef struct  //6
    {
        uint8_t day;
        uint8_t month;
        uint8_t year;
        uint8_t hour;
        uint8_t min;
        uint8_t seconds;
    }struct_vmSaleDate;

    extern RTC_DS1307 rtc;
    extern struct_vmSaleDate thoigian;

    typedef struct // 49 + 7
    {
        uint32_t ID; //4
        uint8_t TrangThaiCoBom; //1
        int8_t IdVoiBom; //1
        struct_vmSaleDate date; //6
        uint32_t partId; //4
        uint64_t cardId; //8
        uint32_t IdLanBom; //4
        uint32_t SoLitBomDuoc; //4
        uint32_t GiaXangDau; //4
        uint32_t ThanhTien; //4
        uint32_t total; //4
        uint16_t LoaiXangDau; //2
        int8_t paymentTerm; //1
        uint16_t checkSum; //2
    } VgasTranactionType;

    typedef struct //Lưu từ ô 1000
    {
        uint32_t partId;
        uint32_t price;
        uint32_t pulse;
        uint32_t calib_pulse;
        uint32_t hesoravan;
        uint32_t hscangong;

        uint8_t pt2;
        uint8_t bu2;

        uint8_t pt5;
        uint8_t bu5;

        uint8_t pt10;
        uint8_t bu10;

        uint8_t pt50; //Chưa sử dụng
        uint8_t bu50; //Chưa sử dụng

        uint32_t lit_total_co;
        uint32_t lastTien;
        uint32_t lastLit;
    } VgasConfigType;

    typedef struct //Lưu từ ô 1100
    {
        uint64_t total_lit;
        uint64_t total_tien;
        uint64_t total_ca_lit;
        uint64_t total_ca_tien;
        uint16_t checkSum;
    } VPETROTotalType;
    extern VPETROTotalType VTotal;

    typedef struct
    {
        uint32_t thanhtien;
        uint32_t lit;
    } VDisplay;
    extern VDisplay HienThi;

    typedef struct
    {
        uint32_t tien;
        uint32_t lit;
        uint32_t gia;
        uint16_t tl; //11: Lit; 12: Tien; 0: clear
        uint16_t checkSum;
    } ledshow;
    extern ledshow hienthiled;
    extern volatile uint16_t kytu_TL;

    extern char key;
    extern char keydata[10];
    extern int keyCount;

    extern QueueHandle_t TranctionDataQueue;
    extern QueueHandle_t TranctionDataQueue_RealTime;
    
    extern ExternalEEPROM myMem;
    extern ExternalEEPROM myMem2;

    extern char num_data[50];
    extern uint8_t currentMode;
    extern U8G2_ST7920_128X64_F_HW_SPI u8g2;
    extern char apSSID[20];
    extern uint64_t ID;

    unsigned short cal_crc_loop_CCITT_A(short l, unsigned char *p);
    bool accessI2C1Bus(std::function<void()> &&Callback, uint32_t timeout);
    bool accessI2CBus(std::function<void()> &&Callback, uint32_t timeout);
    bool accessSPIBus(std::function<void()> &&Callback, uint32_t timeout);

    extern void IRAM_ATTR ISRA();
    extern void IRAM_ATTR ISRB();
    extern void IRAM_ATTR Atudo();
    extern void IRAM_ATTR Btudo();

    extern uint32_t maxTransactionEEPromIdx;
    extern uint32_t IDX;
    extern uint32_t IDX_Synch;
    extern uint32_t IDX_add;
    extern uint32_t Last_IDX_add;

    extern VgasTranactionType last_transac;
    extern VgasTranactionType Transaction_read;
    extern VgasTranactionType Trans_realtime;

    extern uint32_t IDX_add_50;
    extern VgasTranactionType last_transac_50;
    extern VgasTranactionType Transac_read_50;

    extern VgasConfigType VConfig;
    extern bool price_update;

    extern bool day_bo_nho;

    extern int hientai, bandau; 

    extern int gac_co_1, DangBom, gac_co_2;

    extern volatile uint32_t newPosition;
    extern uint32_t Encoder_Value;

    extern float vol, adc;
    extern bool check_update_FOTA;

    extern volatile uint32_t TIEN;
    extern volatile uint32_t LIT;
    extern volatile uint32_t TIEN_PUM;
    extern volatile uint32_t LIT_PUM;
    extern volatile uint32_t PULSE;
    extern volatile uint32_t tick_encoderA1;
    extern volatile uint32_t tick_encoderA2;
    extern volatile uint32_t tick_encoderB1;
    extern volatile uint32_t tick_encoderB2;
    extern volatile bool bandau_A;
    extern volatile bool bandau_B;

    extern volatile uint32_t NewPos_nguoc1;
    extern volatile uint32_t NewPos_nguoc2;
    extern volatile uint32_t tick_KTnguoc1;
    extern volatile uint32_t tick_KTnguoc2;

    extern int TotalCo;
    extern int Last_TotalCo;
    extern int step_totalco;
    
    extern volatile int chucnang;
    extern volatile int step;
    extern volatile int pos_nhanh;
    extern int step_ravan;
    extern volatile bool checkPUM;
    extern volatile bool buttonevent;
    extern int xungcalib;
    extern volatile bool setLTtien;
    extern volatile bool setLTLIT;

    extern uint32_t hesodung;
    extern volatile int typePUM;

    extern uint32_t last_ONVAN;
    extern uint32_t last_ONPUM;
    extern uint32_t tick_TotalCo;
    extern int kt_movan;
    namespace Vgas
    {
        void setup();
        void loop();
        bool write_total(VPETROTotalType total);
        void LT_TIEN_LIT();
        void dungravoi();
        void STOP_PUM();
        void clear_encoder();
        void clearpum();
        void clearkeypad();
        void back_bom();
    }
    
    namespace OTA
    {
        void OTAServerStop();
        void OTAServerLoop();
        void OTAServerInit();
        void WifiBegin();
        void OTAServerBegin();
    }
    namespace FOTA
    {
        void FOTAbegin();
        void Star_update();
    }
    
    namespace KPL
    {
        VgasTranactionType KPL_ReadData(int ID);
        bool setPrice(int ID, uint32_t price);
        uint8_t countNewData(QueueHandle_t queuedata);
        VgasTranactionType getNewData(QueueHandle_t queuedata);
    }
    
    namespace Transactions
    {
        void xem_transac(VgasTranactionType giaodich);
        void VD_full_eeprom2();
        void VD_TranSac_1();
        void clear_trans_1();
        void transac_save();
        void load_du_lieu();
    }

    namespace Menu
    {
        enum MENU_IDX
        {
            menuMainId = 0,
            priceset,
            pulseset,
            pulsecalib,
            hesoravan,
            setxang,
            setsovoi,
            xemlanbom,
            xemTTong,
            xoaTTong,
            xemTCa,
            xoaTCa,
            doipass,
            thongtinmay,
            caidatngay,
            xoaEEPROM2,
            cangong,
            butru,
            kt_rtc,
            kt_bonho,
            menuExitId
        };

        void loop();
        void begin();
        void end();
    }

    namespace Config
    {
        int16_t sumCalc_LED(ledshow data);
        void Json_led();
        String Json_transac_real_time(VgasTranactionType data); // Gui Real time
        String Json_transac(VgasTranactionType data); // lich su giao dich
        String loadChipID();
        void HT_QR(String QR);
        void cai_dat_mat_khau();
        int so_sanh_mk_menu(char *mk);
        int Wifi_RSSI(); // RSSI cua Wifi
        void All_Clear_eeprom(int eep, uint16_t Max_diachi); // Xoa toan bo IC eeprom luu hanh dong mo cua
        uint8_t countNewData(QueueHandle_t queuedata);
        VgasTranactionType getNewData(QueueHandle_t queuedata);
    }

    namespace Keypad
    {
        /* 
        * Chinh sưa thư viện IoAbstraction để nhấn nhanh hơn
        * Vào src -> KeyboardManager.h -> dòng 105 đổi thành 5ms
        */
        void setup();
        void loop();
        char getKey();
        void clearKeypad(char *keyData, int lenght);
        void clearKeypad();
    }

    namespace EEPROM
    {
        void begin();
        bool write_eeprom_2(VgasTranactionType giaodich, uint16_t diachi);
        bool write_eeprom_1(VgasTranactionType giaodich, uint16_t diachi); //50 transaction
        VgasTranactionType read_eeprom_1(uint16_t dia_chi); //50_transaction
        VgasTranactionType read_eeprom_2(uint16_t dia_chi);
    }

    namespace Display
    {
        void loop();
        void manhinh();
        void man_hinh_chinh(VgasTranactionType Data);
        void TB_update_FOTA(); //man hinh thong bao update Fimware
        void hienthi_50lb(uint8_t stt, VgasTranactionType data1, VgasTranactionType data2, VgasTranactionType data3);
        char *ultoa(unsigned long val, char *s);
        void setup();
    }

    namespace RFID
    {
        void begin();
        void loop();
        uint64_t read_id_card();
    }   

    namespace Ds1307
    {
        void begin();
        void loop();
        uint64_t ToTimeStamp(struct_vmSaleDate thoigian);
    } 

    void begin();
}