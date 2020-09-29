
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <Adafruit_NeoPixel.h>

#define led  2

#include <DNSServer.h>

#define NUM_LEDS 256

ESP8266HTTPUpdateServer httpUpdater;

Adafruit_NeoPixel pixels(NUM_LEDS, D5, NEO_GRB + NEO_KHZ800);

#define LIGHT_PIN 7

int STR_NUM = 16; //默认汉字数量对应的字模16进制
int strCnt = 0;
int tipLight = 150;
int addrTipLight = 4092;
int warnLight = 80;
int addrWarnLight = 4082;
int darkLight = 200;
int addrDarkLight = 4072;

const byte DNS_PORT = 53;
//DNSServer dnsServer;

ESP8266WebServer WifiServer(80);
bool isDisplayFromCell = true;



const unsigned char table[][16] = {
  //   恶(0) 灵(1) 避(2) 让(3)  (4)  (5) 诸(6) 邪(7) 退(8) 散(9)

  {0x00, 0x00, 0x7F, 0xFC, 0x04, 0x40, 0x24, 0x48, 0x14, 0x48, 0x14, 0x50, 0x04, 0x40, 0xFF, 0xFE},
  {0x00, 0x00, 0x01, 0x00, 0x08, 0x88, 0x48, 0x84, 0x48, 0x12, 0x48, 0x12, 0x87, 0xF0, 0x00, 0x00}, /*"恶",0*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x00, 0x3F, 0xF8, 0x00, 0x08, 0x00, 0x08, 0x1F, 0xF8, 0x00, 0x08, 0x00, 0x08, 0x3F, 0xF8},
  {0x01, 0x00, 0x11, 0x10, 0x11, 0x10, 0x22, 0xA0, 0x04, 0x40, 0x08, 0x20, 0x30, 0x18, 0xC0, 0x06}, /*"灵",1*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x10, 0x47, 0x88, 0x24, 0xBE, 0x24, 0x80, 0x04, 0xA2, 0x07, 0x94, 0xE4, 0x3E, 0x28, 0x08},
  {0x2F, 0x88, 0x34, 0xBE, 0x24, 0x88, 0x27, 0x88, 0x24, 0x88, 0x50, 0x00, 0x8F, 0xFE, 0x00, 0x00}, /*"避",2*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x40, 0x20, 0x40, 0x10, 0x40, 0x10, 0x40, 0x00, 0x40, 0x00, 0x40, 0xF0, 0x7C, 0x10, 0x40},
  {0x10, 0x40, 0x10, 0x40, 0x10, 0x40, 0x14, 0x40, 0x18, 0x40, 0x10, 0x40, 0x0F, 0xFE, 0x00, 0x00}, /*"让",3*/
  /* (16 X 16 , 宋体 )*/


  {0x00, 0x40, 0x40, 0x44, 0x23, 0xF4, 0x20, 0x48, 0x00, 0x50, 0x07, 0xFE, 0xE0, 0x40, 0x20, 0x80},
  {0x21, 0xF8, 0x23, 0x08, 0x25, 0x08, 0x21, 0xF8, 0x29, 0x08, 0x31, 0x08, 0x21, 0xF8, 0x01, 0x08}, /*"诸",6*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x00, 0x7F, 0xBE, 0x02, 0x22, 0x22, 0x24, 0x22, 0x24, 0x22, 0x28, 0x42, 0x24, 0x7F, 0xE4},
  {0x06, 0x22, 0x0A, 0x22, 0x12, 0x22, 0x22, 0x34, 0x42, 0x28, 0x82, 0x20, 0x0A, 0x20, 0x04, 0x20}, /*"邪",7*/
  /* (16 X 16 , 宋体 )*/

  {0x03, 0xF8, 0x22, 0x08, 0x12, 0x08, 0x13, 0xF8, 0x02, 0x08, 0x02, 0x08, 0xF3, 0xF8, 0x12, 0x44},
  {0x12, 0x28, 0x12, 0x10, 0x12, 0x88, 0x13, 0x04, 0x12, 0x04, 0x28, 0x00, 0x47, 0xFE, 0x00, 0x00}, /*"退",8*/
  /* (16 X 16 , 宋体 )*/

  {0x24, 0x20, 0x24, 0x20, 0x7E, 0x20, 0x24, 0x3E, 0x24, 0x44, 0xFF, 0x44, 0x00, 0x44, 0x7E, 0xA4},
  {0x42, 0x28, 0x7E, 0x28, 0x42, 0x10, 0x7E, 0x10, 0x42, 0x28, 0x42, 0x28, 0x4A, 0x44, 0x44, 0x82} /*"散",9*/
  /* (16 X 16 , 宋体 )*/

};

