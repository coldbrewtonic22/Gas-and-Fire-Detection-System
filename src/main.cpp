char BLYNK_AUTH_TOKEN[32] =     "";
#define BLYNK_TEMPLATE_ID       "TMPL6pfVLC6bj"
#define BLYNK_TEMPLATE_NAME     "Gas and Fire Detection"

#include <WiFi.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleKalmanFilter.h>

#include "def.h"
#include "config.h"

Servo door;
BlynkTimer timer;
WebServer server(80);
LiquidCrystal_I2C LCD(0x27, 16, 2);
SimpleKalmanFilter filter(2, 2, 0.1);

int HYSTERESIS = 100;
int gasThreshold = 22;

bool doorState = OFF;
bool buttonState = HIGH;
bool gasDetected = false;
bool fireDetected = false;
bool blynkConnect = false;
bool buzzerActive = false;
bool lastButtonState = HIGH;
bool startupComplete = false;
bool userSilencedBuzzer = false;
bool sendNotificationsOnce = false;

unsigned long startupTime = 0;
unsigned long debounceDelay = 50;
unsigned long lastSensorCheck = 0;
unsigned long lastDebounceTime = 0;

TaskHandle_t TaskBlynk_Handle = NULL;
TaskHandle_t TaskBuzzer_Handle = NULL;
TaskHandle_t TaskButton_Handle = NULL;
TaskHandle_t TaskWebServer_Handle = NULL;
TaskHandle_t TaskMainDisplay_Handle = NULL;

int readMQ2();
int readMHSensor();
void Timer();
void startLCD();
void setupLCD();
void printMQ2();
void openDoor();
void closeDoor();
void connectSTA();
void configPage();
void startSystem();
void switchAPmode();
void checkSensors();
void handleAlerts();
void buzzerWarning();
void sendDatatoBlynk();
void sendSuccessPage();
void readConfigEEPROM();
void clearConfigEEPROM();
void handleConfigSubmit();
void controlDoor(bool ONOFF);
void TaskBlynk(void* pvParameters);
void TaskBuzzer(void* pvParameters);
void TaskButton(void* pvParameters);
void TaskWebServer(void* pvParameters);
void sendErrorPage(String errorMessage);
void TaskMainDisplay(void* pvParameters);
void writeThresholdEEPROM(int gasThreshold);
void LCDprint(int columns, int rows, String message, bool isClear);

