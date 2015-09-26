#include "I2Cdev.h"
#include "IO.h"
#include "font.h"
#include "tft.h"
#include "MPU6050.h"
// -------------------
// Version Information
#define NAME "RKY"
#define VERSION_MAJ 0
#define VERSION_MIN 01
#define RELEASE "juin 2015"
//--------------------
// init variable ST7735 : afficheur TFT 1.8" w/SD Card
#define dc      D2 // pin D2
#define cs      D4 // pin D4
#define rst     D3 // Pin D3 for reset
#define sclk    A3
#define sid     A5
//OneWire one = OneWire(D5); // DS18B20 capteur tempé sur D5
Temperature Temp= Temperature(D5);
double celsius=0.0;
char szROM[80];

//ST7735 tft = ST7735(cs, dc, sid, sclk, rst);
ST7735 tft = ST7735(cs, dc, rst);
char message[80];
char result[64]; // cloud variable JSON
unsigned long lastTime=0,currentTime;
// Gestion du Menu
#define MENU_PRINCIPAL 4;
#define MENU_CAPTEURS 5;
#define MENU_REGLAGES 4;
#define MENU_GYRO 4;
char    menuL[][5][30] = {{"Capteurs","Data","Reglages","Sleep"},{"Humidite","Temperature","Luminosite","Record","exit"},{"Status","Infos","Courbes","Exit"},{"Gyro","Accel","Record","exit"}};
short   menuN = MENU_PRINCIPAL; //nb d'options
short   menuS = 1; //selection 1..
short   menuD = 0;
// gestion des boutons
void click(void); // la function d'interruption
void click2(void); // la function d'interruption
ClickButton down = ClickButton();
ClickButton choix = ClickButton();
//gestion du cycle
long signature = 0;
// DHT humidite et Temperature
DHT dht(D6);
double dht_t,dht_h;
// Luminosite  A1  -  Gaz A6
double lumen;
double gaz;
// Accelero MPU5060 // cablé sur port D0-SDA et D1-SCK
MPU6050 Gyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t temp;
float f_ax,f_ay,f_az;
float f_gx,f_gy,f_gz;
float accelBias[3] = { 0.0, 0.0, 0.0 }; // gravity
float gyroBias[3] = {-3.30, 2.29, 1.34 }; // my own !!
bool record = false, reclog = false;
// courbes
byte data[3][128];
byte courbe_s = 0; //index de lecture
//------------------------------------------------------
int WebCde(String Cde);
//-------------
void setup() {
    Serial.begin(9600);
    // --
    Time.zone(+2);
    lastTime = Time.now();

    delay(2000);
    Serial.print("Rky!");
    tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE,ST7735_BLACK);
    tft.setTextWrap(true);
    tft.println("Welcome @HOME. \n\n(c) e-Coucou 2015\n\n\n\nDemarrage du SYSTEME\n\nPatientez ...");

    pinMode(A7, INPUT_PULLUP);    // sets pin as input
    attachInterrupt(A7, click, FALLING);
    pinMode(A0, INPUT_PULLUP);    // sets pin as input
    attachInterrupt(A0, click2, FALLING);

    pinMode(A1, INPUT); // luminosite
    pinMode(A1, INPUT); // gaz

    Wire.begin(); // wire pour initialiser le gyro sur port D0-D1
    Gyro.initialize(); // Init Gyroscope

    Temp.getROM(szROM);

	dht_h = dht.getHumidity();
    lumen = 100.0 - (analogRead(A1) / 40.96);
    dht_t = dht.getTempCelcius();
    
    for(short j=0;j<3;j++) { for(short i=0;i<128;data[j][i++]=140); }
    
    // Spark Variables
    Spark.variable("Humidite",&dht_h,DOUBLE);
    Spark.variable("Temperature_DHT",&dht_t,DOUBLE);
    Spark.variable("Luminosite",&lumen,DOUBLE);
    Spark.variable("Temperature",&celsius,DOUBLE);
    Spark.variable("result", &result, STRING); //cloud JSON
    
    Spark.function("Commande",WebCde);

    delay(5000);
    main_screen();
    affMenu();
}