const unsigned char table2[][16] = {
  //请(0) 关(1) 远(2) 光(3) ，(4) 安(5) 全(6) 驾(7) 驶(8)

  {0x00, 0x40, 0x40, 0x40, 0x27, 0xFC, 0x20, 0x40, 0x03, 0xF8, 0x00, 0x40, 0xE7, 0xFE, 0x20, 0x00},
  {0x23, 0xF8, 0x22, 0x08, 0x23, 0xF8, 0x22, 0x08, 0x2B, 0xF8, 0x32, 0x08, 0x22, 0x28, 0x02, 0x10}, /*"请",0*/
  /* (16 X 16 , 宋体 )*/

  {0x10, 0x10, 0x08, 0x10, 0x08, 0x20, 0x00, 0x00, 0x3F, 0xF8, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00},
  {0xFF, 0xFE, 0x01, 0x00, 0x02, 0x80, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x30, 0x18, 0xC0, 0x06}, /*"关",1*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x00, 0x23, 0xF8, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x07, 0xFC, 0xF1, 0x20, 0x11, 0x20},
  {0x11, 0x20, 0x11, 0x20, 0x11, 0x24, 0x12, 0x24, 0x12, 0x24, 0x14, 0x1C, 0x28, 0x00, 0x47, 0xFE}, /*"远",2*/
  /* (16 X 16 , 宋体 )*/

  {0x01, 0x00, 0x21, 0x08, 0x11, 0x08, 0x09, 0x10, 0x09, 0x20, 0x01, 0x00, 0xFF, 0xFE, 0x04, 0x40},
  {0x04, 0x40, 0x04, 0x40, 0x04, 0x40, 0x08, 0x42, 0x08, 0x42, 0x10, 0x42, 0x20, 0x3E, 0xC0, 0x00}, /*"光",3*/
  /* (16 X 16 , 宋体 )*/


  {0x02, 0x00, 0x01, 0x00, 0x3F, 0xFC, 0x20, 0x04, 0x42, 0x08, 0x02, 0x00, 0x02, 0x00, 0xFF, 0xFE},
  {0x04, 0x20, 0x08, 0x20, 0x18, 0x40, 0x06, 0x40, 0x01, 0x80, 0x02, 0x60, 0x0C, 0x10, 0x70, 0x08}, /*"安",5*/
  /* (16 X 16 , 宋体 )*/

  {0x01, 0x00, 0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x10, 0x10, 0x2F, 0xE8, 0xC1, 0x06},
  {0x01, 0x00, 0x01, 0x00, 0x1F, 0xF0, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7F, 0xFC, 0x00, 0x00}, /*"全",6*/
  /* (16 X 16 , 宋体 )*/

  {0x08, 0x00, 0x08, 0x00, 0x7F, 0x7C, 0x11, 0x44, 0x11, 0x44, 0x25, 0x7C, 0x42, 0x00, 0x1F, 0xE0},
  {0x00, 0x20, 0x08, 0x20, 0x08, 0x20, 0x0F, 0xFC, 0x00, 0x04, 0x7F, 0xC4, 0x00, 0x14, 0x00, 0x08}, /*"驾",7*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x20, 0xF8, 0x20, 0x08, 0x20, 0x49, 0xFC, 0x49, 0x24, 0x49, 0x24, 0x49, 0x24, 0x7D, 0x24},
  {0x05, 0xFC, 0x04, 0x20, 0x1C, 0xA0, 0xE4, 0x60, 0x44, 0x60, 0x04, 0x90, 0x29, 0x08, 0x12, 0x06}, /*"驶",8*/
  /* (16 X 16 , 宋体 )*/

};

