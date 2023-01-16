#include <Arduino.h>
#include <vhitek.h>

namespace VHITEK
{
  char key;
  char keydata[10];
  int keyCount;

  char num_data[50];
  char apSSID[20];
  uint64_t ID;
  password Vpass;

  QueueHandle_t TranctionDataQueue;
  QueueHandle_t TranctionDataQueue_RealTime;

  uint32_t IDX;
  uint32_t IDX_Synch;
  uint32_t IDX_add;

  uint32_t Last_IDX_add;

  SemaphoreHandle_t SPISemaphoreHandle;  // SPI bus semaphore
  uint32_t SPISemaphoreHandleTakenTick;  // SPI bus semaphore taken tick

  SemaphoreHandle_t I2CSemaphoreHandle;  // I2C bus semaphore
  uint32_t I2CSemaphoreHandleTakenTick;  // I2C bus semaphore taken tick

  SemaphoreHandle_t I2C1SemaphoreHandle; // I2C1 bus semaphore
  uint32_t I2C1SemaphoreHandleTakenTick; // I2C1 bus semaphore taken tick

  VDisplay HienThi;
  ledshow hienthiled;
  volatile uint16_t kytu_TL;
  
  uint8_t lastMode = 1;
  uint8_t currentMode = 1; // 0 = menu, 1 = work

  RTC_DS1307 rtc;
  struct_vmSaleDate thoigian;

  ExternalEEPROM myMem;
  ExternalEEPROM myMem2;

  VgasTranactionType last_transac;
  VgasTranactionType Transaction_read;
  VgasTranactionType Trans_realtime;

  uint32_t IDX_add_50;
  VgasTranactionType last_transac_50;
  VgasTranactionType Transac_read_50;

  VgasConfigType VConfig;
  VPETROTotalType VTotal;
  bool price_update;

  bool day_bo_nho;
  int gac_co_1, DangBom, gac_co_2;

  volatile uint32_t newPosition;
  uint32_t Encoder_Value;
  uint32_t poslast;
  float vol, adc;

  bool check_update_FOTA;

  uint32_t last_tien;
  uint32_t last_lit;
  uint32_t last_gia;

  volatile uint32_t TIEN;
  volatile uint32_t LIT;
  volatile uint32_t TIEN_PUM;
  volatile uint32_t LIT_PUM;
  volatile uint32_t PULSE;
  volatile uint32_t tick_encoderA1;
  volatile uint32_t tick_encoderA2;
  volatile uint32_t tick_encoderB1;
  volatile uint32_t tick_encoderB2;
  volatile bool bandau_A;
  volatile bool bandau_B;

  volatile uint32_t NewPos_nguoc1;
  volatile uint32_t NewPos_nguoc2;
  volatile uint32_t tick_KTnguoc1;
  volatile uint32_t tick_KTnguoc2;

  int TotalCo;
  int Last_TotalCo;
  int step_totalco;
  
  volatile int chucnang;
  volatile int step;
  volatile int pos_nhanh;
  int step_ravan;
  volatile bool checkPUM;
  volatile bool buttonevent;
  int xungcalib;
  volatile bool setLTtien;
  volatile bool setLTLIT;

  uint32_t hesodung;
  volatile int typePUM;

  uint32_t last_ONVAN;
  uint32_t last_ONPUM;
  uint32_t tick_TotalCo;
  int kt_movan;

