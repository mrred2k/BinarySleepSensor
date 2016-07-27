// EgSlimReed2
// By m26872, 2015-12-22
// by mrRed, 2016-07-27
// Interrupt driven binary switch for Slim Node with Reed switch and external pull-up (10Mohm)
// Inspired by mysensors example:
// https://github.com/mysensors/Arduino/blob/master/libraries/MySensors/examples/BinarySwitchSleepSensor/BinarySwitchSleepSensor.ino

//////////TODO:////////
//Battery sensor als define
//////////////////////


// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#include <SPI.h>
#include <MySensors.h>
#include <Vcc.h>

#define BATT_SENSOR    199

//#define NODE_ID 5 //12 var senaste "reed-node"-id // 110    // Use static Node_ID  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#define SKETCH_NAME "EgSlimReed"
#define SKETCH_MAJOR_VER "2.1 2016-27-07"
#define SKETCH_MINOR_VER "0"

#define PRIMARY_CHILD_ID 3
#define SECONDARY_CHILD_ID 4

#define SW_CHILD_ID 5 //necessary
#define SW_PIN 3


#define BATTERY_REPORT_DAY 1   // Desired heartbeat interval when inactive. Maximum heartbeat/report interval is equal to this due to the dayCounter.
#define BATTERY_REPORT_BY_IRT_CYCLE 5  //Count after how many switch trigger a battery report should be done.
#define ONE_DAY_SLEEP_TIME 86400000
#define VCC_MIN 1.9
#define VCC_MAX 3.0
const float VccCorrection = 3.15/3.06;  // Measured Vcc by multimeter divided by reported Vcc


#ifdef BATT_SENSOR
MyMessage msgBatt(BATT_SENSOR, V_VOLTAGE);
#endif



int dayCounter = 0;
int irtCounter = 0;
uint8_t value;
uint8_t sentValue = 2;
bool interruptReturn = false;
int oldBatteryPcnt = 0;

Vcc vcc(VccCorrection);

MyMessage msg(SW_CHILD_ID, V_TRIPPED);

void setup()
{
  delay(100); // to settle power for radio
  pinMode(SW_PIN, INPUT);
  digitalWrite(SW_PIN, HIGH);

}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);

  // Register binary input sensor to sensor_node (they will be created as child devices)
  // You can use S_DOOR, S_MOTION or S_LIGHT here depending on your usage.
  // If S_LIGHT is used, remember to update variable type you send in. See "msg" above.
  present(PRIMARY_CHILD_ID, S_DOOR);
}

void loop()
{
  uint8_t value;
  static uint8_t sentValue = 2;

  if (!interruptReturn) { // Woke up by timer (or first run)
    dayCounter++;
    if (dayCounter >= BATTERY_REPORT_DAY) {
      dayCounter = 0;
      sendBatteryReport();
    }
  }
  else {    // Woke up by pin change
    irtCounter++;
    sleep(50);       // Short delay to allow switch to properly settle
    value = digitalRead(SW_PIN);
    Serial.println(irtCounter);
    if (value != sentValue) {
      send(msg.set(value == HIGH ? 1 : 0));
      sentValue = value;
    }
    if (irtCounter >= BATTERY_REPORT_BY_IRT_CYCLE) {
      irtCounter = 0;
      sendBatteryReport();
      
    }
  }

  // Sleep until something happens with the sensor,   or one sleep_time has passed since last awake.
  interruptReturn = sleep(SW_PIN - 2, CHANGE, ONE_DAY_SLEEP_TIME);

}

void sendBatteryReport() {
  float p = vcc.Read_Perc(VCC_MIN, VCC_MAX, true);
  int batteryPcnt = static_cast<int>(p);
    #ifdef BATT_SENSOR
        float v = vcc.Read_Volts(); 
        send(msgBatt.set(v,2));
    #endif
  if (oldBatteryPcnt != batteryPcnt) {
    sendBatteryLevel(batteryPcnt);
    oldBatteryPcnt = batteryPcnt;


#ifdef MY_DEBUG
    Serial.print("Battery: ");
    Serial.print(batteryPcnt);
    Serial.print("% ");
        Serial.print(v);
    Serial.println(" V");
    #endif
  }


}
