#include "Arduino_BMI270_BMM150.h"
#include <ArduinoBLE.h>

struct Packet {
  uint32_t time;
  float ax, ay, az; // acceleration data
  float gx, gy, gz; // gyoscope data
};

BLEService imuService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic accelCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, sizeof(Packet));

float ax, ay, az;
float gx, gy, gz;



void setup() {
  Serial.begin(9600);
  while (!Serial); // waits

  if (!IMU.begin()){ // from arduino doc, ensures IMU initializes
    Serial.println("Failued to initialize IMU");
    while (1);
  }

  if (!BLE.begin()){
    Serial.println("starting BLE failed!");
    while (1);
  }



  // some data
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");

  BLE.setLocalName("Nano33_IMU");
  BLE.setAdvertisedService(imuService);
  imuService.addCharacteristic(accelCharacteristic);
  BLE.addService(imuService);
  BLE.advertise();

  Serial.println("BLE IMU device active, waiting for connections");
  

}

void loop() {
  BLEDevice central = BLE.central();
   // collect time in ms


  if (central) {

    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
     
      Packet packet; // initializes packet
      packet.time = millis(); // will be used to check time between packets

      IMU.readGyroscope(packet.gx, packet.gy, packet.gz);
      IMU.readAcceleration(packet.ax, packet.ay, packet.az); // updates packet values

      accelCharacteristic.writeValue((void*)&packet, sizeof(packet));

    }
  }
}