  unsigned short cal_crc_loop_CCITT_A(short l, unsigned char *p)
  {
    int i, j;
    unsigned short seed = 0xFFFF;
    unsigned short cd = 0x8005;
    unsigned short crc = seed;

    for (i = 0; i < l; ++i)
    {
      crc ^= (p[i] << 8);
      for (j = 0; j < 8; ++j)
      {
        if (crc & 0x8000)
          crc = (crc << 1) ^ cd;
        else
          crc <<= 1;
      }
    }
    return crc;
  }
  bool accessI2C1Bus(std::function<void()> &&Callback, uint32_t timeout)
  {
    if (xSemaphoreTake(I2C1SemaphoreHandle, timeout) == pdTRUE)
    {
      I2C1SemaphoreHandleTakenTick = millis();
      Callback();
      xSemaphoreGive(I2C1SemaphoreHandle);
      return true;
    }
    return false;
  }
  bool accessI2CBus(std::function<void()> &&Callback, uint32_t timeout)
  {
    if (xSemaphoreTake(I2CSemaphoreHandle, timeout) == pdTRUE)
    {
      I2CSemaphoreHandleTakenTick = millis();
      Callback();
      xSemaphoreGive(I2CSemaphoreHandle);
      return true;
    }
    return false;
  }
  bool accessSPIBus(std::function<void()> &&Callback, uint32_t timeout)
  {
    if (xSemaphoreTake(SPISemaphoreHandle, timeout) == pdTRUE)
    {
      SPISemaphoreHandleTakenTick = millis();
      Callback();
      SPISemaphoreHandleTakenTick = 0;
      xSemaphoreGive(SPISemaphoreHandle);
      return true;
    }
    SPISemaphoreHandleTakenTick = 0;
    return false;
  }

