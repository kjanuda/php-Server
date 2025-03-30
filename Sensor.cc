#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <math.h>  // For power function

// WiFi Credentials
const char* ssid = "Dialog 4G";
const char* password = "200225403366";

// Server URL (Change to your actual server IP)
const String serverURL = "http://192.168.8.106/dht11_project/test_data.php";

// DHT11 Sensor
#define DHTPIN 4  // DHT11 connected to GPIO 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ-135 Sensor
int mq135Pin = 34;  // MQ-135 sensor connected to GPIO 34

// MQ-135 Constants (You must calibrate RZERO)
#define RZERO 76.63  // Base resistance of sensor at clean air
#define SCALEFACTOR 116.6020682
#define EXPONENT -2.769034857
// LED Pins and Thresholds
#define LED_TEMP_PIN 2  // LED for temperature
#define LED_HUM_PIN 15   // LED for humidity
#define BUZZER_PIN 13   // Buzzer pin
#define TEMP_THRESHOLD 32  // Temperature threshold
#define HUM_THRESHOLD 80   // Humidity threshold

// Define musical notes (frequencies in Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523


// Function to connect to WiFi
void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// Function to calculate CO2 PPM from raw analog data
float getPPM(int rawValue) {
    float ratio = rawValue / RZERO;
    float ppm = SCALEFACTOR * pow(ratio, EXPONENT);
    return ppm;
}

// Function to convert PPM to AQI level
String getAQICategory(float ppm) {
    if (ppm <= 50) return "Good";
    else if (ppm <= 100) return "Moderate";
    else if (ppm <= 150) return "Unhealthy for Sensitive Groups";
    else if (ppm <= 200) return "Unhealthy";
    else return "Very Unhealthy / Hazardous";
}

// Function to send data to the server
void sendData(float temperature, float humidity, float airQualityPPM, String aqiCategory) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverURL);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
       
       // Ensure floating point numbers are formatted with a decimal point
        char tempStr[10], humStr[10], airQStr[10];
        dtostrf(temperature, 6, 2, tempStr);   // Format to 2 decimal places
        dtostrf(humidity, 6, 2, humStr);
        dtostrf(airQualityPPM, 6, 2, airQStr);

        // Create the POST request data
        String postData = "temperature=" + String(temperature) +
                          "&humidity=" + String(humidity) +
                          "&airQuality=" + String(airQualityPPM) +
                          "&aqiCategory=" + aqiCategory;

        int httpCode = http.POST(postData);
        String payload = http.getString();

        // Print response
        Serial.println("\n[HTTP POST Request]");
        Serial.println("URL: " + serverURL);
        Serial.println("Data: " + postData);
        Serial.println("Response Code: " + String(httpCode));
        Serial.println("Response: " + payload);
        Serial.println("----------------------------------------");

        http.end();
    } else {
        Serial.println("WiFi Disconnected. Reconnecting...");
        connectWiFi();
    }
}
void playMelody() {
    int melody[] = { NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5 };
    int noteDurations[] = { 300, 300, 300, 300, 300, 300, 300, 500 };

    for (int i = 0; i < 8; i++) {
        ledcWriteTone(0, melody[i]); // Play the note
        delay(noteDurations[i]); // Hold the note
        ledcWrite(0, 0); // Stop sound between notes
        delay(50);
    }
}



#define LED_PIN 2  // Define the GPIO pin for the LED
#define TEMP_THRESHOLD 32  // Set the temperature threshold
#define AQI_THRESHOLD 10  // Air Quality threshold (PPM)
void setup() {
    Serial.begin(115200);
    dht.begin();  // Initialize DHT11 sensor
    // Setup PWM for the buzzer
ledcSetup(0, 2000, 8);  // Channel 0, 2 kHz frequency, 8-bit resolution
ledcAttachPin(BUZZER_PIN, 0);

      // Initialize LED pins
    pinMode(LED_TEMP_PIN, OUTPUT);
    pinMode(LED_HUM_PIN, OUTPUT);
     pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(LED_TEMP_PIN, LOW); // Initially turn off LED
    digitalWrite(LED_HUM_PIN, LOW);  // Initially turn off LED
    digitalWrite(BUZZER_PIN, LOW);//Initially turn off BUFFER
    connectWiFi();   // Connect to WiFi
}

void loop() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int rawAirQuality = analogRead(mq135Pin);  // Read raw MQ-135 sensor data
    float airQualityPPM = getPPM(rawAirQuality); // Convert to PPM
    String aqiCategory = getAQICategory(airQualityPPM); // Convert PPM to AQI level

     // **Control LEDs independently**
    if (temperature > TEMP_THRESHOLD) {
        digitalWrite(LED_TEMP_PIN, HIGH); // Turn on temperature LED
    } else {
        digitalWrite(LED_TEMP_PIN, LOW); // Turn off temperature LED
    }

    if (humidity > HUM_THRESHOLD) {
        digitalWrite(LED_HUM_PIN, HIGH); // Turn on humidity LED
    } else {
        digitalWrite(LED_HUM_PIN, LOW); // Turn off humidity LED
    }

    // Buzzer & Melody
    if (temperature > TEMP_THRESHOLD || humidity > HUM_THRESHOLD || airQualityPPM > AQI_THRESHOLD) {
        Serial.println("Warning! Playing Melody...");
        playMelody();  
    }

    // Print sensor data to Serial Monitor
    Serial.println("\n[Sensor Readings]");
    Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Air Quality (Raw ADC): "); Serial.println(rawAirQuality);
    Serial.print("Air Quality (CO2 PPM): "); Serial.println(airQualityPPM);
    Serial.print("AQI Category: "); Serial.println(aqiCategory);

    // Send sensor data to the server
    sendData(temperature, humidity, airQualityPPM, aqiCategory);

    delay(3000);  // Wait for 5 seconds before the next reading
}
