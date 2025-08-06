#include "ICM_20948.h" 
#include <Arduino.h>
#include <lx16a-servo.h>
#include <Constants.h>
#include "TripodGait.h"
#include "WaveGait.h"
#include "Enums.h"

#include <WiFi.h>

#define WIRE_PORT Wire
#define AD0_VAL 0

ICM_20948_I2C myICM;
LX16ABus servoBus;
LX16AServo* servos[18];

TripodGait tripodGait(servoBus, servos);
WaveGait waveGait(servoBus, servos);

WiFiClient persistentClient;
bool clientConnected = false;

TaskHandle_t wifiTaskHandle = NULL;

const int PORT = 8080;

WiFiServer server(PORT);

GaitPattern currentGait = TRIPOD;

bool stop = true;


void trimIncomingString(String& incoming) {
    incoming.trim();
    // Remove all control characters (including null bytes)
    String cleanCommand = "";
    for (int i = 0; i < incoming.length(); i++) {
        char c = incoming.charAt(i);
        if (c >= 32 && c <= 126) { // Only keep printable ASCII characters
            cleanCommand += c;
        }
    }
    incoming = cleanCommand;
}


void initLegs() {
    for (int i = 0; i < 18; i += 3) {
        if(servos[i]->pos_read() != COXA_DEFAULT) {
            servos[i]->move_time(COXA_DEFAULT, 0);
        }
        if(servos[i+1]->pos_read() != FEMUR_DOWN) {
            servos[i+1]->move_time(FEMUR_DOWN, 0);
        }
        if(servos[i+2]->pos_read() != TIBIA_DOWN) {
            servos[i+2]->move_time(TIBIA_DOWN, 0);
        }
    }
}

void moveForward() {
    switch(currentGait) {
        case TRIPOD: 
            tripodGait.move();
            break;
        case WAVE:
            waveGait.move();
            break;
        case RIPPLE:
            break;
        default:
            tripodGait.move();
            break;
    }
}

void rotateLeft() {
    tripodGait.rotateInPlace(LEFT);
}

void rotateRight() {
    tripodGait.rotateInPlace(RIGHT);
}

void rotateBackward() {
    rotateLeft();
    rotateLeft();
}



void rotate(Direction direction) {
    switch(direction) {
        case BACKWARD:
            rotateBackward();
            break;
            
        case LEFT:
            rotateLeft();
            break;
            
        case RIGHT:
            rotateRight();
            break;
    }
}

void moveBackward() {
    rotate(BACKWARD);
    moveForward();
}

void moveLeft() {
    rotate(LEFT);
    stop = true;
}

void moveRight() {
    rotate(RIGHT);
    stop = true;
}

void stopMoving() {
    stop = true;
}

float getBatteryVoltage() {
    float voltage = 0;

    for(int i = 0; i < 20; i++) {
        voltage += servos[0]->vin();
        delay(10);
    }

    voltage /= 20;
    
    voltage /= 1000.0; // Convert mV to V
    
    // Clamp voltage to reasonable range for 2S battery (6.0V - 8.2V)
    if (voltage < 6.0) voltage = 6.0;
    if (voltage > 8.2) voltage = 8.2;
    
    Serial.print("Battery voltage: ");
    Serial.print(voltage, 2);
    Serial.println("V");
    
    return voltage;
}