void setup()
{
    EEPROM.begin(512);
    Serial.begin(9600);

    Serial.println("System Starting...");

    setupLCD();

    WiFi.mode(WIFI_AP_STA);     Serial.println("Configuring WiFi...");
    
    pinMode(LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(RELAY_FAN, OUTPUT);
    pinMode(MQ2_SENSOR, INPUT);
    pinMode(RELAY_PUMP, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(MH_SENSOR, INPUT_PULLUP);

    digitalWrite(LED, LOW);
    digitalWrite(RELAY_FAN, OFF);
    digitalWrite(RELAY_PUMP, OFF);
    digitalWrite(BUZZER, BUZZER_OFF);
    digitalWrite(MH_SENSOR, MH_SENSOR_ON);

    door.setPeriodHertz(50);          
    door.attach(SERVO, 500, 2400);
    closeDoor();

    readConfigEEPROM();
    connectSTA();

    gasThreshold = EEPROM.read(202) * 100 + EEPROM.read(203);
    if (gasThreshold > 9999)    gasThreshold = GAS_THRESHOLD;
    Serial.print("Gas Threshold: ");    Serial.println(gasThreshold);

    startSystem();

    Serial.println("\nCreating FreeRTOS Tasks...");

    xTaskCreatePinnedToCore(TaskWebServer,   "TaskWebServer",   8192, NULL, 5, &TaskWebServer_Handle,   0);
    xTaskCreatePinnedToCore(TaskBlynk,       "TaskBlynk",       8192, NULL, 5, &TaskBlynk_Handle,       0);
    xTaskCreatePinnedToCore(TaskMainDisplay, "TaskMainDisplay", 4096, NULL, 5, &TaskMainDisplay_Handle, 1);
    xTaskCreatePinnedToCore(TaskBuzzer,      "TaskBuzzer",      2048, NULL, 5, &TaskBuzzer_Handle,      1);
    xTaskCreatePinnedToCore(TaskButton,      "TaskButton",      2048, NULL, 5, &TaskButton_Handle,      1);

    Serial.println("All Tasks created successfully!\n");
}

void loop()
{
    vTaskDelete(NULL);
}

void TaskWebServer(void* pvParameters)
{
    Serial.println("[TaskWebServer] Started");

    while (true)
    {
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
        {
            server.handleClient();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void TaskBlynk(void* pvParameters)
{
    Serial.println("[TaskBlynk] Started");

    sendDatatoBlynk();

    while (true)
    {
        if (blynkConnect)
        {
            Blynk.run();
            timer.run();
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void TaskMainDisplay(void* pvParameters)
{
    delay(1000);
    
    Serial.println("[TaskMainDisplay] Started");

    startupTime = millis();

    while (!startupComplete)
    {
        unsigned long elapsed = (millis() - startupTime) / 1000;
        if (elapsed < 60)
        {
            static unsigned long lastUpdate = 0;
            if (millis() - lastUpdate >= 1000)
            {
                int timeRemain = 60 - elapsed;
                LCDprint(0, 1, "Wait: " + String(timeRemain) + " (s)", false);
                lastUpdate = millis();
            }
        }
        else
        {
            startupComplete = true;
            LCDprint(0, 0, "System's ready", true);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            LCD.clear();

            Serial.println("[TaskMainDisplay] Warmup completed - System ready!");
        }
    }

    printMQ2();

    while (true)
    {
        checkSensors();
        handleAlerts();

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void TaskBuzzer(void* pvParameters)
{
    Serial.println("[TaskBuzzer] Started");

    while (true)
    {
        if (buzzerActive)
        {
            buzzerWarning();
        }
        else
        {
            digitalWrite(BUZZER, BUZZER_OFF);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void TaskButton(void* pvParameters)
{
    Serial.println("[TaskButton] Started");

    while (true)
    {
        int buttonRead = digitalRead(BUTTON);
        if (buttonRead != lastButtonState)
        {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay)
        {
            if (buttonRead != buttonState)
            {
                buttonState = buttonRead;
                if (buttonState == LOW)
                {
                    Serial.println("[TaskButton] Button pressed - Stoping alerts!");
                    
                    buzzerActive = false;
                    userSilencedBuzzer = true;
                    
                    digitalWrite(BUZZER, BUZZER_OFF);

                    LCDprint(0, 0, "Buzzer Silenced", true);    
                    LCDprint(0, 1, "by User", false);

                    vTaskDelay(2000 / portTICK_PERIOD_MS);
                }
            }
        }

        lastButtonState = buttonRead;

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void readConfigEEPROM()
{
    Serial.println("\nReading Configuration from EEPROM");

    for (int i = 0; i < 32; i++)
    {
        char c = char(EEPROM.read(i));
        if (c != 0 && c != 255) EEPROMssid += c;
    }

    for (int i = 32; i < 96; i++)
    {
        char c = char(EEPROM.read(i));
        if (c != 0 && c != 255) EEPROMpassword += c;
    }

    for (int i = 96; i < 128; i++)
    {
        char c = char(EEPROM.read(i));
        if (c != 0 && c != 255) EEPROMblynkToken += c;
    }

    Serial.print("SSID: ");         Serial.println(EEPROMssid);
    Serial.print("Password: ");     Serial.println(EEPROMpassword);
    Serial.print("Blynk Token: ");  Serial.println(EEPROMblynkToken);
}

void clearConfigEEPROM()
{
    Serial.println("Clearing configuration EEPROM (0-127)...");

    for (int i = 0; i < 128; i++)   EEPROM.write(i, 0);
    EEPROM.commit();

    Serial.println("Configuration EEPROM cleared!");
}

void checkSensors()
{
    int gasValue = readMQ2();
    int fireValue = readMHSensor();

    if (gasValue > gasThreshold)                       gasDetected = true;
    else if (gasValue < gasThreshold - HYSTERESIS)     gasDetected = false;
    fireDetected = fireValue == MH_SENSOR_ON;

    Serial.print("Gas: ");      Serial.print(gasValue);
    Serial.print(" | Fire: ");  Serial.println(fireValue);
}

void handleAlerts()
{
    static bool lastAlertState = false;
    bool currentAlertState = (gasDetected || fireDetected);

    if (currentAlertState && !lastAlertState)
    {
        if (userSilencedBuzzer)
        {
            Serial.println("Reactivating Buzzer");
        }

        buzzerActive = true;
        userSilencedBuzzer = false;
    }
    lastAlertState = currentAlertState;
    
    if (gasDetected && fireDetected)
    {
        if (!userSilencedBuzzer) 
        {
            buzzerActive = true;
        }
        else
        {
            buzzerActive = false;
        }

        digitalWrite(LED, HIGH);
        
        openDoor(); doorState = ON;
        
        digitalWrite(RELAY_FAN, ON);
        digitalWrite(RELAY_PUMP, ON);

        if (!sendNotificationsOnce && blynkConnect)
        {
            Blynk.logEvent("gas_fire_detection", "WARNING: FIRE & GAS DETECTED!");
            Blynk.virtualWrite(SERVO_PIN, 1);
            Blynk.virtualWrite(RELAY_PIN, 3);

            Serial.println("WARNING: GAS & FIRE DETECTED!");

            sendNotificationsOnce = true;
        }

        LCDprint(4, 0, "WARNING!", true);
        LCDprint(2, 1, "GAS & FIRE", false);
    }
    else if (gasDetected && !fireDetected) 
    {
        if (!userSilencedBuzzer) 
        {
            buzzerActive = true;
        }
        else
        {
            buzzerActive = false;
        }

        digitalWrite(LED, HIGH);
        
        openDoor();     doorState = ON;
        
        digitalWrite(RELAY_FAN, ON);
        digitalWrite(RELAY_PUMP, OFF);

        if (!sendNotificationsOnce && blynkConnect) 
        {
            Blynk.logEvent("gas_fire_detection", "GAS CONCENTRATION HIGH!");
            Blynk.virtualWrite(SERVO_PIN, 1);
            Blynk.virtualWrite(RELAY_PIN, 1);

            Serial.println("WARNING: GAS CONCENTRATION HIGH!");

            sendNotificationsOnce = true;
        }

        LCDprint(4, 0, "WARNING", true);
        LCDprint(2, 1, "GAS DETECTED", true);
    }
    else if (!gasDetected && fireDetected)
    {
        if (!userSilencedBuzzer) 
        {
            buzzerActive = true;
        }
        else
        {
            buzzerActive = false;
        }

        digitalWrite(LED, HIGH);
        
        openDoor();     doorState = ON;

        digitalWrite(RELAY_FAN, OFF);
        digitalWrite(RELAY_PUMP, ON);
        
        if (!sendNotificationsOnce && blynkConnect)
        {
            Blynk.logEvent("gas_fire_detection", "FIRE DETECTED!");
            Blynk.virtualWrite(SERVO_PIN, 1);
            Blynk.virtualWrite(RELAY_PIN, 2);

            Serial.println("WARNING: FIRE DETECTED!");

            sendNotificationsOnce = true;
        }

        LCDprint(4, 0, "WARNING", true);
        LCDprint(2, 1, "FIRE DETECTED", false);
    }
    else
    {
        buzzerActive = false;
        userSilencedBuzzer = false;

        digitalWrite(LED, LOW);
        
        closeDoor();    doorState = OFF;
        
        digitalWrite(RELAY_FAN, OFF);
        digitalWrite(RELAY_PUMP, OFF);

        Blynk.virtualWrite(SERVO_PIN, 0);
        Blynk.virtualWrite(RELAY_PIN, 0);

        sendNotificationsOnce = false;

        printMQ2();
    }
}

void LCDprint(int columns, int rows, String message, bool isClear)
{
    if (isClear)    LCD.clear();

    LCD.setCursor(columns, rows);
    LCD.print(message);
}

void startLCD()
{   
    LCDprint(1, 0, "Gas and Fire", true);
    LCDprint(0, 1, "Detection System", false);
}

void setupLCD()
{
    LCD.init();     LCD.backlight();

    startLCD();

    delay(3000);    LCD.clear();
}

void startSystem()
{
    LCDprint(0, 0, "Warming Up", true);
    LCDprint(0, 1, "Sensors...", false);

    startupTime = millis();
}

int readMHSensor()
{
    return digitalRead(MH_SENSOR);
}

int readMQ2()
{
    float MQ2value = analogRead(MQ2_SENSOR);
    MQ2value = filter.updateEstimate(MQ2value);
    MQ2value = map(MQ2value, 0, 4095, 0, 10000);

    return int(MQ2value);
}

void printMQ2()
{
    int MQ2value = readMQ2();

    LCDprint(0, 0, "System running", true);
    LCDprint(0, 1, "Gas:" + String(MQ2value) + "ppm  ", false);
}

void writeThresholdEEPROM(int gasThreshold)
{
    int firstTwoDigits = gasThreshold / 100;
    int secondTwoDigits = gasThreshold % 100;

    EEPROM.write(202, firstTwoDigits);
    EEPROM.write(203, secondTwoDigits);
    EEPROM.commit();

    Serial.print("Threshold saved to EEPROM: ");
    Serial.println(gasThreshold);
}

void openDoor()
{
    door.write(90);
}

void closeDoor()
{
    door.write(0);
}

void controlDoor(bool ONOFF)
{
    if (ONOFF)  openDoor();
    else        closeDoor();
}

void buzzerWarning()
{
    digitalWrite(BUZZER, BUZZER_ON);    delay(1000);
    digitalWrite(BUZZER, BUZZER_OFF);   delay(100);
}

void configPage()
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Gas & Fire Detection System - Configuration</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        html, body { width: 100%; height: 100%; overflow: hidden; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            background: linear-gradient(-45deg, #ff4444, #ff6b35, #ffa500, #ffd700, #4caf50, #2196f3);
            background-size: 400% 400%;
            animation: gradientShift 20s ease infinite;
            position: relative;
        }
        @keyframes gradientShift {
            0% { background-position: 0% 50%; }
            25% { background-position: 100% 50%; }
            50% { background-position: 100% 100%; }
            75% { background-position: 0% 100%; }
            100% { background-position: 0% 50%; }
        }
        .bg-shapes { position: fixed; width: 100%; height: 100%; top: 0; left: 0; pointer-events: none; overflow: hidden; }
        .shape { position: absolute; opacity: 0.1; animation: float 25s infinite ease-in-out; }
        .shape:nth-child(1) { width: 200px; height: 200px; background: radial-gradient(circle, #ff4444, transparent); top: 10%; left: 10%; animation-delay: 0s; }
        .shape:nth-child(2) { width: 300px; height: 300px; background: radial-gradient(circle, #ffa500, transparent); top: 60%; right: 10%; animation-delay: 5s; }
        .shape:nth-child(3) { width: 150px; height: 150px; background: radial-gradient(circle, #4caf50, transparent); bottom: 20%; left: 20%; animation-delay: 10s; }
        .shape:nth-child(4) { width: 250px; height: 250px; background: radial-gradient(circle, #2196f3, transparent); top: 30%; right: 30%; animation-delay: 15s; }
        @keyframes float {
            0%, 100% { transform: translate(0, 0) rotate(0deg) scale(1); }
            25% { transform: translate(50px, 50px) rotate(90deg) scale(1.1); }
            50% { transform: translate(0, 100px) rotate(180deg) scale(0.9); }
            75% { transform: translate(-50px, 50px) rotate(270deg) scale(1.05); }
        }
        .particles { position: fixed; width: 100%; height: 100%; top: 0; left: 0; pointer-events: none; }
        .particle { position: absolute; width: 4px; height: 4px; background: white; border-radius: 50%; opacity: 0.6; animation: particleFloat 20s infinite linear; }
        .particle:nth-child(1) { left: 10%; animation-delay: 0s; animation-duration: 15s; }
        .particle:nth-child(2) { left: 20%; animation-delay: 2s; animation-duration: 18s; }
        .particle:nth-child(3) { left: 30%; animation-delay: 4s; animation-duration: 20s; }
        .particle:nth-child(4) { left: 40%; animation-delay: 6s; animation-duration: 16s; }
        .particle:nth-child(5) { left: 50%; animation-delay: 8s; animation-duration: 19s; }
        .particle:nth-child(6) { left: 60%; animation-delay: 10s; animation-duration: 17s; }
        .particle:nth-child(7) { left: 70%; animation-delay: 12s; animation-duration: 21s; }
        .particle:nth-child(8) { left: 80%; animation-delay: 14s; animation-duration: 15s; }
        .particle:nth-child(9) { left: 90%; animation-delay: 16s; animation-duration: 22s; }
        @keyframes particleFloat {
            0% { transform: translateY(100vh) rotate(0deg); opacity: 0; }
            10% { opacity: 0.6; }
            90% { opacity: 0.6; }
            100% { transform: translateY(-100px) rotate(720deg); opacity: 0; }
        }
        .scroll-container { max-height: 100vh; overflow-y: auto; overflow-x: hidden; width: 100%; display: flex; justify-content: center; align-items: flex-start; padding: 20px; }
        .container {
            background: rgba(255, 255, 255, 0.98);
            backdrop-filter: blur(30px) saturate(150%);
            border-radius: 40px;
            padding: 50px 45px;
            max-width: 550px;
            width: 100%;
            box-shadow: 0 40px 100px rgba(0, 0, 0, 0.4), 0 0 0 1px rgba(255, 255, 255, 0.5) inset, 0 0 80px rgba(255, 107, 53, 0.3);
            animation: containerFadeIn 0.8s cubic-bezier(0.34, 1.56, 0.64, 1);
            position: relative;
            z-index: 10;
            margin: 20px 0;
        }
        @keyframes containerFadeIn { from { opacity: 0; transform: translateY(50px) scale(0.9); } to { opacity: 1; transform: translateY(0) scale(1); } }
        .header { text-align: center; margin-bottom: 40px; animation: headerSlideDown 1s ease-out 0.2s both; padding-top: 10px; }
        @keyframes headerSlideDown { from { opacity: 0; transform: translateY(-30px); } to { opacity: 1; transform: translateY(0); } }
        .fire-icon-container { position: relative; display: inline-block; margin-bottom: 15px; }
        .fire-icon { font-size: 80px; display: inline-block; filter: drop-shadow(0 0 30px rgba(255, 68, 68, 0.8)); animation: firePulse 2.5s ease-in-out infinite; }
        @keyframes firePulse {
            0%, 100% { transform: scale(1) rotate(-5deg); filter: drop-shadow(0 0 30px rgba(255, 68, 68, 0.8)); }
            50% { transform: scale(1.15) rotate(5deg); filter: drop-shadow(0 0 50px rgba(255, 165, 0, 1)); }
        }
        .glow-ring { position: absolute; top: 50%; left: 50%; width: 150px; height: 150px; border: 3px solid transparent; border-radius: 50%; transform: translate(-50%, -50%); animation: glowRing 3s linear infinite; }
        @keyframes glowRing {
            0% { border-color: rgba(255, 68, 68, 0.5); transform: translate(-50%, -50%) scale(0.8) rotate(0deg); opacity: 1; }
            50% { border-color: rgba(255, 165, 0, 0.3); }
            100% { border-color: rgba(255, 68, 68, 0); transform: translate(-50%, -50%) scale(1.5) rotate(360deg); opacity: 0; }
        }
        .header h1 { font-size: 28px; font-weight: 800; margin-bottom: 10px; background: linear-gradient(135deg, #ff4444 0%, #ff6b35 25%, #ffa500 50%, #4caf50 75%, #2196f3 100%); -webkit-background-clip: text; -webkit-text-fill-color: transparent; background-clip: text; letter-spacing: -0.5px; animation: textShimmer 3s ease-in-out infinite; background-size: 200% auto; line-height: 1.2; }
        @keyframes textShimmer { 0%, 100% { background-position: 0% 50%; } 50% { background-position: 100% 50%; } }
        .header p { color: #666; font-size: 15px; font-weight: 500; margin-top: 8px; }
        .form-group { margin-bottom: 25px; animation: formSlideUp 0.6s ease-out both; }
        .form-group:nth-child(1) { animation-delay: 0.3s; }
        .form-group:nth-child(2) { animation-delay: 0.4s; }
        .form-group:nth-child(3) { animation-delay: 0.5s; }
        .form-group:nth-child(4) { animation-delay: 0.6s; }
        @keyframes formSlideUp { from { opacity: 0; transform: translateY(20px); } to { opacity: 1; transform: translateY(0); } }
        label { display: block; margin-bottom: 10px; font-weight: 700; color: #333; font-size: 14px; text-transform: uppercase; letter-spacing: 0.5px; }
        .optional-tag { font-size: 11px; font-weight: 500; color: #666; text-transform: none; margin-left: 5px; background: #e0e0e0; padding: 2px 8px; border-radius: 4px; }
        .input-wrapper { position: relative; }
        input { width: 100%; padding: 16px 20px; border: 3px solid #e0e0e0; border-radius: 16px; font-size: 16px; font-weight: 500; transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1); background: white; font-family: inherit; color: #333; }
        input[type="password"], input[type="text"]#password { padding-right: 55px; }
        input:focus { outline: none; border-color: #ff6b35; background: #fff9f5; box-shadow: 0 0 0 4px rgba(255, 107, 53, 0.15), 0 8px 24px rgba(255, 107, 53, 0.2); transform: translateY(-2px); }
        input::placeholder { color: #aaa; font-weight: 400; }
        .toggle-password { position: absolute; right: 18px; top: 50%; transform: translateY(-50%); cursor: pointer; font-size: 20px; user-select: none; transition: all 0.3s ease; }
        .toggle-password:hover { transform: translateY(-50%) scale(1.1); }
        .toggle-password:active { transform: translateY(-50%) scale(0.95); }
        .char-counter { position: absolute; right: 20px; top: 50%; transform: translateY(-50%); font-size: 13px; font-weight: 600; color: #999; pointer-events: none; transition: all 0.3s ease; }
        .char-counter.valid { color: #4caf50; }
        .char-counter.invalid { color: #ff4444; }
        .info-box { background: linear-gradient(135deg, rgba(33, 150, 243, 0.1) 0%, rgba(33, 150, 243, 0.05) 100%); border-left: 5px solid #2196f3; padding: 16px 18px; border-radius: 12px; margin-bottom: 25px; font-size: 14px; line-height: 1.7; color: #0d47a1; box-shadow: 0 4px 12px rgba(33, 150, 243, 0.1); animation: infoPulse 2s ease-in-out infinite; }
        @keyframes infoPulse { 0%, 100% { box-shadow: 0 4px 12px rgba(33, 150, 243, 0.1); } 50% { box-shadow: 0 6px 16px rgba(33, 150, 243, 0.2); } }
        .info-box strong { color: #1565c0; }
        .submit-btn { width: 100%; padding: 20px; border: none; border-radius: 18px; font-size: 18px; font-weight: 800; text-transform: uppercase; letter-spacing: 1px; cursor: pointer; position: relative; overflow: hidden; background: linear-gradient(135deg, #ff4444 0%, #ff6b35 25%, #ffa500 50%, #ffd700 100%); color: white; box-shadow: 0 12px 35px rgba(255, 68, 68, 0.4), 0 0 0 3px rgba(255, 255, 255, 0.3) inset; transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1); animation: buttonBounce 0.8s ease-out 0.7s both; }
        @keyframes buttonBounce { 0% { opacity: 0; transform: scale(0.8); } 50% { transform: scale(1.05); } 100% { opacity: 1; transform: scale(1); } }
        .submit-btn::before { content: ""; position: absolute; top: 50%; left: 50%; width: 0; height: 0; border-radius: 50%; background: rgba(255, 255, 255, 0.4); transform: translate(-50%, -50%); transition: width 0.8s, height 0.8s; }
        .submit-btn:hover { transform: translateY(-4px) scale(1.02); box-shadow: 0 20px 50px rgba(255, 68, 68, 0.5), 0 0 0 3px rgba(255, 255, 255, 0.3) inset; }
        .submit-btn:hover::before { width: 400px; height: 400px; }
        .submit-btn:active { transform: translateY(-2px) scale(0.98); }
        .submit-btn:disabled { cursor: not-allowed; opacity: 0.7; }
        .submit-btn span { position: relative; z-index: 1; display: flex; align-items: center; justify-content: center; gap: 10px; }
        @media (max-width: 600px) {
            .container { padding: 35px 25px; border-radius: 30px; margin: 10px; }
            .header h1 { font-size: 22px; }
            .fire-icon { font-size: 60px; }
            input { font-size: 15px; padding: 14px 18px; }
            input[type="password"], input[type="text"]#password { padding-right: 50px; }
            .submit-btn { padding: 18px; font-size: 16px; }
            .toggle-password { font-size: 18px; }
        }
    </style>
</head>
<body>
    <div class="bg-shapes">
        <div class="shape"></div>
        <div class="shape"></div>
        <div class="shape"></div>
        <div class="shape"></div>
    </div>
    <div class="particles">
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
        <div class="particle"></div>
    </div>
    <div class="scroll-container">
        <div class="container">
            <div class="header">
                <div class="fire-icon-container">
                    <div class="glow-ring"></div>
                    <div class="fire-icon">🔥</div>
                </div>
                <h1>GAS & FIRE DETECTION SYSTEM</h1>
                <p>WiFi and Blynk Token Configuration</p>
            </div>
            <form id="configForm" onsubmit="handleSubmit(event)">
                <div class="form-group">
                    <label for="ssid">📡 SSID</label>
                    <div class="input-wrapper">
                        <input type="text" id="ssid" name="ssid" placeholder="Enter WiFi network name" maxlength="32" required autocomplete="off">
                    </div>
                </div>
                <div class="form-group">
                    <label for="password">🔒 Password <span class="optional-tag">Optional</span></label>
                    <div class="input-wrapper">
                        <input type="password" id="password" name="password" placeholder="Leave empty for open network" maxlength="64" autocomplete="off">
                        <span class="toggle-password" onclick="togglePassword()">👁️</span>
                    </div>
                </div>
                <div class="info-box">
                    <strong>💡 How to get Blynk Token:</strong><br>
                    Open Blynk App → <strong>Project Settings</strong> → <strong>Auth Token</strong>
                </div>
                <div class="form-group">
                    <label for="token">🎫 Token</label>
                    <div class="input-wrapper">
                        <input type="text" id="token" name="token" placeholder="Enter Blynk authentication token (32 chars)" maxlength="32" required autocomplete="off" oninput="updateCharCounter()">
                        <span class="char-counter" id="charCounter">0/32</span>
                    </div>
                </div>
                <button type="submit" class="submit-btn">
                    <span>💾 SAVE CONFIGURATION</span>
                </button>
            </form>
        </div>
    </div>
    <script>
        let passwordVisible = false;
        function togglePassword() {
            const passwordInput = document.getElementById("password");
            const toggleIcon = document.querySelector(".toggle-password");
            passwordVisible = !passwordVisible;
            if (passwordVisible) {
                passwordInput.type = "text";
                toggleIcon.textContent = "🙈";
            } else {
                passwordInput.type = "password";
                toggleIcon.textContent = "👁️";
            }
        }
        function updateCharCounter() {
            const tokenInput = document.getElementById("token");
            const counter = document.getElementById("charCounter");
            const length = tokenInput.value.length;
            counter.textContent = length + "/32";
            counter.classList.remove("valid", "invalid");
            if (length === 32) {
                counter.classList.add("valid");
            } else if (length > 0) {
                counter.classList.add("invalid");
            }
        }
        function handleSubmit(event) {
            event.preventDefault();
            const ssid = document.getElementById("ssid").value.trim();
            const password = document.getElementById("password").value; // Don't trim - allow spaces in password
            const token = document.getElementById("token").value.trim();
            
            // SSID is required
            if (!ssid) {
                alert("Please enter WiFi SSID!");
                return false;
            }
            
            // Password is OPTIONAL - can be empty for open networks
            // No validation needed for password
            
            // Token must be exactly 32 characters
            if (!token) {
                alert("Please enter Blynk Token!");
                return false;
            }
            if (token.length !== 32) {
                alert("Blynk token must be exactly 32 characters!\nCurrent length: " + token.length);
                return false;
            }
            
            const btn = document.querySelector(".submit-btn");
            btn.disabled = true;
            btn.innerHTML = "<span>⏳ Saving Configuration...</span>";
            btn.style.background = "linear-gradient(135deg, #4CAF50 0%, #45a049 100%)";
            setTimeout(() => {
                const params = new URLSearchParams({
                    ssid: ssid,
                    pass: password,  // Send even if empty
                    token: token
                });
                window.location.href = "/save?" + params.toString();
                setTimeout(() => {
                    btn.innerHTML = "<span>✅ Configuration Saved!</span>";
                }, 500);
            }, 1500);
            return false;
        }
        window.addEventListener("load", () => {
            document.getElementById("ssid").focus();
            updateCharCounter();
        });
        const inputs = document.querySelectorAll("input");
        inputs.forEach((input) => {
            input.addEventListener("input", function() {
                if (this.value.length > 0) {
                    this.style.fontWeight = "600";
                } else {
                    this.style.fontWeight = "500";
                }
            });
        });
    </script>
</body>
</html>
    )rawliteral";
    
    server.send(200, "text/html", html);
}

void handleConfigSubmit()
{
    Serial.println("\n[WebServer] Configuration Submitted!");

    vTaskSuspend(TaskMainDisplay_Handle);

    if (server.hasArg("ssid") && server.hasArg("token"))
    {
        WEBPAGEssid = server.arg("ssid");
        WEBPAGEblynkToken = server.arg("token");
        
        if (server.hasArg("pass")) 
        {
            WEBPAGEpassword = server.arg("pass");
        } 
        else 
        {
            WEBPAGEpassword = "";
        }

        if (WEBPAGEssid.length() < 1 || WEBPAGEssid.length() > 32) 
        {
            sendErrorPage("Invalid SSID length (1-32 characters)");
            vTaskResume(TaskMainDisplay_Handle);
            return;
        }

        if (WEBPAGEpassword.length() > 64) 
        {
            sendErrorPage("Password too long (maximum 64 characters)");
            vTaskResume(TaskMainDisplay_Handle);
            return;
        }

        if (WEBPAGEblynkToken.length() != 32) 
        {
            sendErrorPage("Invalid Token length (must be 32 characters)");
            vTaskResume(TaskMainDisplay_Handle);
            return;
        }

        Serial.println("Received Credentials:");
        Serial.print("\tSSID: ");       
        Serial.println(WEBPAGEssid);
        Serial.print("\tPassword: "); 

        if (WEBPAGEpassword.length() == 0) 
        {
            Serial.println("[EMPTY - Open Network]");
        } 
        else 
        {
            Serial.println(WEBPAGEpassword);
        }

        Serial.print("\tToken: ");      
        Serial.println(WEBPAGEblynkToken);

        LCDprint(0, 0, "Config WiFi STA", true);
        LCDprint(0, 1, "Please Check", false);
        delay(2000);
        
        LCDprint(0, 0, WEBPAGEssid, true);

        if (WEBPAGEpassword.length() == 0) 
        {
            LCDprint(0, 1, "[Open Network]", false);
        } 
        else 
        {
            LCDprint(0, 1, WEBPAGEpassword, false);
        }
        delay(5000);

        clearConfigEEPROM();    delay(10);

        for (int i = 0; i < WEBPAGEssid.length(); i++)
        { 
            EEPROM.write(i, WEBPAGEssid[i]); 
        }

        for (int i = 0; i < WEBPAGEpassword.length(); i++)
        {
            EEPROM.write(i + 32, WEBPAGEpassword[i]);
        }

        for (int i = 0; i < WEBPAGEblynkToken.length(); i++)
        {
            EEPROM.write(i + 96, WEBPAGEblynkToken[i]);
        }
        
        EEPROM.commit();
        Serial.println("Credentials saved to EEPROM!");

        sendSuccessPage();

        LCDprint(0, 0, "    RESTART   ", true);      delay(2000);
        LCDprint(0, 1, "     DONE     ", false);     delay(2000);

        Serial.println("Restarting ESP32...");
        ESP.restart();
    }
    else
    {
        sendErrorPage("Missing required fields (SSID and Token)");
        vTaskResume(TaskMainDisplay_Handle);
    }
}

void sendSuccessPage() 
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Success</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: linear-gradient(135deg, #FF4444 0%, #FFA500 50%, #4CAF50 100%); 
            display: flex; 
            justify-content: center; 
            align-items: center; 
            height: 100vh; 
            margin: 0; 
        }
        .box { 
            background: white; 
            padding: 50px; 
            border-radius: 30px; 
            text-align: center; 
            box-shadow: 0 30px 80px rgba(0,0,0,0.4); 
            max-width: 500px;
        }
        h1 { 
            color: #4CAF50; 
            font-size: 48px; 
            margin-bottom: 20px; 
            animation: scaleIn 0.5s ease;
        }
        @keyframes scaleIn {
            0% { transform: scale(0); }
            50% { transform: scale(1.1); }
            100% { transform: scale(1); }
        }
        p { color: #666; font-size: 18px; line-height: 1.8; margin: 15px 0; }
        .spinner { 
            border: 5px solid #f3f3f3; 
            border-top: 5px solid #FF6B35; 
            border-radius: 50%; 
            width: 60px; 
            height: 60px; 
            animation: spin 1s linear infinite; 
            margin: 30px auto; 
        }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
        .info { 
            background: #fff3cd; 
            padding: 15px; 
            border-radius: 10px; 
            margin-top: 20px; 
            font-size: 14px; 
            color: #856404;
        }
    </style>
</head>
<body>
    <div class='box'>
        <h1>✅ Success!</h1>
        <p><strong>Configuration saved successfully!</strong></p>
        <p>Connecting to: <strong>)rawliteral" + WEBPAGEssid + R"rawliteral(</strong></p>
        <div class='spinner'></div>
        <p style='color: #999; font-size: 14px;'>System is restarting...</p>
        <div class='info'>
            <strong>⏳ Please wait 30-60 seconds</strong><br>
            System will connect to your WiFi and Blynk
        </div>
    </div>
</body>
</html>
    )rawliteral";
    
    server.send(200, "text/html", html);
}

void sendErrorPage(String errorMessage) 
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Error</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%); 
            display: flex; 
            justify-content: center; 
            align-items: center; 
            height: 100vh; 
            margin: 0; 
        }
        .box { 
            background: white; 
            padding: 50px; 
            border-radius: 30px; 
            text-align: center; 
            box-shadow: 0 30px 80px rgba(0,0,0,0.4); 
            max-width: 500px;
        }
        h1 { color: #f5576c; font-size: 48px; margin-bottom: 20px; }
        p { color: #666; font-size: 18px; margin: 20px 0; }
        a { 
            display: inline-block; 
            margin-top: 20px; 
            padding: 15px 40px; 
            background: #FF6B35; 
            color: white; 
            text-decoration: none; 
            border-radius: 15px; 
            font-weight: 600;
            transition: all 0.3s;
        }
        a:hover {
            background: #FF4444;
            transform: translateY(-2px);
        }
    </style>
</head>
<body>
    <div class='box'>
        <h1>❌ Error</h1>
        <p>)rawliteral" + errorMessage + R"rawliteral(</p>
        <a href='/'>← Go Back</a>
    </div>
</body>
</html>
    )rawliteral";
    
    server.send(400, "text/html", html);
}

void connectSTA()
{
    if (EEPROMssid.length() > 1)
    {
        Serial.println("\nAttemping WiFi Connection");
        Serial.print("SSID: ");         Serial.println(EEPROMssid);
        Serial.print("Password: ");     Serial.println(EEPROMpassword);
        Serial.print("Blynk Token: ");  Serial.println(EEPROMblynkToken);

        WiFi.begin(EEPROMssid.c_str(), EEPROMpassword.c_str());

        int countConnect = 0;
        String dotConnect = "";
        while (WiFi.status() != WL_CONNECTED)
        {
            LCDprint(0, 0, "WiFi Connecting", true);
            delay(500);

            dotConnect += ".";
            if (dotConnect.length() > 15)
            {
                dotConnect = "";
                LCD.clear();
            }
            LCDprint(0, 1, dotConnect, false);

            if (countConnect++ == 20)
            {
                Serial.println("\nConnection Failed! - Timeout after 20 attemps");

                LCDprint(0, 0, "Disconnect WiFi", true);
                LCDprint(0, 1, "Check Again", false);
                delay(2000);

                LCDprint(0, 0, "Connect ESP32", true);
                LCDprint(0, 1, "192.168.4.1", false);
                delay(2000);

                switchAPmode();
                return;
            }
        }

        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        LCDprint(0, 0, "WiFi Connected", true);
        LCDprint(0, 1, EEPROMssid, false);
        delay(2000);

        Serial.println("Connecting to Blynk");

        Blynk.config(EEPROMblynkToken.c_str());
        blynkConnect = Blynk.connect();

        if (blynkConnect == false)
        {
            Serial.println("Blynk Connection Failed!");

            LCDprint(0, 0, "Disconnect Blynk", true);
            LCDprint(0, 1, "Check Again", false);
            delay(3000);

            switchAPmode();
        }
        else
        {
            Serial.println("Blynk Connected!");

            LCDprint(0, 0, "Blynk Connected", true);
            delay(1000);

            timer.setInterval(2000L, Timer);

            sendDatatoBlynk();

            Serial.println("System Ready\n");
        }
    }
    else
    {
        Serial.println("No WiFi credentials found in EEPROM");
        Serial.println("Switching to Access Point mode for configuring...");

        LCDprint(0, 0, "No WiFi Config", true);
        LCDprint(0, 1, "Setup Required", false);

        switchAPmode();
    }
}

void switchAPmode()
{
    Serial.println("\nSwitching to Access Point mode");

    WiFi.softAP(APssid, APpassword);    delay(100);

    IPAddress apIP = WiFi.softAPIP();
    Serial.print("Access Point IP Address: ");
    Serial.println(apIP);

    server.on("/", configPage);
    server.on("/save", handleConfigSubmit);
    
    server.begin();

    Serial.println("WebServer Started");
    Serial.println("1. Connect to WiFi: 'ESP32'");
    Serial.println("2. Open Browser: 192.168.4.1");
    Serial.println("3. Configure WiFi and Blynk credentials");

    delay(300);
}

void Timer()
{
    if (blynkConnect) Blynk.virtualWrite(GAS_PIN, readMQ2());
}

BLYNK_WRITE(RELAY_PIN)
{
    int relayState = param.asInt();

    switch (relayState) 
    {
        case 0:
            digitalWrite(RELAY_FAN, OFF);
            digitalWrite(RELAY_PUMP, OFF);
            break;
        case 1:
            digitalWrite(RELAY_FAN, ON);
            digitalWrite(RELAY_PUMP, OFF);
            break;
        case 2:
            digitalWrite(RELAY_FAN, OFF);
            digitalWrite(RELAY_PUMP, ON);
            break;
        case 3:
            digitalWrite(RELAY_FAN, ON);
            digitalWrite(RELAY_PUMP, ON);
            break;
    }
}

BLYNK_WRITE(SERVO_PIN)
{
    doorState = param.asInt();
    controlDoor(doorState);
}

BLYNK_WRITE(THRESHOLD_PIN)
{
    gasThreshold = param.asInt();
    writeThresholdEEPROM(gasThreshold);

    vTaskSuspend(TaskMainDisplay_Handle);

    Serial.print("Threshold updated to: ");
    Serial.println(gasThreshold);
    
    Timer();

    vTaskResume(TaskMainDisplay_Handle);
}

void sendDatatoBlynk()
{
    if (blynkConnect)
    {
        Blynk.virtualWrite(GAS_PIN, readMQ2());
        Blynk.virtualWrite(THRESHOLD_PIN, gasThreshold);
        Blynk.virtualWrite(SERVO_PIN, doorState);
    }
}