  void IRAM_ATTR KTnguoc1()
  {
    if (digitalRead(PIN_S1)==1 && digitalRead(PIN_S2)==0)
    {
      if ((uint32_t)(micros() - VHITEK::tick_KTnguoc1) >= 1000)
      {
        VHITEK::NewPos_nguoc1++;
        VHITEK::NewPos_nguoc2 = VHITEK::NewPos_nguoc1;
        VHITEK::tick_KTnguoc1 = micros();
      }
    }
  }
  void IRAM_ATTR KTnguoc2()
  {
    if (digitalRead(PIN_S1)==0 && digitalRead(PIN_S2)==1)
    {
      if ((uint32_t)(micros() - VHITEK::tick_KTnguoc2) >= 1000)
      {
        // VHITEK::NewPos_nguoc2--;
        if(VHITEK::NewPos_nguoc2>0) VHITEK::NewPos_nguoc2--;
        else  VHITEK::NewPos_nguoc2 = VHITEK::NewPos_nguoc1 = 0;
        VHITEK::tick_KTnguoc2 = micros();
      }
    }
  }
  void IRAM_ATTR ISRA()
  {
    KTnguoc1();

    if(digitalRead(PIN_S1)!=VHITEK::bandau_A && VHITEK::NewPos_nguoc2==0 && VHITEK::NewPos_nguoc1==0)
    {
      if (digitalRead(PIN_S1)==1 && digitalRead(PIN_S2)==1)
      {
        if ((uint32_t)(micros() - VHITEK::tick_encoderA1) >= 1000)
        {
          if(VHITEK::newPosition>=VHITEK::PULSE)
          {
            digitalWrite(PIN_VAL1, LOW);
            digitalWrite(PIN_VAL2, LOW);
            digitalWrite(PIN_PUMP, LOW);
            VHITEK::checkPUM=true;
            detachInterrupt(PIN_S1);
            detachInterrupt(PIN_S2);
            VHITEK::step=4;
          }
          else VHITEK::newPosition++;
          VHITEK::tick_encoderA1 = micros();
        }
      } 
      else
      {
        if ((uint32_t)(micros() - VHITEK::tick_encoderA2) >= 1000)
        {
          if(VHITEK::newPosition>=VHITEK::PULSE)
          {            
            digitalWrite(PIN_VAL1, LOW);
            digitalWrite(PIN_VAL2, LOW);
            digitalWrite(PIN_PUMP, LOW);
            VHITEK::checkPUM=true;
            detachInterrupt(PIN_S1);
            detachInterrupt(PIN_S2);
            VHITEK::step=4;
          }
          else VHITEK::newPosition++;
          VHITEK::tick_encoderA2 = micros();
        }
      }
      VHITEK::bandau_A = digitalRead(PIN_S1);
    }
  }
  void IRAM_ATTR ISRB()
  {
    KTnguoc2();

    if(digitalRead(PIN_S2)!=VHITEK::bandau_B && VHITEK::NewPos_nguoc2==0 && VHITEK::NewPos_nguoc1==0)
    {
      if (digitalRead(PIN_S1)==0 && digitalRead(PIN_S2)==1)
      {
        if ((uint32_t)(micros() - VHITEK::tick_encoderB1) >= 1000)
        {  
          if(VHITEK::newPosition>=VHITEK::PULSE)
          {
            digitalWrite(PIN_VAL1, LOW);
            digitalWrite(PIN_VAL2, LOW);
            digitalWrite(PIN_PUMP, LOW);
            VHITEK::checkPUM=true;
            detachInterrupt(PIN_S1);
            detachInterrupt(PIN_S2);
            VHITEK::step=4;
          }
          else VHITEK::newPosition++;
          VHITEK::tick_encoderB1 = micros();
        }
      } 
      else
      {
        if ((uint32_t)(micros() - VHITEK::tick_encoderB2) >= 1000)
        {   
          if(VHITEK::newPosition>=VHITEK::PULSE)
          {
            digitalWrite(PIN_VAL1, LOW);
            digitalWrite(PIN_VAL2, LOW);
            digitalWrite(PIN_PUMP, LOW);
            VHITEK::checkPUM=true;
            detachInterrupt(PIN_S1);
            detachInterrupt(PIN_S2);
            VHITEK::step=4;
          }
          else VHITEK::newPosition++;
          VHITEK::tick_encoderB2 = micros();
        }
      }
      VHITEK::bandau_B = digitalRead(PIN_S2);
    }
  }
  void IRAM_ATTR Atudo()
  {
    KTnguoc1();

    if(digitalRead(PIN_S1)!=VHITEK::bandau_A && VHITEK::NewPos_nguoc2==0 && VHITEK::NewPos_nguoc1==0)
    // if(digitalRead(PIN_S1)!=VHITEK::bandau_A)
    {
      if (digitalRead(PIN_S1)==1 && digitalRead(PIN_S2)==1)
      {
        if ((uint32_t)(micros() - VHITEK::tick_encoderA1) >= 1000)
        {
          VHITEK::newPosition++;
          VHITEK::tick_encoderA1 = micros();
        }
      }
      else 
      {
        if ((uint32_t)(micros() - VHITEK::tick_encoderA2) >= 1000)
        {
          VHITEK::newPosition++;
          VHITEK::tick_encoderA2 = micros();
        }
      }
      VHITEK::bandau_A = digitalRead(PIN_S1);
    }
  }
  void IRAM_ATTR Btudo()
  {
    KTnguoc2();

    if(digitalRead(PIN_S2)!=VHITEK::bandau_B && VHITEK::NewPos_nguoc2==0 && VHITEK::NewPos_nguoc1==0)
    // if(digitalRead(PIN_S2)!=VHITEK::bandau_B)
    {
      if (digitalRead(PIN_S1)==0 && digitalRead(PIN_S2)==1)
      {
        if ((uint32_t)(micros() - VHITEK::tick_encoderB1) >= 1000)
        {
          VHITEK::newPosition++;
          VHITEK::tick_encoderB1 = micros();
        }
      } 
      else 
      {
        if ((uint32_t)(micros() - VHITEK::tick_encoderB2) >= 1000)
        {
          VHITEK::newPosition++;
          VHITEK::tick_encoderB2 = micros();
        }
      }
      VHITEK::bandau_B = digitalRead(PIN_S2);
    }
  }

  /*
   * Task to handle Keypad
   */  

  void TaskKeyPadTime(void *parameter)
  {
    while (1)
    {
      accessI2CBus([&]{
            VHITEK::Keypad::loop(); // Call keypad loop     
            VHITEK::Ds1307::loop();                    
      }, 100);
      vTaskDelay(5 / portTICK_PERIOD_MS ); 
    }
  }

  void menuMode()
  {
    Menu::loop();
  }

