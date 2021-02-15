#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h> 
#include <WaziDev.h>
#include <xlpp.h>

// LoRaWANKey is used as both NwkSKey (Network Session Key) and Appkey (AppKey) for secure LoRaWAN transmission.
// Copy'n'paste the key to your Wazigate: 23158D3BBC31E6AF670D195B5AED5525
unsigned char LoRaWANKey[16] = {0x23, 0x15, 0x8D, 0x3B, 0xBC, 0x31, 0xE6, 0xAF, 0x67, 0x0D, 0x19, 0x5B, 0x5A, 0xED, 0x55, 0x25};
// Copy'n'paste the DevAddr (Device Address): 26011D87
unsigned char DevAddr[4] = {0x26, 0x01, 0x1D, 0x88};
// You can change the Key and DevAddr as you want.

WaziDev wazidev;

// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

#define LSM9DS1_SCK A5
#define LSM9DS1_MISO 12
#define LSM9DS1_MOSI A4
#define LSM9DS1_XGCS 6
#define LSM9DS1_MCS 5
// You can also use software SPI
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_SCK, LSM9DS1_MISO, LSM9DS1_MOSI, LSM9DS1_XGCS, LSM9DS1_MCS);
// Or hardware SPI! In this case, only CS pins are passed in
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_XGCS, LSM9DS1_MCS);

float taraX, taraY, taraZ = 0;

const int pin_red=11;
const int pin_green=10;

int counter = 0;

void setupSensor()
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}


void setup() 
{
  Serial.begin(115200);

  // wazidev.setupLoRaWAN(DevAddr, LoRaWANKey);

  pinMode(pin_red,OUTPUT);
  pinMode(pin_green,OUTPUT);

  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }
  
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    while (1);
  }
  Serial.println("Found LSM9DS1 9DOF");

  // helper to just set the default scaling we want, see above!
  setupSensor();

  delay(1000);

  Serial.print("TARA ");

  for(int i = 0; i< 100; i++) {
    lsm.read();
    
    sensors_event_t a, m, g, temp;

    lsm.getEvent(&a, &m, &g, &temp);
    
    taraX = taraX + a.acceleration.x;
    taraY = taraY + a.acceleration.y;
    taraZ = taraZ + a.acceleration.z;

    delay(10);

    if(i%10 == 0) {
      Serial.print("====");
    }
  }
  Serial.println(" DONE");

  taraX = taraX / 100;
  taraY = taraY / 100;
  taraZ = taraZ / 100;

  Serial.print("tara: ");
  Serial.print("Accel X: "); Serial.print(taraX); Serial.print(" m/s^2");
  Serial.print("\tY: "); Serial.print(taraY);     Serial.print(" m/s^2 ");
  Serial.print("\tZ: "); Serial.print(taraZ);     Serial.println(" m/s^2 ");

  Serial.println("=================================================");
  for(int i = 0; i < 10; i++){
    set_outputs_plus_ground(pin_red,pin_green);
    delay(50);
    set_outputs_plus_ground(pin_green,pin_red);   
    delay(50);
  }
  Serial.println("reading....");
  set_outputs_plus_ground(pin_red,pin_green);
  delay(1000);
}

XLPP xlpp(120);

void loop() 
{
   counter ++;

  if(counter > 100){
    set_outputs_plus_ground(pin_green,pin_red);  
  }
  float meanX = 0;
  float meanY = 0;
  float meanZ = 0;

  for(int j = 0; j < 5; j++) {
    lsm.read();
    sensors_event_t a, m, g, temp;
    lsm.getEvent(&a, &m, &g, &temp);
    meanX += a.acceleration.x - taraX;
    meanY += a.acceleration.y - taraY;
    meanZ += a.acceleration.z - taraZ;   
  
  
    delay(20);
  }

    float finX = meanX / 5;
    float finY = meanY / 5;
    float finZ = meanZ / 5;
    
    Serial.print("Accel X: "); Serial.print(finX); Serial.print(" m/s^2");
    Serial.print("\tY: "); Serial.print(finY);     Serial.print(" m/s^2 ");
    Serial.print("\tZ: "); Serial.print(finZ);     Serial.println(" m/s^2 ");

    // xlpp.reset();
    // xlpp.addAccelerometer(1, finX, finY, finZ);
    
    // wazidev.sendLoRaWAN(xlpp.buf, xlpp.len);

    

    
    delay(100);  
}

void set_outputs_plus_ground(int pin_plus, int pin_ground)
{
  //set both pins low first so you don't have them pushing 5 volts into eachother
  digitalWrite(pin_plus,LOW);  
  digitalWrite(pin_ground,LOW);
  //set output pin as high
  digitalWrite(pin_plus,HIGH);  
  } 
