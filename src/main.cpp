/*
    main.cpp
    Date: 18 March 2023
    Author: Berk Yaşar Yavuz
    Mail: berkyavuz1997@gmail.com
*/

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/****************************      ! TO THE ATTENTION OF THE USER !     ****************************/
/***********     You must fill the following fields with your data to use this code:    ************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
const char blynkAuth[] = "YourAuthToken";

// Blynk Virtual Pins
#define BLYNK_MAP_VIRTUAL_PIN V0
#define BLYNK_STEP_COUNTER_VIRTUAL_PIN V1

// Your GPRS credentials, if any
const char apn[] = "YourAPN";
const char user[] = "";
const char pass[] = "";
const char simPIN[] = ""; // SIM card PIN (leave empty, if not defined)

// Your phone number to send SMS: + (plus sign) and country code
// For example, for Türkiye +90, followed by phone number.
// SMS_TARGET Example for Türkiye  +905522141197 (My phone number, if you want to send me a message ;)
// SMS_TARGET Example for Portugal +351XXXXXXXXX
#define SMS_TARGET "+905522141197"

// For example if 5 and 100 are selected respectively, then SMS is send for step 5, 105, 205, 305, etc.
#define SEND_FIRST_SMS_AFTER_N_STEPS 5 // Send first SMS after N steps.
#define SEND_SMS_EVERY_N_STEPS 100     // Sends SMS every N steps.

#define MAX_NO_OF_TRIALS_FOR_SMS 5 // Maximum number of trials to send SMS if it fails.

// SMS content. You can use %d to insert step count.
#define SMS_CONTENT "Patient took %d steps. You might want to check on him/her."

#define MAP_LABEL "Patient Location"

// Force Sensitive Resistor Pin (attached to pin 34 as indicated in the schematic)
#define FSR_PIN 34

// TTGO T-Call pins (Most probably you don't need to change these)
#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define I2C_SDA 21
#define I2C_SCL 22

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/****************************      ! TO THE ATTENTION OF THE USER !     ****************************/
/*************     You must fill the above fields with your data to use this code:    **************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

// Comment this out to disable prints from Blynk and save space
// #define BLYNK_PRINT Serial

// Set serial for GPS communication (to NEO6M module)
#define SerialGPS Serial

// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

// Default heartbeat interval for GSM is 60
// If you want override this value, uncomment and set this option:
// #define BLYNK_HEARTBEAT 30

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800   // Modem is SIM800
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb

// Include GSM and Blynk libraries
#include <Wire.h>
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>

// Include GPS parsing libary
#include <TinyGPS++.h>

// Include our private libraries
#include <ForceSensitiveResistor.h>

// TinyGSM Modem object
TinyGsm modem(SerialAT);

// TinyGPS++ object
TinyGPSPlus gps;

// Force Sensitive Resistor object
ForceSensitiveResistor fsr(FSR_PIN);

// Blynk objects and widgets
WidgetMap myMap(BLYNK_MAP_VIRTUAL_PIN);
SimpleTimer timer;

#define IP5306_ADDR 0x75
#define IP5306_REG_SYS_CTL0 0x00

#include "helper_functions.h"

void setup()
{
  SerialGPS.begin(115200);
  delay(10);

  // Keep power when running from battery
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);

  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);

  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  modem.restart();
  // modem.init(); // Restart takes quite some time. To skip it, you may call init() instead of restart()

  // Unlock your SIM card with a PIN if needed
  if (strlen(simPIN) && modem.getSimStatus() != 3)
    modem.simUnlock(simPIN);

  String modemInfo = modem.getModemInfo();

  Blynk.begin(blynkAuth, modem, apn, user, pass);

  timer.setInterval(1000L, periodicUpdate);
  timer.setInterval(60 * 1000, reconnectBlynk);
}

void loop()
{
  timer.run();
  fsr.run();
  if (Blynk.connected())
    Blynk.run();

  updateGPS();

  static unsigned int previousStepsTaken = 0;
  const unsigned int stepsTaken = fsr.getSteps();

  // Send sms
  if (stepsTaken % SEND_SMS_EVERY_N_STEPS == SEND_FIRST_SMS_AFTER_N_STEPS)
    sendSms(stepsTaken);

  // Send step data to Blynk
  if (previousStepsTaken != stepsTaken)
    Blynk.virtualWrite(BLYNK_STEP_COUNTER_VIRTUAL_PIN, stepsTaken);

  previousStepsTaken = stepsTaken;
}

/*
    main.cpp
    Date: 18 March 2023
    Author: Berk Yaşar Yavuz
    Mail: berkyavuz1997@gmail.com
*/