  void display(void *parameter)
  {
    while(1)
    {
      if (currentMode == 1)
      {
        VHITEK::Display::loop();
        VHITEK::key = Keypad::getKey();
        if(key)
        {
          if(key=='D' && digitalRead(PIN_CO)!=ON_PUM)
          {
            if (currentMode == 1)
            {
              VHITEK::OTA::OTAServerBegin();
              Menu::begin();
              VHITEK::Keypad::clearKeypad();
              currentMode = 0;
            }
          }
        }
      }
      else menuMode();

      vTaskDelay( 5 / portTICK_PERIOD_MS ); 
    }
  }

  void mainTask(void *parameter)
  {
    static uint32_t last_ledsend=0;
    while (1)
    {
      if (check_update_FOTA == true && day_bo_nho==false) VHITEK::Display::TB_update_FOTA();
      else if(day_bo_nho==false && check_update_FOTA == false) VHITEK::Vgas::loop();

      if(((uint32_t)(millis() - last_ledsend) > 10) or VHITEK::HienThi.thanhtien!=VHITEK::last_tien or VHITEK::HienThi.lit!=VHITEK::last_lit)
      {
        VHITEK::hienthiled.tien = VHITEK::HienThi.thanhtien;
        VHITEK::hienthiled.lit = VHITEK::HienThi.lit;
        VHITEK::hienthiled.gia = VHITEK::VConfig.price;
        VHITEK::hienthiled.tl = VHITEK::kytu_TL;
        VHITEK::hienthiled.checkSum = VHITEK::Config::sumCalc_LED(hienthiled);
        VHITEK::Config::Json_led();

        VHITEK::last_tien = VHITEK::HienThi.thanhtien;
        VHITEK::last_lit = VHITEK::HienThi.lit;
        VHITEK::last_gia = VHITEK::VConfig.price;
        
        last_ledsend = millis();
      }

      vTaskDelay( 5 / portTICK_PERIOD_MS ); 
    }
  }

