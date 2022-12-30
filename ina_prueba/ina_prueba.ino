#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLNXjDP_bx"
#define BLYNK_DEVICE_NAME "multimetro"


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include <SD.h>
#include <BlynkSimpleEsp32.h>

/****************************************************************************/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_INA219 ina219;
RTC_DS3231 rtc;
TimeSpan step = TimeSpan(60);
File file;
DateTime now, later;
String data;
BlynkTimer timer;

char auth[] = "_zh-V47ti_h3IBFWfak1fOZKX3c8EIlo";
char ssid[] = "CARBAJALJORDAN";
char pass[] = "77wilbermariel77";



unsigned long previousMillis = 0;
unsigned long interval = 2000;
float shuntvoltage = 0.00;
float busvoltage = 0.00;
float current = 0.00;
float loadvoltage = 0.00;
//float energy = 0.00,  energyCost, energyPrevious, energyDifference;
float power = 0.00;
float capacity = 0.00;
static const uint8_t image_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 
    0x00, 0x80, 0x78, 0x00, 0x10, 0x00, 
    0x01, 0x00, 0xfc, 0x00, 0x08, 0x00, 
    0x01, 0x00, 0xfc, 0x00, 0x08, 0x00, 
    0x03, 0x00, 0xfe, 0x00, 0x0c, 0x00, 
    0x06, 0x00, 0xff, 0x00, 0x04, 0x00, 
    0x06, 0x00, 0xff, 0x80, 0x06, 0x00, 
    0x06, 0x00, 0xff, 0xc0, 0x06, 0x00, 
    0x0e, 0x00, 0xff, 0xc0, 0x07, 0x00, 
    0x0e, 0x00, 0xff, 0xe0, 0x07, 0x00, 
    0x0e, 0x00, 0x7f, 0xe0, 0x07, 0x00, 
    0x0e, 0x00, 0x7f, 0xf0, 0x07, 0x00, 
    0x1e, 0x00, 0x3f, 0xf0, 0x07, 0x00, 
    0x1e, 0x00, 0x3f, 0xf0, 0x07, 0x80, 
    0x1f, 0x00, 0x1f, 0xf0, 0x0f, 0x80, 
    0x1f, 0x00, 0x0f, 0xf0, 0x0f, 0x80, 
    0x1f, 0x00, 0x07, 0xf0, 0x0f, 0x80, 
    0x1f, 0x80, 0x03, 0xf0, 0x1f, 0x00, 
    0x0f, 0x80, 0x03, 0xf0, 0x1f, 0x00, 
    0x0f, 0xc0, 0x01, 0xe0, 0x3f, 0x00, 
    0x0f, 0xe0, 0x01, 0xe0, 0x7f, 0x00, 
    0x0f, 0xf0, 0x01, 0xc0, 0xff, 0x00, 
    0x07, 0xf8, 0x01, 0xc1, 0xfe, 0x00, 
    0x07, 0xfc, 0x01, 0x83, 0xfe, 0x00, 
    0x07, 0xff, 0x01, 0x0f, 0xfc, 0x00, 
    0x03, 0xff, 0xe2, 0x7f, 0xfc, 0x00, 
    0x01, 0xff, 0xff, 0xff, 0xf8, 0x00, 
    0x01, 0xff, 0xff, 0xff, 0xf8, 0x00, 
    0x00, 0xff, 0xff, 0xff, 0xf0, 0x00, 
    0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 
    0x00, 0x3f, 0xff, 0xff, 0xc0, 0x00, 
    0x00, 0x1f, 0xff, 0xff, 0x80, 0x00, 
    0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 
    0x00, 0x03, 0xff, 0xfc, 0x00, 0x00, 
    0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