const unsigned char table3[][16] = {

  // 你(0) 已(1) 违(2) 反(3) 深(4) 圳(5) 道(6) 交(7) 处(8) 罚(9) 条(10) 例(11) ，(12) 将(13) 罚(14) 款(15)
  // 五(16) 百(17) 元(18)

  {0x08, 0x80, 0x08, 0x80, 0x08, 0x80, 0x11, 0xFE, 0x11, 0x02, 0x32, 0x04, 0x34, 0x20, 0x50, 0x20},
  {0x91, 0x28, 0x11, 0x24, 0x12, 0x24, 0x12, 0x22, 0x14, 0x22, 0x10, 0x20, 0x10, 0xA0, 0x10, 0x40}, /*"你",0*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x00, 0x3F, 0xF0, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x20, 0x10, 0x20, 0x10, 0x3F, 0xF0},
  {0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x1F, 0xFC, 0x00, 0x00}, /*"已",1*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x80, 0x40, 0x80, 0x2F, 0xFC, 0x20, 0x80, 0x00, 0x80, 0x07, 0xF8, 0xE0, 0x80, 0x20, 0x80},
  {0x2F, 0xFC, 0x20, 0x84, 0x20, 0x84, 0x20, 0x94, 0x20, 0x88, 0x50, 0x80, 0x8F, 0xFE, 0x00, 0x00}, /*"违",2*/
  /* (16 X 16 , 宋体 )*/

  {0x10, 0x04, 0x10, 0x04, 0x17, 0xC4, 0x22, 0x14, 0x22, 0x14, 0x63, 0xD4, 0x62, 0x54, 0xA4, 0x54},
  {0x26, 0x54, 0x25, 0x54, 0x28, 0x94, 0x20, 0x94, 0x21, 0x04, 0x22, 0x04, 0x24, 0x14, 0x28, 0x08}, /*"例",11*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x00, 0x7F, 0xFC, 0x44, 0x44, 0x44, 0x44, 0x7F, 0xFC, 0x00, 0x00, 0x20, 0x08, 0x10, 0x88},
  {0x00, 0x88, 0xF0, 0x88, 0x10, 0x88, 0x10, 0x88, 0x14, 0x88, 0x18, 0x08, 0x10, 0x28, 0x00, 0x10}, /*"罚",14*/
  /* (16 X 16 , 宋体 )*/

  {0x08, 0x20, 0x08, 0x20, 0xFF, 0x20, 0x08, 0x7E, 0x08, 0x42, 0x7E, 0x84, 0x00, 0x10, 0x7E, 0x10},
  {0x00, 0x10, 0xFF, 0x10, 0x08, 0x28, 0x4A, 0x28, 0x49, 0x48, 0x89, 0x44, 0x28, 0x84, 0x11, 0x02}, /*"款",15*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x00, 0x7F, 0xFC, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x3F, 0xF0, 0x04, 0x10},
  {0x04, 0x10, 0x04, 0x10, 0x04, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0xFF, 0xFE, 0x00, 0x00}, /*"五",16*/
  /* (16 X 16 , 宋体 )*/

  {0x00, 0x00, 0xFF, 0xFE, 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x3F, 0xF8, 0x20, 0x08, 0x20, 0x08},
  {0x20, 0x08, 0x3F, 0xF8, 0x20, 0x08, 0x20, 0x08, 0x20, 0x08, 0x20, 0x08, 0x3F, 0xF8, 0x20, 0x08}, /*"百",17*/
  //  /* (16 X 16 , 宋体 )*/
  //

};


void setup() {
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  SPIFFS.begin();

  EEPROM.begin(4096);
  int _tipLight = EEPROM.read(addrTipLight);
  if (_tipLight > 0)tipLight = _tipLight;
  Serial.println(tipLight);
  int _darkLight = EEPROM.read(addrDarkLight);
  if (_darkLight > 0)darkLight = _darkLight;
  Serial.println(darkLight);
  int _warnLight = EEPROM.read(addrWarnLight);
  if (_warnLight > 0)warnLight = _warnLight;
  Serial.println(warnLight);

  EEPROM.commit();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("WifiCar", "3.1415926"); //设立softAP
  Serial.println("Soft ap is start.");
  delay(200);
  //  WiFi.begin("CMCC-5051", "13828748112");//连接网络
  bool requestOn = false;
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(100);
  //  }
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);
  httpUpdater.setup(&WifiServer);
  WifiServer.on("/", handleRoot);
  WifiServer.on("/kaka", handleKaka);
  WifiServer.on("/ledsetting", ledSetting);
  WifiServer.on("/ledset", ledSet);
   WifiServer.on("/showlight", showlight);
  WifiServer.on("/setdisplay", setdisplay);

  WifiServer.onNotFound([]() {
    WifiServer.send(404, "text/html", "File not found");
  });
  WifiServer.begin();
  Serial.println("Wifi Server is started.");
  //  setDNS();
  pixels.begin();
}
int light = 0;