  void Synch_real_time(void *parameter)
  {
    DynamicJsonDocument doc(10000);
    VgasTranactionType data;

    static uint32_t lastTick_Price=0;
    static uint32_t lastTick_Tranc_realtime=0;

    while(1)
    {
      if(WiFi.status() == WL_CONNECTED) //Real Time update
      {
        if ( ((uint32_t)(millis() - lastTick_Tranc_realtime) > 500) or digitalRead(PIN_CO) == ON_PUM)
        {
          HTTPClient http;
          String json_data;
          int post;
          // Serial.println(apirealtime.c_str());
          http.begin("http://159.223.48.4:85/vgas/real_time_update"); //API
          http.addHeader("Content-Type", "application/json"); 

          Trans_realtime.date = thoigian;
          Trans_realtime.GiaXangDau = VConfig.price;
          if(digitalRead(PIN_CO) == ON_PUM)
          {
            Trans_realtime.TrangThaiCoBom = 1;
            Trans_realtime.SoLitBomDuoc = VHITEK::LIT_PUM;
            Trans_realtime.ThanhTien = VHITEK::TIEN_PUM;
          }
          else 
          {
            Trans_realtime.TrangThaiCoBom = 0;
          }

          json_data=VHITEK::Config::Json_transac_real_time(Trans_realtime);
          post = http.POST(json_data.c_str());

          // Serial.println(json_data);
          String payload = http.getString();
          // Serial.println(payload);  

          if (post == 200)  //Check for the returning code
          { 
            DeserializationError error = deserializeJson(doc, payload);
            if (error == 0)
            {
              if(doc["status"].as<boolean>() == true) //NEU da gui duoc
              {
                // Serial.println(json_data);
                // Serial.println(payload);
                // Serial.println("Real time TRANSACTION");  
              }
            }
          }
          http.end();
          lastTick_Tranc_realtime = millis();
        }
      }

      if(digitalRead(PIN_CO) != ON_PUM && WiFi.status() == WL_CONNECTED) //Check update giá
      {
        if ((uint32_t)(millis() - lastTick_Price) > 500) //3s check 1 lan
        {
          HTTPClient http;
          String apiprice="http://159.223.48.4:85/vgas/get-price/";
          apiprice = apiprice + apSSID;
          http.begin(apiprice.c_str()); //API
          http.addHeader("Content-Type", "application/json");              
          int httpResponseCode = http.GET();

          // Serial.println(apiurl.c_str());       

          if (httpResponseCode == 200)  //Check for the returning code
          { 
            String payload = http.getString();
            DeserializationError error = deserializeJson(doc, payload);
            if (error == 0)
            {
              if(doc["status"].as<boolean>() == true)
              {
                if(doc["price"].as<int>() != VHITEK::VConfig.price)
                {
                  VHITEK::VConfig.price = doc["price"].as<int>();
                  price_update = true;
                  // VHITEK::KPL::setPrice(1, doc["price"].as<int>());
                  // Serial.println("Đã Set giá mới");
                }
              }        
            }
          }
          http.end();  
          lastTick_Price = millis();
        }
      }

      if(VHITEK::Config::countNewData(TranctionDataQueue)!=0) //update Transaction
      { 
        HTTPClient http;
        http.begin("http://159.223.48.4:85/vgas/insert_trade_record");
        http.addHeader("Content-Type", "application/json");   
        data = VHITEK::KPL::getNewData(TranctionDataQueue);           
        String json_data=VHITEK::Config::Json_transac(data);
        // Serial.println(json_data);

        if(WiFi.status() == WL_CONNECTED)
        {
          int post = http.POST(json_data.c_str());
          String payload = http.getString();
          // Serial.println(payload);  

          if (post == 200)  //Check for the returning code
          { 
            DeserializationError error = deserializeJson(doc, payload);
            if (error == 0)
            {
              if(doc["status"].as<boolean>() == true) //NEU da gui duoc
              {
                // Serial.println("Đã gửi được TRANSACTION");
                // Serial.printf(" | IDX: %d - IDX_Synch: %d\n", IDX, IDX_Synch);
              }
            }
          }
          http.end();
        }
      }
      
      vTaskDelay( 5 / portTICK_PERIOD_MS ); 
    }
  }

  void Synch(void *parameter)
  {
    DynamicJsonDocument doc(10000);
    VgasTranactionType data;

    static uint32_t lastTick_FOTA=0;
    static uint32_t lastTick_wifi=0;

    while(1)
    {
      if (!WiFi.isConnected()) //Kiem tra ket noi Wifi
      {
        if ((uint32_t)(millis() - lastTick_wifi) > 5000)
        {
          if (WiFi.reconnect())
          {
            esp_wifi_set_ps(WIFI_PS_NONE);
          }
          lastTick_wifi = millis();
        }
      }

      if((WiFi.status() == WL_CONNECTED))  //Kiem tra update FOTA
      {
        if((uint32_t)(millis() - lastTick_FOTA) > 60000) //60s kiem tra update 1 lan
        {
          // Serial.println("Kiem tra FOTA");
          VHITEK::FOTA::Star_update();
          lastTick_FOTA = millis();
        }  
      }
      
      if(WiFi.status() == WL_CONNECTED) //Check update TranSaction EEPROM
      {
        for(int i=0; i<=64000; i+=sizeof(VgasTranactionType))
        {
          data = VHITEK::EEPROM::read_eeprom_2(i);
          if(data.TrangThaiCoBom==1) //Chưa gửi lên Server
          {
            data.TrangThaiCoBom=0;
            HTTPClient http;
            http.begin("http://159.223.48.4:85/vgas/insert_trade_record");
            http.addHeader("Content-Type", "application/json");              
            String json_data=VHITEK::Config::Json_transac(data);
            // Serial.println(json_data);

            int post = http.POST(json_data.c_str());
            String payload = http.getString();
            // Serial.println(payload);  

            if (post == 200)  //Check for the returning code
            { 
              DeserializationError error = deserializeJson(doc, payload);
              if (error == 0)
              {
                if(doc["status"].as<boolean>() == true) //NEU da gui duoc
                {
                  VHITEK::EEPROM::write_eeprom_2(data, i);

                  if(IDX_Synch>0)
                  {
                    if(IDX_Synch<=0) IDX_Synch=0;
                    else IDX_Synch--;
                    if(IDX_Synch <= maxTransactionEEPromIdx/2) day_bo_nho=false;
                  }

                  // Serial.print("EEPROM - Đã gửi được TRANSACTION");  
                  // Serial.printf(" | IDX: %d - IDX_Synch: %d\n", IDX, IDX_Synch);
                }
              }
            }
            http.end();
          }
          delay(5);
        }
      }

      vTaskDelay( 5 / portTICK_PERIOD_MS ); 
    } //end while
  }

