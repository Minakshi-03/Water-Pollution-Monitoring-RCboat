#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TinyGPS++.h>
#include <HTTPClient.h>

#define ONE_WIRE_BUS 4   // DS18B20 temperature sensor pin
#define PH_PIN 35        // pH sensor pin
#define TURBIDITY_PIN 32 // Turbidity sensor pin

#define ENA 23  // Motor Speed Control (PWM)
#define IN1 18  // Motor Direction 1
#define IN2 19  // Motor Direction 2

#define VOLTAGE_REF 3.3  // ESP32 ADC reference voltage
#define ADC_RESOLUTION 4096 // ESP32 ADC resolution (12-bit)

#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

float neutralVoltage = 1.65;
float acidVoltage = 2.8;
float baseVoltage = 0.5;

const char* ssid = "Device Name";
const char* password = "Password";
const char* apiKey = "AIzaSyCE4YEyNGird5CArVQP97dMj2G2CpN_eo4"; // Secure your API key!

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WebServer server(80);

String latitude = "N/A";
String longitude = "N/A";
bool motorRunning = false;

void motorForward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 150);
    motorRunning = true;
}

void motorStop() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    motorRunning = false;
}

float readTemperature() {
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}

float readPH() {
    int analogValue = analogRead(PH_PIN);
    float voltage = (float)analogValue / ADC_RESOLUTION * VOLTAGE_REF;
    return (voltage >= neutralVoltage) ? (7 + (voltage - neutralVoltage) * (3.0 / (acidVoltage - neutralVoltage))) : (7 - (neutralVoltage - voltage) * (3.0 / (neutralVoltage - baseVoltage)));
}

int readTurbidity() {
    return analogRead(TURBIDITY_PIN);
}

String readGPSData() {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
    return gps.location.isValid() ? "Lat: " + String(gps.location.lat(), 6) + "<br>Lon: " + String(gps.location.lng(), 6) : "Waiting for GPS signal...";
}

void getLocation() {
    WiFi.scanNetworks(true, false);
    delay(5000);
    HTTPClient http;
    String url = "https://www.googleapis.com/geolocation/v1/geolocate?key=" + String(apiKey);
    String jsonPayload = "{\"wifiAccessPoints\":[]}";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonPayload);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response: " + response);  // Debugging

        int latStart = response.indexOf("\"lat\":") + 6;
        int latEnd = response.indexOf(",", latStart);
        int lonStart = response.indexOf("\"lng\":") + 6;
        int lonEnd = response.indexOf("\n", lonStart);

        if (latStart > 5 && lonStart > 5) {
            latitude = response.substring(latStart, latEnd);
            longitude = response.substring(lonStart, lonEnd);
        } else {
            Serial.println("Failed to parse location data.");
            latitude = "N/A";
            longitude = "N/A";
        }
    } else {
        Serial.println("Error getting location.");
        latitude = "N/A";
        longitude = "N/A";
    }

    http.end();
}

void handleRoot() {
    float temperature = readTemperature();
    float pH = readPH();
    int turbidity = readTurbidity();
    String gpsData = readGPSData();

    String html = "<html><head><meta http-equiv='refresh' content='5'><title>Water Monitoring</title>";
    html += "<style>body{font-family:Arial;text-align:center;background:#0072ff;color:white;padding:20px;} ";
    html += ".container{max-width:600px;margin:auto;background:#222;padding:20px;border-radius:10px;box-shadow:0 4px 8px rgba(255,255,255,0.2);} ";
    html += ".box{border:2px solid white;padding:15px;margin:10px 0;border-radius:5px;background:rgba(255,255,255,0.1);} .alert{color:yellow;font-weight:bold;}";
    html += "button{padding:10px 20px;font-size:16px;border:none;background:#ff9800;color:white;cursor:pointer;border-radius:5px;margin:5px;} ";
    html += "button:hover{background:#e68900;}</style></head><body>";

    html += "<div class='container'><h1>Water Quality Monitoring</h1>";
    
    html += "<div class='box'><h2>Temperature</h2><p>" + String(temperature, 2) + "°C";
    if (temperature < 5 || temperature > 40) html += " <span class='alert'>(Out of Range!)</span>";
    html += "</p></div>";
    
    html += "<div class='box'><h2>pH Level</h2><p>" + String(pH, 2);
    if (pH < 5 || pH > 9) html += " <span class='alert'>(Out of Range!)</span>";
    html += "</p></div>";
    
    html += "<div class='box'><h2>Turbidity</h2><p>" + String(turbidity);
    if (turbidity < 3000 || turbidity > 4095) html += " <span class='alert'>(Out of Range!)</span>";
    html += "</p></div>";

    html += "<div class='box'><h2>GPS Data</h2><p>" + gpsData + "</p></div>";
    html += "<div class='box'><h2>Google API Location</h2><p>Latitude: " + latitude + "<br>Longitude: " + longitude + "</p></div>";
    
    html += "<p><form action='/start_motor' method='post'><button type='submit'>Start Motor</button></form></p>";
    html += "<p><form action='/stop_motor' method='post'><button type='submit'>Stop Motor</button></form></p>";

    html += "</div></body></html>";
    
    server.send(200, "text/html", html);
}

void handleStartMotor() {
    motorForward();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleStopMotor() {
    motorStop();
    server.sendHeader("Location", "/");
    server.send(303);
}

void setup() {
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    Serial.begin(115200);
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
    sensors.begin();
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) { 
        delay(1000); 
        Serial.print("."); 
    }
    
    Serial.println("\nConnected to Wi-Fi");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    getLocation();
    server.on("/", handleRoot);
    server.on("/start_motor", HTTP_POST, handleStartMotor);
    server.on("/stop_motor", HTTP_POST, handleStopMotor);
    server.begin();
}

void loop() {
    server.handleClient();
}