void loop() {

  //
  // put your main code here, to run repeatedly:
  WifiServer.handleClient();
   light = analogRead(A0);
  if (!isDisplayFromCell)return;//如果设置不显示
 
  //    Serial.println(light);
  if (light <= tipLight && light > 0) {
    Serial.println(light);
    pixels.clear();
    draw((int)light);
    //    delay(1000);
    //    draw((int)light);
    //    delay(1000);
    //    //    draw((int)light);
  }
  else if (light > darkLight * 3.91 && light < 1000) {
    pixels.clear();
    draw((int)light);
  }
  else {
    pixels.clear();
    pixels.show();
  }
//  delay(100);

  //  dnsServer.processNextRequest();
}

//
//void setDNS()
//{
//  dnsServer.setTTL(300);
//  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
//  dnsServer.start(DNS_PORT, "wificar", WiFi.localIP());
//}
void handleRoot() {
  WifiServer.send(200, "text/html", "<h1>Hello from <b>ESP8266</b>,pls click <a href=\"/ledsetting\">here</a> to set . </h1><br/><h1>Click <a href=\"/showlight\">here</a> to know the light value<h1/><br/>ver 0.24 ");
}


void draw(int light) {
  int *lines;
  int count = 0;
  int r = 255, g = 255, b = 255;

  for (int k = 0; k < STR_NUM; k++) {
    if (k % 2 == 0) {
      r = random(10, 255);
      g = random(10, 255);
      b = random(10, 255);
    }
    for (int m = 0; m < 16; m++) {
      //对于每个16进制的数组，进行两个数字的组合成16位编码&&
      //        Serial.println("default");
      if (light <= warnLight)lines = getBin(table3[k][m], table3[k][m + 1]);
      else if (light < tipLight && light > warnLight)lines = getBin(table2[k][m], table2[k][m + 1]);
      else if (light < 220 && light >= tipLight)lines = getBin(table[k][m], table[k][m + 1]);
      else if (light >= darkLight && light <= 900)lines = getBin(table[k][m], table[k][m + 1]);
      //      else        lines = getBin(table2[k][m], table2[k][m + 1]);

      if (m % 4 == 2 || m % 4 == 3)//0,1 代表偶数行，2，3代表技术行，奇数行从左到右输出，偶数行从右到左输出
      {
        for (int i = 0; i < 16; i++) {
          if (lines[i] == 1)pixels.setPixelColor(count, r, g, b);
          count++;
        }
      }
      else {
        for (int i = 15; i >= 0 ; i--) {
          if (lines[i] == 1)  pixels.setPixelColor(count,  r, g, b);
          count++;
        }
      }
      m++;
      pixels.show();
    }
    if (k % 2 == 1) {//由于使用了默认字模，所以每个汉字是两个16进制数组，所以在这里进行了处理
      pixels.show();
      delay(500);
      //Serial.println(k);
      pixels.clear();
      pixels.show();
      count = 0;
    }
  }
}

void setdisplay() {
  if ( WifiServer.arg("isdisplay").toInt()) {
    isDisplayFromCell = true;
    WifiServer.send(200, "text/html", "display true." );
  }
  else {
    isDisplayFromCell = false;
    WifiServer.send(200, "text/html", "display cancleed." );
    Serial.println("Cancle display.");

  }
  return;
}