  void begin()
  {
    Serial.begin(115200);
    Wire.begin(25, 26, 100000);    

    Serial2.begin(115200, SERIAL_8N1, 18, 05, false); //kết nối bảng LED; RX=18, TX=5

    // Set PIN MODE
    pinMode(PIN_S1, INPUT);
    pinMode(PIN_S2, INPUT);
    pinMode(35, INPUT);
    analogReadResolution(12);
    
    pinMode(PIN_CO, INPUT_PULLUP);
    pinMode(PIN_PUMP, OUTPUT);
    pinMode(PIN_TOTAL, OUTPUT);
    pinMode(PIN_VAL1, OUTPUT);
    pinMode(PIN_VAL2, OUTPUT);
    digitalWrite(PIN_PUMP, LOW);
    digitalWrite(PIN_TOTAL, LOW);
    digitalWrite(PIN_VAL1, LOW);
    digitalWrite(PIN_VAL2, LOW);

    // ENABLE THE LEVEL SHIFTER (I2C BUS, SERIAL 1 BUS)
    pinMode(PIN_LEVEL_SHIFTER_ENABLE, OUTPUT);
    digitalWrite(PIN_LEVEL_SHIFTER_ENABLE, HIGH);

    TranctionDataQueue = xQueueCreate(50, sizeof(VgasTranactionType));
    TranctionDataQueue_RealTime = xQueueCreate(50, sizeof(VgasTranactionType));

    // Initialize bus semaphore
    vSemaphoreCreateBinary(I2C1SemaphoreHandle);
    vSemaphoreCreateBinary(I2CSemaphoreHandle);
    vSemaphoreCreateBinary(SPISemaphoreHandle);

    EEPROM::begin();
    Display::setup();
    RFID::begin();
    Keypad::setup();
    Ds1307::begin();
    VHITEK::Config::loadChipID();
    VHITEK::OTA::WifiBegin();
    VHITEK::FOTA::FOTAbegin();
    Vgas::setup();
    VHITEK::Config::cai_dat_mat_khau(); //Neu chua luu MK de vai menu thi se tu luu

    // VHITEK::Config::All_Clear_eeprom(2, 64000);
    // VHITEK::Transactions::VD_full_eeprom2();
    // VHITEK::Transactions::clear_trans_1();
    // VHITEK::Transactions::VD_TranSac_1();
    VHITEK::Transactions::load_du_lieu();

    // Start Keypad Task
    xTaskCreateUniversal(TaskKeyPadTime, "KeyPadTask", 10000, NULL, 3, NULL, 1);
    xTaskCreateUniversal(display, "DisplayTask", 10000, NULL, 3, NULL, 1);
    xTaskCreateUniversal(mainTask, "taskMain", 10000, NULL, 2, NULL, 1);
    xTaskCreateUniversal(Synch_real_time, "Task_real_time", 10000, NULL, 3, NULL, CONFIG_ARDUINO_RUNNING_CORE);
    xTaskCreateUniversal(Synch, "Task_Synch", 10000, NULL, 3, NULL, CONFIG_ARDUINO_RUNNING_CORE);

    if(digitalRead(PIN_CO)!=ON_PUM) //Gac co
    {
      gac_co_1=1; //Khong co bom
      DangBom=1;
    } 

  }

} //END namespace VHITEK