void loop() {
    currentTime=Time.now();
    if ( (currentTime-lastTime) % 61 == 0 ) {
        lastTime = currentTime-1;
        copyright();
        if (reclog) {
        	dht_h = dht.getHumidity();
    	    lumen = 100.0 - (analogRead(A1) / 40.96);
            celsius = Temp.getTemperature();
            dht_t = dht.getTempCelcius();
            sprintf(result, "{\"temp\":%4.1f,\"humid\":%4.1f,\"lum\":%4.1f}", celsius,dht_h,lumen);
//            Spark.publish("Value",result,60,PUBLIC);
            Spark.publish("Librato_L",String(lumen),60,PRIVATE);
            Spark.publish("Librato_T",String(dht_t),60,PRIVATE);
            Spark.publish("Librato_H",String(dht_h),60,PRIVATE);
            Spark.publish("Librato_Temp",String(celsius),60,PRIVATE);
            data[0][courbe_s] = (byte) (140 - lumen);
            data[1][courbe_s] = (byte) (140 - celsius*3);
//            data[1][courbe_s] = (byte) (140 - dht_t*3);
            courbe_s = (courbe_s + 1) & 0x7F;
            drawCourbe();
        }
    }
    //lecture des boutons ..
    noInterrupts();
    down.Update();
    choix.Update();
    if (Gyro.getIntStatus() && 0x01) {
        getAccelgyro();
        if (record) {
            sprintf(message,"DEL:%f;#ACC:%f;%f;%f",0.0,f_ax,f_ay,f_az);
            Serial.println(message);
        }
//        Spark.publish("Librato_aa",String(f_az),60,PRIVATE);
    }
    interrupts();
    if (down.clicks !=0) signature = (signature & 0xFF01) + 0x10;
    if (choix.clicks !=0) signature = (signature & 0xFF01) + 0x20;
//debug    Serial.println(signature,HEX);
    //...
    switch (signature) {
        case 0 : //menu principal
            tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
            menuD = 0; menuS = 1; menuN=MENU_PRINCIPAL;
            affMenu();
            signature = 0x000E;
            break;
        case 0x3010:
        case 0x2010:
        case 0x1010:
        case 0x0010:
            (++menuS > menuN) ? menuS=1 : 0;
            affMenu();
            signature = (signature & 0xFF00) + 0x0E;
            break;
        case 0x3020:
        case 0x2020:
        case 0x1020:
        case 0x0020:
            signature = (signature & 0xF000)  + (0x100 * menuS);
            break;
        case 0x1021:
            reclog=false;
            aff_record(false);
            signature &= 0xFFFE;
        case 0x0100: // Menu Capteurs
            menuD = 1; menuS = 1; menuN=MENU_CAPTEURS;
            tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
            affMenu();
            signature = 0x100E;
            break;
        case 0x1100:
        	dht_h = dht.getHumidity();
    	    AffRing(dht_h, 1.0, "Humidite");
            signature=0x100F;
            break;
        case 0x1200:
    	    dht_t = dht.getTempCelcius();
            celsius = Temp.getTemperature();
    	    AffRing(celsius, 100.0/30.0, "Temperature");
            signature=0x100F;
            break;
        case 0x1300: // Luminosite
    	    lumen = 100.0 - (analogRead(A1) / 40.96);
    	    AffRing(lumen, 1.0, "Luminosite");
            signature=0x100F;
            break;
        case 0x1400: // Record
            reclog=true;
            test("RECORD");
            aff_record(true);
            signature=0x100F;
            break;
        case 0x3400: 
        case 0x2400:
        case 0x1500: //EXIT MENU Level 1
            signature = 0x0;
            break;
        case 0x3021:
            record=false;
            aff_record(false);
            signature &= 0xFFFE;
        case 0x0200: // Menu Data
            menuD = 3; menuS = 1; menuN=MENU_GYRO;
            tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
            affMenu();
            signature = 0x300E;
            break;
        case 0x2021:
            signature &= 0xFFFE;
        case 0x0300: // Menu Reglages
            menuD = 2; menuS = 1; menuN=MENU_REGLAGES;
            tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
            affMenu();
            signature = 0x200E;
            break;
        case 0x2100: // Menu Status
            tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
            tft.setTextColor(ST7735_WHITE,ST7735_BLACK);
            tft.setCursor(0,40);
            tft.setTextSize(1);
            tft.println("Status");
            tft.println("------\n");
            tft.println("Temperature...... v");
            tft.println("Humidite......... v");
            tft.println("Luminosite....... v");
            tft.println("Web variables.... v");
            tft.println("WebHook.......... v");
//            tft.setTextColor(ST7735_WHITE,0x0028);
            tft.println("Acceleration..... -");
            tft.println("Gyroscope........ -");
            tft.setTextColor(ST7735_WHITE,ST7735_RED);
            tft.println("LED.............. i");
            signature = 0x200F;
            break;
        case 0x2200:
            tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
            tft.setCursor(0,40);
            tft.setTextSize(1);
            tft.setTextColor(ST7735_WHITE,ST7735_BLACK);
            tft.print("IP     : ");tft.println(WiFi.localIP());
            tft.print("Signal : ");tft.print(WiFi.RSSI());tft.println(" dB");
            tft.print("SSID   : ");tft.println(WiFi.SSID());
            tft.print("Gyro   : ");tft.print(Gyro.getDeviceID(),HEX);tft.println( (Gyro.getDeviceID()==0x34) ? " ok" : " NOK");
            signature = 0x200F;
            break;
        case 0x2300:
            drawCourbe();
            signature = 0x200F;
            break;
        case 0x3100:
            sprintf(message,"Gx=%5.2f\nGy=%5.2f\nGz=%5.2f",f_gx,f_gy,f_gz);
            Aff_Message(message,0,55,ST7735_WHITE,ST7735_BLACK,2);
            signature=0x300F;
            break;
        case 0x3200:
            sprintf(message,"Ax=%5.2f\nAy=%5.2f\nAz=%5.2f",f_ax,f_ay,f_az);
            Aff_Message(message,0,55,ST7735_WHITE,ST7735_BLACK,2);
            signature=0x300F;
            break;
        case 0x3300:
            record=true;
            test("RECORD");
            aff_record(true);
            signature=0x300F;
            break;
        case 0xFF20:
            tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
            signature = 0;
        case 0x0400:
            signature = 0;
        case 0x300E:
        case 0x200E:
        case 0x100E:
        case 0x000E: // wait ...
            break;
        default :
            break;
    }

}
/* ------------------------------------------
   Commande Web
   ------------------------------------------  */