void handleIncoming(String incoming) {
    if (incoming.indexOf("STOP") != -1) {
        stopMoving();
    } else if (incoming.indexOf("FORWARD") != -1) {
        stop = false;
        moveForward();
    } else if (incoming.indexOf("BACKWARD") != -1) {
        stop = false;
        moveBackward();
    } else if (incoming.indexOf("LEFT") != -1) {
        stop = false;
        moveLeft();
    } else if (incoming.indexOf("RIGHT") != -1) {
        stop = false;
        moveRight();
    } else if (incoming.indexOf("STAND") != -1) {
        initLegs();
        stop = true;
    } else if (incoming.indexOf("LAY_DOWN") != -1) {
        // Implement lay down position
        stop = true;
    } else if (incoming.indexOf("DANCE") != -1) {
        // Implement dance sequence
        stop = true;
    } else if (incoming.indexOf("TRIPOD_GAIT") != -1) {
        currentGait = TRIPOD;
    } else if (incoming.indexOf("WAVE_GAIT") != -1) {
        currentGait = WAVE;
    } else if (incoming.indexOf("RIPPLE_GAIT") != -1) {
        currentGait = RIPPLE;
    } else if (incoming.indexOf("STAIRCASE_MODE") != -1) {
        // Implement staircase mode
    } else if (incoming.indexOf("GET_BATTERY") != -1) {
        // Handle battery status request
        float voltage = getBatteryVoltage();
        String response = "BATTERY:" + String(voltage, 2);
        
        if (clientConnected && persistentClient.connected()) {
            persistentClient.println(response);
            Serial.println("Sent battery response: " + response);
        }
        return; // Don't send "OK" response for battery requests
    } else if (incoming.indexOf("PING") != -1) {
        // Handle keep-alive ping - just ignore it, the "OK" response is enough
        Serial.println("Keep-alive ping received");
        return; // The "OK" response will be sent by the caller
    } else {
        Serial.println("Unknown command: " + incoming);
    }
}

void wifiListenTask(void* parameter) {
    for (;;) {
        // Check for new clients (both persistent and temporary)
        WiFiClient newClient = server.available();
        if (newClient) {
            Serial.println("New client connected");
            
            // Handle this client immediately
            if (newClient.connected()) {
                if (newClient.available()) {
                    String incoming = newClient.readStringUntil('\n');
                    incoming.trim();
                    
                    // Remove null characters
                    while (incoming.length() > 0 && incoming.charAt(0) == '\0') {
                        incoming = incoming.substring(1);
                    }
                    
                    Serial.println("Received: " + incoming);
                    
                    if (incoming.indexOf("GET_BATTERY") != -1) {
                        // Handle battery request immediately
                        float voltage = getBatteryVoltage();
                        String response = "BATTERY:" + String(voltage, 2);
                        newClient.println(response);
                        Serial.println("Sent battery response: " + response);
                        newClient.flush();
                        delay(10); // Small delay to ensure data is sent
                        newClient.stop(); // Close the connection
                    } else if (incoming.length() > 0) {
                        // Handle other commands
                        handleIncoming(incoming);
                        newClient.println("OK");
                        
                        // This becomes the persistent client for movement commands
                        if (!clientConnected) {
                            persistentClient = newClient;
                            clientConnected = true;
                            Serial.println("Client set as persistent");
                        } else {
                            newClient.stop(); // Close if we already have a persistent client
                        }
                    }
                }
            }
        }

        // Handle persistent client for movement commands
        if (clientConnected && persistentClient.connected()) {
            if (persistentClient.available()) {
                String incoming = persistentClient.readStringUntil('\n');
                incoming.trim();
                
                // Remove null characters
                while (incoming.length() > 0 && incoming.charAt(0) == '\0') {
                    incoming = incoming.substring(1);
                }

                if (incoming.length() > 0) {
                    handleIncoming(incoming);
                    persistentClient.println("OK");
                }
            }
            
            // Handle ping requests for persistent client
            if (persistentClient.available() == 1) {
                byte pingByte = persistentClient.read();
                if (pingByte == 0) {
                    persistentClient.write((uint8_t)0);
                }
            }
        }

        // Check if persistent client disconnected
        if (clientConnected && !persistentClient.connected()) {
            clientConnected = false;
            persistentClient.stop();
            Serial.println("Persistent client disconnected");
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); 
    while (!Serial) {}

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();

    WIRE_PORT.begin();
    WIRE_PORT.setClock(400000);

    servoBus.beginOnePinMode(&Serial2, 15);
    servoBus.debug(false);

    Serial.println("Hexapod Starting...");

    for (int i = 0; i < 18; i++) {
        servos[i] = new LX16AServo(&servoBus, i + 1);
    }

    initLegs();

    delay(1500);
    
    Serial.println("Ready to walk!");

    // Start background task to listen to WiFi
    xTaskCreatePinnedToCore(
        wifiListenTask,     // Function to run
        "WiFiTask",         // Task name
        8192,               // Stack size
        NULL,               // Parameters
        1,                  // Priority
        &wifiTaskHandle,    // Handle
        0                   // Core
    );
}

void loop() {
    if(!stop) {
        moveForward();
    } else {
        initLegs();
    } 
}