void setup()
{
  VHITEK::begin();
}

void loop()
{ 
  // if(VHITEK::HienThi.thanhtien!=VHITEK::last_tien or VHITEK::HienThi.lit!=VHITEK::last_lit or VHITEK::last_gia!=VHITEK::VConfig.price)
  // {
  //   for(int i=0; i<3; i++) VHITEK::Config::Json_led();

  //   VHITEK::last_tien = VHITEK::HienThi.thanhtien;
  //   VHITEK::last_lit = VHITEK::HienThi.lit;
  //   VHITEK::last_gia = VHITEK::VConfig.price;
  // }

  // if(VHITEK::Encoder_Value!=VHITEK::newPosition)
  // {
  //   Serial.printf("NEW: %d - Nguoc1: %d - Nguoc2: %d \n",VHITEK::newPosition, VHITEK::NewPos_nguoc1, VHITEK::NewPos_nguoc2);
  //   VHITEK::Encoder_Value=VHITEK::newPosition;
  // }

//////////////////////////////////////////////////////////////////////////

  // VHITEK::adc = analogRead(35);
  // VHITEK::vol = map(VHITEK::adc, 0, 4095, 0, 5);
  // Serial.printf("Vol: %f\n", VHITEK::vol);

  // digitalWrite(PIN_TOTAL, HIGH);
  // delay(10);
  // digitalWrite(PIN_TOTAL, LOW);
  // delay(5000);

  // digitalWrite(PIN_PUMP, LOW);
  // digitalWrite(PIN_VAL1, LOW);
  // digitalWrite(PIN_VAL2, HIGH);
  // delay(3000);
  // digitalWrite(PIN_PUMP, HIGH);
  // digitalWrite(PIN_VAL1, HIGH);
  // digitalWrite(PIN_VAL2, LOW);
  // delay(3000);
  // Serial.printf("TT co: %d\n", digitalRead(PIN_CO)); //0: Bom - 1: gac co
} 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                          /**** PHÁT XUNG ẢO ****/
/*
bool pos=false;
int thoigianbom;
uint32_t last_tick_bom;

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_CO, INPUT_PULLUP);
  pinMode(PIN_PUMP, OUTPUT);
  pinMode(PIN_TOTAL, OUTPUT);
  pinMode(PIN_VAL1, OUTPUT);
  pinMode(PIN_VAL2, OUTPUT);

  pinMode(PIN_LEVEL_SHIFTER_ENABLE, OUTPUT);
  digitalWrite(PIN_LEVEL_SHIFTER_ENABLE, HIGH);

  digitalWrite(PIN_TOTAL, LOW);
  last_tick_bom=millis();
  thoigianbom = random(2000, 10000);
}

void loop()
{
  if(digitalRead(PIN_CO)==0)
  {
    if(pos==false)
    {
      // Serial.printf("Thoi gian bom: %d\n", thoigianbom);
      if((uint32_t)(millis()-last_tick_bom) < thoigianbom)
      {
        digitalWrite(PIN_TOTAL, HIGH);

        digitalWrite(PIN_VAL1, HIGH);
        delay(20);
        digitalWrite(PIN_VAL2, HIGH);
        delay(20);
        digitalWrite(PIN_VAL1, LOW);
        delay(20);
        digitalWrite(PIN_VAL2, LOW);
        delay(20);
      }
      else 
      {
        digitalWrite(PIN_TOTAL, LOW);
        last_tick_bom=millis();
        pos=true;
      }
    }
    else //if(pos==true)
    {
      thoigianbom = random(2000, 10000);
      pos=false;
      delay(1000);
      last_tick_bom=millis();
    }
  }
  else
  {
    pos=false;
    last_tick_bom=millis();
    digitalWrite(PIN_TOTAL, LOW);
  } 
}
*/