int WebCde(String Cde) {
    if (Cde == "Gyro")  signature = 0x3100;
    if (Cde == "Acc")   signature = 0x3200;
    if (Cde == "Rec")   signature = 0x3300;
    if (Cde == "NoRec") signature = 0x3021;
    if (Cde == "Log")   signature = 0x1400;
    if (Cde == "NoLog") signature = 0x1021;
    if (Cde == "Hum")   signature = 0x1100;
    if (Cde == "Temp")  signature = 0x1200;
    if (Cde == "Lum")   signature = 0x1300;
    if (Cde == "Home")  signature = 0x0;
    if (Cde == "Info")  signature = 0x2200;
    if (Cde == "Status") signature = 0x2100;
    if (Cde == "Courbes") signature = 0x2300;
    return signature;
}
/* ------------------------------------------
   Routines Interruptions
   ------------------------------------------  */
void click()
{
    long now = (long)millis();
    if ( now - down.lastBounceTime > down.debounceTime ) {
        down.pressed = true;
        down.lastBounceTime = now;
    }
}
void click2()
{
    long now = (long)millis();
    if ( now - choix.lastBounceTime > choix.debounceTime ) {
        choix.pressed = true;
        choix.lastBounceTime = now;
    }
}
/* ------------------------------------------
   Routines Générales
   ------------------------------------------  */