void send_sensor() {
  now = rtc.now();
  if(later == now) {
    later = now + step;
    ina219values();
    Serial.println("Hora de grabar......");   
    //loadvoltage,current,power,capacity     
    if(isnan(loadvoltage) || isnan(current)||isnan(power) || isnan(capacity)) {
      Serial.println("Error de lectura");
      return;
    }
    file = SD.open("/Ejercicio1.csv", FILE_APPEND);
    if (file) {
      Serial.println("imprimiendo....");     
      data = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + "," + String(now.hour()) + ":" + 
      String(now.minute()) + ":" + String(now.second()) + " , " + String(loadvoltage) + "V   , " + String(current)+ "mA  , " + String(power) + "mW  ,  " + String(capacity)+"mAh";
      file.println(data);
      file.close();
    } else {
      Serial.println("Error al abrir el archivo");
    }
    Blynk.virtualWrite(V0, loadvoltage);
    
    Blynk.virtualWrite(V1, current);
    Blynk.virtualWrite(V2, power/1000);
    Blynk.virtualWrite(V4, capacity);
    Blynk.virtualWrite(V7, current);    
    
   // displaydata(); 
  }
}  

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    // will pause Zero, Leonardo, etc until serial console opens
    delay(1);
  }

  uint32_t currentFrequency;

  Serial.begin(9600);
  // initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Multimetro");
  display.drawBitmap(40,20, image_data,44, 45, 1);
  display.display(); 


  // Inicia INA219.

  if (! ina219.begin()) {
    Serial.println("Error encontrando INA219 chip");
    while (1) {
      delay(10);
    }
  }
  if (!rtc.begin()) {
      Serial.println("Error en el reloj");
      while (1);
  }
  if (!SD.begin()) {
      Serial.println("Error en la sd");
      while(1);
    }
  if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  if (!SD.exists("/Ejercicio1.csv")) {
    file = SD.open("/Ejercicio1.csv", FILE_WRITE);
    if (file) {
      file.println("Fecha,Hora,Humedad,Temperatura");
      file.close();
    } else {
      Serial.println("Error al crear el archivo.");
    }
    
  }
  now = rtc.now();
  later = rtc.now() + TimeSpan(60);


  Serial.println("Multimetro con INA219 ...");
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(10, send_sensor);
}


void loop()
{

  Blynk.run();
  timer.run();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    ina219values();
    displaydata();
  }
  send_sensor();
}
void ina219values()  {

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  power = loadvoltage * current;
  capacity = capacity + current / 1000;

  if (loadvoltage < 1 )loadvoltage = 0;
  if (current < 1 )
  {
    current = 0;
    power = 0;
    capacity = 0;
  }

  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power); Serial.println(" mW");
  Serial.print("Capacity:      "); Serial.print(capacity); Serial.println(" mAh");

  Serial.println("-------------------------");
}
void displaydata() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  //variables de salida loadvoltage,current,power,capacity
  // VOLTAJE
  display.setCursor(0, 5);
  display.print(loadvoltage, 2);
  display.print(" V");
  // CORRIENTE
  if (current > 1000) {
    display.setCursor(60, 5);
    display.print((current / 1000), 2);
    display.println(" A");
    display.setCursor(0, 15);
    display.println("--------------------");
  }
  else
  {
    display.setCursor(60, 5);
    display.print(current, 1);
    display.println(" mA");
    display.setCursor(0, 15);
    display.println("--------------------");
  }

//Impresion  para separacion
  display.setCursor(60, 20);
  display.print("|");
  display.setCursor(60, 24);
  display.print("|");
  display.setCursor(60, 28);
  display.print("|");
  display.setCursor(60, 32);
  display.print("|");
  display.setCursor(60, 36);
  display.print("|");
  display.setCursor(60, 40);
  display.print("|");
  display.setCursor(0, 46);
  display.print("--------------------");

  // Potencia
  if (power > 1000) {
    display.setCursor(0, 24);
    display.print("Consumo");    
    display.setCursor(0, 36);
    display.print(String((power / 1000), 2));
    display.println(" W");
  }
  else {
    display.setCursor(0, 24);
    display.print("Consumo: ");   
    display.setCursor(0, 36);
    display.print(power, 2);
    display.println(" mW");
  }

  // Capacidad
  if (capacity > 1000) {
    display.setCursor(65, 24);
    display.println("Capacidad: ");
    display.setCursor(65, 36);
    display.print((capacity / 1000), 2);
    display.println(" Ah"); 
  }
  else
  {
    display.setCursor(65, 24);
    display.println("Capacidad: ");
    display.setCursor(65, 36);
    display.print(capacity, 2);
    display.println("mAh");
  }
  display.display();
}