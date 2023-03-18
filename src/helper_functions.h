bool setPowerBoostKeepOn(int en)
{
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
    {
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    }
    else
    {
        Wire.write(0x35); // 0x37 is default reg value
    }
    return Wire.endTransmission() == 0;
}

void sendSms(const int stepCount)
{
    static int lastSmsSentWithStepCount = 0;
    static int noOfTrial = 0;

    // Prevents sending same SMS again
    if (lastSmsSentWithStepCount == stepCount)
        return;

    if (noOfTrial >= MAX_NO_OF_TRIALS_FOR_SMS)
    {
        // SerialMon.println("SMS failed to send after " + String(noOfTrial) + " trials.");
        lastSmsSentWithStepCount = stepCount;
        noOfTrial = 0;
        return;
    }

    char smsMessage[512];
    sprintf(smsMessage, SMS_CONTENT, stepCount);

    if (modem.sendSMS(SMS_TARGET, String(smsMessage)))
    {
        lastSmsSentWithStepCount = stepCount; // Prevents sending same SMS again
                                              // SerialMon.println("SMS successfully sent: " + String(smsMessage));
    }
    else
    {
        noOfTrial++;
        // SerialMon.println("SMS failed to send: " + String(smsMessage));
    }
}

void updateGPS()
{
    // read data from GPS module
    while (SerialGPS.available() > 0)
        gps.encode(SerialGPS.read());
}

void reconnectBlynk()
{
    if (!Blynk.connected())
        Blynk.connect();
}

// Show GPS lat and lng on LCD
void periodicUpdate()
{
    // Send GPS data to Blynk
    if (gps.location.isValid())
        myMap.location(2, gps.location.lat(), gps.location.lng(), MAP_LABEL);
}