void ledSet() {
  tipLight = WifiServer.arg("tiplight").toInt();
  warnLight = WifiServer.arg("warnlight").toInt();
  darkLight = WifiServer.arg("darklight").toInt();
  int displaytimes = WifiServer.arg("displaytimes").toInt();
  //把相关设置写入EEPROM中
  EEPROM.begin(4096);
  if (tipLight > 0)EEPROM.write(addrTipLight, tipLight);
  if (warnLight > 0)EEPROM.write(addrWarnLight, warnLight);
  if (darkLight > 0)EEPROM.write(addrDarkLight, darkLight);

  EEPROM.commit();

  String str = WifiServer.arg("str");
  int len = str.length();
  int strCnt = len / 64;
  Serial.println(str);
  Serial.println(strCnt);
  WifiServer.send(200, "text/html", "Get your str:\r\n<br/>" );
  int *lines;

  int r = 255, g = 255, b = 255;
  if (len >= 64) {

    //    while (isDisplayFromCell) {
    for (int a = 0; a < displaytimes; a++) {
      for (int m = 0; m < strCnt; m++) {
        int count = 0;
        //      if (m % 2 == 0) {
        r = random(10, 255); g = random(10, 255); b = random(10, 255);
        //      }
        int p = 0;
        for (int n = 0; n <  64; n++) {
          String tmp = "0x00";
          String tmp2 = "0x00";
          int n1 = 0, n2 = 0;
          int k = m * 64 + n;
          tmp[2] = str[k]; tmp[3] = str[k + 1];
          tmp2[2] = str[k + 2]; tmp2[3] = str[k + 3];

          sscanf(tmp.c_str(), "%x", &n1) ;
          sscanf(tmp2.c_str(), "%x", &n2) ;
          //        Serial.print(tmp) ;Serial.print("=") ;Serial.print(n1) ;Serial.print("    ")  ;Serial.print(tmp2);Serial.print("=") ;Serial.println(n2);
          //                    Serial.print(n1);
          //                    Serial.println(n2);
          //        Serial.println(p);
          lines = getBin(n1, n2);
          n = n + 3;
          //          Serial.println(lines);
          if (p % 4 == 2 || p % 4 == 3)//0,1 代表偶数行，2，3代表技术行，奇数行从左到右输出，偶数行从右到左输出
          {
            for (int i = 0; i < 16; i++) {
              if (lines[i] == 1)pixels.setPixelColor(count, r, g, b);
              //            Serial.print(lines[i]);
              count++;
            }
          }
          else {
            for (int i = 15; i >= 0 ; i--) {
              if (lines[i] == 1)  pixels.setPixelColor(count,  r, g, b);
              //             Serial.print(lines[i]);
              count++;
            }
          }
          //        Serial.println("");
          p = p + 2;
          //          delay(2);
          //          pixels.show();
        }
        pixels.show();
        delay(500);
        pixels.clear();
        pixels.show();
        delay(200);
        //      }
        //      delay(5000);
      }
    }
    delay(2000);
  }
  //  strCnt = fmtStr(str);
  //  STR_NUM = strCnt / 2;


}

int* getBin(int Dec, int Dec2 ) {
  //  Dec=128;
  static int a[16];
  for (int i = 7 ; i >= 0 ; i--) {
    if (pow(2, i) <= Dec) {
      Dec = Dec - pow(2, i);
      a[7 - i] = 1;
    } else {
      a[7 - i] = 0;
    }
  }
  for (int i = 7 ; i >= 0 ; i--) {
    if (pow(2, i) <= Dec2) {
      Dec2 = Dec2 - pow(2, i);
      a[15 - i] = 1;
    } else {
      a[15 - i] = 0;
    }
  }
  return a;
}

//
//int fmtStr(String str) {
//  Serial.println(str);
//  int len = str.length();
//  int hexsize = 2;
//  String tmp = "0x00";
//  int n = 0, m = 0;
//  //  Serial.println(len);
//  for (int i = 0; i < len; i++) {
//    tmp[hexsize] = str[i];
//    hexsize++;
//    if (hexsize == 4) {
//      int num = 0;
//
//      sscanf(tmp.c_str(), "%x", &num) ;
//      httpTbl[m][n] = num;
//
//      Serial.print(httpTbl[m][n]);
//      Serial.print(",");
//      hexsize = 2;
//      n++;
//    }
//    if (n == 16) {
//      n = 0;
//      m++;
//    }
//  }
//  return m;
//}
//
//int fmtChar(char* chr) {
//  //  Serial.println(chr);
//  int len = strlen(chr);
//  int hexsize = 2;
//  String tmp = "0x00";
//  int n = 0, m = 0;
//  //  Serial.println(len);
//  for (int i = 0; i < len; i++) {
//    tmp[hexsize] = chr[i];
//    hexsize++;
//    if (hexsize == 4) {
//      int num = 0;
//      sscanf(tmp.c_str(), "%x", &num) ;
//      httpTbl[m][n] = num;
//      //      Serial.print(httpTbl[m][n]);
//      //      Serial.print(",");
//      hexsize = 2;
//      n++;
//    }
//    if (n == 16) {
//      n = 0;
//      m++;
//    }
//  }
//  return m;
//}

void ledSetting() {
  String act = WifiServer.arg("act");
  File file = SPIFFS.open("/getledstr.htm", "r");
  WifiServer.streamFile(file, "text/html");
  file.close();

}
void showlight(){
   WifiServer.send(200, "text/html", "the right light value is :"+(String)light);
}
void handleKaka() {
  String state = WifiServer.arg("led");
  if (state == "on") {
    digitalWrite(led, HIGH);
    WifiServer.send(200, "text/html", "you came from future.<br/> LED is On.");
  }
  else if (state == "off") {
    digitalWrite(led, LOW);
    WifiServer.send(200, "text/html", "you came from future.<br/> LED is Off.");
  }
  else {
    WifiServer.send(200, "text/html", "parameter is wrong.");
  }
}
