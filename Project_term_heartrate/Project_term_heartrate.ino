#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DFRobot_MAX30102.h>

DFRobot_MAX30102 particleSensor;

WiFiClient client;

String thingSpeakAddress = "http://api.thingspeak.com/update?";
String writeAPIKey;
String tsfield1Name;
String request_string;
String api_key = "UHQ5VCJ1F8CKZNIU";
HTTPClient http;

float o;
float A;



// ส่วนของ Wifi
long prev = 0;
void init_wifi()
{
    WiFi.begin("iotgateway","1234567890");
    do
    {
        Serial.print("♥");
        delay(500);
    } while ((!(WiFi.status() == WL_CONNECTED)));
    Serial.println("  WiFi Connected");
    Serial.print("IP address: ");
    Serial.println((WiFi.localIP().toString()));
}
int error;

void setup()
{
    Wire.begin();
    Wire.beginTransmission(0x27);
    error = Wire.endTransmission();
    Serial.print("Error: ");
    Serial.print(error);

    Serial.begin(115200);

    init_wifi();

    while (!particleSensor.begin())
    {
        Serial.println("MAX30102 was not found");
        delay(1000);
    }

    particleSensor.sensorConfiguration(50, SAMPLEAVG_4, MODE_MULTILED, SAMPLERATE_100, PULSEWIDTH_411, ADCRANGE_16384);
}

int32_t SPO2;
int8_t SPO2Valid;
int32_t heartRate;
int8_t heartRateValid;
int i = 0;
int x = 0;
const byte RATE_SIZE = 4;
byte ratesheart[RATE_SIZE];
byte ratesspo2[RATE_SIZE];
byte ratestemp[RATE_SIZE];
void loop()
{
    particleSensor.heartrateAndOxygenSaturation(&SPO2, &SPO2Valid, &heartRate, &heartRateValid);

    if (i < 3)
    {
        ratesheart[i] = heartRate;
        ratesspo2[i] = SPO2;
        i++;
    }
    else
    {
        ratesheart[i - 1] = ratesheart[i];
        ratesspo2[i - 1] = ratesspo2[i];
        i++;
        if (i > 3)
        {
            i = 0;
        }
    }

    //อัตตราการเต้นของหัวใจ
    heartRate = (ratesheart[0] + ratesheart[1] + ratesheart[2] + ratesheart[3]) / 4;
    ratesheart[i] = heartRate;


    //ออกซิเจนในเลือด
    SPO2 = (ratesspo2[0] + ratesspo2[1] + ratesspo2[2] + ratesspo2[3]) / 4;
    ratesspo2[i] = SPO2;


    //ค่าออกซิเจนในเลือด
    if (SPO2 > 100)
    {
        SPO2 = 100;
    }
    if (heartRate > 180)
    {
        heartRate = 180;
    }

    //แสดงค่าทั้งหมด
    Serial.println(F(""));
    Serial.print(F("heartRate="));
    Serial.print(heartRate, DEC);
    Serial.print(F("; SPO2="));
    Serial.print(SPO2, DEC);

    if (millis() - prev >= 1000)
    {
        thingspeak();
        prev = millis();
    }
}


//ส่วนของขึ้นกราฟในเว็ป
void thingspeak()
{
    if (client.connect("api.thingspeak.com", 80))
    {
        request_string = thingSpeakAddress;
        request_string += "key=";
        request_string += api_key;
        request_string += "&field1=";
        request_string += heartRate;
        request_string += "&field2=";
        request_string += SPO2;

        http.begin(client, request_string);
        http.GET();
        http.end();
        request_string = "";
    }
}