void main_screen() {
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_YELLOW);
    tft.setCursor(0,0);
    tft.setTextSize(3);
    tft.println(NAME);
    tft.setTextSize(1);
    tft.print("by eric");
    // draw line
    tft.drawLine(0,38 , tft.width(), 38, ST7735_YELLOW);
    copyright();
}
void copyright() {
    // copyright
    tft.setTextSize(1);
    tft.setTextColor(0xEEEE,ST7735_BLACK);
    tft.setCursor(0, 151);
    sprintf(message,"(c) Rky %d.%d - %02d:%02d",VERSION_MAJ,VERSION_MIN,Time.hour(),Time.minute());
    tft.print(message);
}
void affMenu() {
    for (short i=0; i<menuN;printMenu(i++));
    copyright();
}
void printMenu(short n) {
    tft.fillRect(1,(n*15)+40,tft.width()-2,13,(n==(menuS-1)) ? ST7735_BLUE : 0x5555);
    tft.setCursor(3,(n*15)+42);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE);
    tft.print(menuL[menuD][n]);
}

// Affichage d'un Message
void Aff_Message(char *mess ,int16_t x, int16_t y, uint16_t color, uint16_t bkcolor, short size) {
    tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
    tft.setTextColor(color,bkcolor);
    tft.setCursor(x,y);
    tft.setTextSize(size);
    tft.println(mess);
}

void getAccelgyro() {
    // read raw accel/gyro measurements from device
    Gyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    f_ax = ax *2.0 /32768.0 ;//- accelBias[0];// + 1.0; //add gravity ?
    f_ay = ay *2.0 /32768.0 ;//- accelBias[1];
    f_az = az *2.0 /32768.0 ;//- accelBias[2];
    f_gx = gx *250.0 /32768.0 - gyroBias[0];
    f_gy = gy *250.0 /32768.0 - gyroBias[1];
    f_gz = gz *250.0 /32768.0 - gyroBias[2];
/*    temp = accelgyro.getTemperature();
    celsius_gyro = temp / 340.0 + 36.53;
*/
}

void aff_record(bool enable) {
    if (enable) {
        tft.fillCircle(tft.width()-25,17,10,ST7735_WHITE);
        tft.fillCircle(tft.width()-25,17,8,0xF800);
    } else
        tft.fillCircle(tft.width()-25,17,11,ST7735_BLACK);
        
}

void AffRing(float val, float coef, char *mesure) {
    tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
    tft.setCursor(40,82);
    tft.setTextSize(2);
    tft.setTextColor(ST7735_WHITE,ST7735_BLACK);
    sprintf(message,"%4.1f",val);
    tft.print(message);
    val *= coef;
    tft.drawTor(64,90,35,49, 100 ,ST7735_BLUE); // 
    tft.drawTor(64,90,35,49, val ,ST7735_YELLOW); // 
    tft.setCursor(24,135);
    tft.setTextSize(1);
    tft.print(mesure);
}

void drawCourbe(){
    tft.fillRect(0,40,tft.width(),110,ST7735_BLACK);
    tft.drawFastVLine(0,40,100,ST7735_GREEN);
    tft.drawFastHLine(0,140,tft.width(),ST7735_GREEN);
    for (byte x=40;x<141;x=x+10) {
        tft.drawPixel(1,x,ST7735_GREEN);
    }
    for(byte x=0;x<128;x++)
    {
        tft.drawPixel(x,data[0][(x+courbe_s) % 128],ST7735_WHITE);
        tft.drawPixel(x,data[1][(x+courbe_s) % 128],ST7735_CYAN);
    }
}

void test(char text[40]) {
    tft.setTextColor(ST7735_RED);
    tft.setCursor(0,50);
    tft.setTextSize(3);
    tft.print(text);
}
