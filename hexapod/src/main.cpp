#include "ICM_20948.h" 
#include <Arduino.h>
#include <lx16a-servo.h>
#include "Constants.h"
#include "TripodGait.h"
#include "WaveGait.h"
#include "HorizontalBalance.h"
#include "Enums.h"
#include "BatteryReader.h"

#include <WiFi.h>

#define WIRE_PORT Wire
#define AD0_VAL 0

ICM_20948_I2C myICM;
LX16ABus servoBus;
LX16AServo* servos[18];

TripodGait tripodGait(servoBus, servos);
WaveGait waveGait(servoBus, servos);
HorizontalBalance balanceSystem(myICM, servos);
BatteryReader batteryReader(batteryPin);

WiFiClient persistentClient;
bool clientConnected = false;

TaskHandle_t wifiTaskHandle = NULL;

const int PORT = 8080;

WiFiServer server(PORT);

GaitPattern currentGait = TRIPOD;
 
RobotMode currentMode = IDLE;


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
    // Move legs to stable standing position only if they're not already there
    bool anyMoved = false;
    for (int i = 0; i < 18; i += 3) {
        if(servos[i]->pos_read() != COXA_DEFAULT) {
            servos[i]->move_time(COXA_DEFAULT, 200);
            anyMoved = true;
        }
        if(servos[i+1]->pos_read() != FEMUR_DOWN) {
            servos[i+1]->move_time(FEMUR_DOWN, 200);
            anyMoved = true;
        }
        if(servos[i+2]->pos_read() != TIBIA_DOWN) {
            servos[i+2]->move_time(TIBIA_DOWN, 200);
            anyMoved = true;
        }
    }
    if(anyMoved) {
        delay(250); // Only delay if servos actually moved
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

void layDown() {
    Serial.println("Laying down posture (gentle, hold)");

    // Gentler movement: slower timings and smaller deltas
    const int32_t targetCoxa = COXA_DEFAULT;

    // Soften femur/tibia targets for less aggressive tuck (avoid min() to silence -Wstrict-overflow)
    const int32_t targetFemurStage1 = (FEMUR_UP - 500); // smaller initial raise
    const int32_t targetFemurStage2 = FEMUR_UP - 250;                // less extreme final

    const int32_t targetTibiaStage1 = TIBIA_UP + 120; // small bend
    const int32_t targetTibiaStage2 = TIBIA_UP + 260; // moderate bend

    // Stage 1: ensure coxa are neutral
    for (int base = 0; base < 18; base += 3) {
        servos[base]->move_time(targetCoxa, 380);
    }
    delay(420);

    // Stage 2: gentle femur raise + slight tibia bend
    for (int base = 0; base < 18; base += 3) {
        servos[base + 1]->move_time(targetFemurStage1, 420);
        servos[base + 2]->move_time(targetTibiaStage1, 420);
    }
    delay(450);

    // Stage 3: finish tuck with moderate femur/tibia targets
    for (int base = 0; base < 18; base += 3) {
        servos[base + 1]->move_time(targetFemurStage2, 520);
        servos[base + 2]->move_time(targetTibiaStage2, 520);
    }
    delay(540);

    Serial.println("Lay down complete");
}

void standUp() {
    Serial.println("Standing up from laid posture (tripod-staged)");

    const int32_t targetCoxa = COXA_DEFAULT;

    // Step 0: neutralize all coxa to avoid sweeping
    for (int base = 0; base < 18; base += 3) {
        servos[base]->move_time(targetCoxa, 380);
    }
    delay(400);

    // Helper lambdas for grouped moves
    auto moveTripod = [&](const int legs[3], int32_t femur, int32_t tibia, int time) {
        for (int i = 0; i < 3; i++) {
            int base = legs[i];
            servos[base + 1]->move_time(femur, time);
            servos[base + 2]->move_time(tibia, time);
        }
    };

    auto moveTripodFemurOnly = [&](const int legs[3], int32_t femur, int time) {
        for (int i = 0; i < 3; i++) {
            int base = legs[i];
            servos[base + 1]->move_time(femur, time);
        }
    };

    auto moveTripodTibiaOnly = [&](const int legs[3], int32_t tibia, int time) {
        for (int i = 0; i < 3; i++) {
            int base = legs[i];
            servos[base + 2]->move_time(tibia, time);
        }
    };

    // Conservative targets
    const int32_t femurLift = (FEMUR_UP - 280);
    const int32_t tibiaUnbend1 = (TIBIA_UP + 110);
    const int32_t tibiaUnbend2 = (TIBIA_UP + 50);
    const int32_t femurApproach = (FEMUR_DOWN + 180);
    const int32_t tibiaStance = TIBIA_DOWN;
    const int32_t femurStance = FEMUR_DOWN;

    // Tripod 1 staged stand-up (15, 12, 6)
    // Targeted relief for leg 15 first (known sticky)
    servos[15 + 1]->move_time(FEMUR_UP - 250, 420);
    servos[15 + 2]->move_time(TIBIA_UP + 120, 420);
    delay(440);

    // Stage A1: lift femur and unbend tibia
    moveTripod(TRIPOD1_LEGS, femurLift, tibiaUnbend1, 420);
    delay(440);
    // Stage A2: more tibia extension while keeping femur high
    moveTripodTibiaOnly(TRIPOD1_LEGS, tibiaUnbend2, 440);
    delay(460);
    // Stage A3: lower femur towards stance and extend tibia to stance
    moveTripod(TRIPOD1_LEGS, femurApproach, tibiaStance, 480);
    delay(500);
    // Stage A4: finalize femur stance
    moveTripodFemurOnly(TRIPOD1_LEGS, femurStance, 500);
    delay(520);

    // Tripod 2 staged stand-up (0, 3, 9)
    // Stage B1: lift femur and unbend tibia
    moveTripod(TRIPOD2_LEGS, femurLift, tibiaUnbend1, 420);
    delay(440);
    // Stage B2: more tibia extension while keeping femur high
    moveTripodTibiaOnly(TRIPOD2_LEGS, tibiaUnbend2, 440);
    delay(460);
    // Stage B3: lower femur towards stance and extend tibia to stance
    moveTripod(TRIPOD2_LEGS, femurApproach, tibiaStance, 480);
    delay(500);
    // Stage B4: finalize femur stance
    moveTripodFemurOnly(TRIPOD2_LEGS, femurStance, 500);
    delay(520);

    // Small final normalization to exact stance for all legs
    initLegs();

    Serial.println("Stand up complete");
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
    currentMode = IDLE;
}

void moveRight() {
    rotate(RIGHT);
    currentMode = IDLE;
}

void stopMoving() {
    currentMode = IDLE;
}

int getBatteryPercentage() {
    return batteryReader.getPercentage();
}

void handleIncoming(String incoming) {
    if (incoming.indexOf("STOP") != -1) {
        currentMode = IDLE;
    } else if (incoming.indexOf("FORWARD") != -1) {
        currentMode = MOVE_FORWARD;
    } else if (incoming.indexOf("BACKWARD") != -1) {
        // Rotate 180 then continue moving forward
        rotate(BACKWARD);
        currentMode = MOVE_FORWARD;
    } else if (incoming.indexOf("LEFT") != -1) {
        // One-off rotation
        moveLeft();
    } else if (incoming.indexOf("RIGHT") != -1) {
        // One-off rotation
        moveRight();
    } else if (incoming.indexOf("STAND") != -1) {
        currentMode = IDLE;
    } else if (incoming.indexOf("LAY_DOWN") != -1) {
        currentMode = LAY_DOWN;
    } else if (incoming.indexOf("DANCE") != -1) {
        currentMode = BALANCE;
    } else if (incoming.indexOf("BALANCE") != -1) {
        currentMode = BALANCE;
    } else if (incoming.indexOf("TRIPOD_GAIT") != -1) {
        currentGait = TRIPOD;
    } else if (incoming.indexOf("WAVE_GAIT") != -1) {
        currentGait = WAVE;
    } else if (incoming.indexOf("RIPPLE_GAIT") != -1) {
        currentGait = RIPPLE;
    } else if (incoming.indexOf("STAIRCASE_MODE") != -1) {
        // Implement staircase mode
    } else if (incoming.indexOf("GET_BATTERY") != -1) {
        int percentage = getBatteryPercentage();
        String response = "BATTERY:" + String(percentage);
        
        if (clientConnected && persistentClient.connected()) {
            persistentClient.println(response);
            Serial.println("Sent battery response: " + response);
        }
        return; // Don't send "OK" response for battery requests
    } else if (incoming.indexOf("CALIBRATE_BALANCE") != -1) {
        // Calibrate balance system
        Serial.println("Calibrating balance system...");
        balanceSystem.calibrateLevel();
        Serial.println("Balance calibration complete!");
    } else if (incoming.indexOf("BALANCE_STATUS") != -1) {
        // Report balance system status
        bool isActive = balanceSystem.isBalanceActive();
        double roll = balanceSystem.getRoll();
        double pitch = balanceSystem.getPitch();
        
        String status = "BALANCE_STATUS:Active=" + String(isActive ? "true" : "false") + 
                       ",Roll=" + String(roll, 2) + ",Pitch=" + String(pitch, 2);
        
        if (clientConnected && persistentClient.connected()) {
            persistentClient.println(status);
            Serial.println("Sent: " + status);
        }
        return;
    } else if (incoming.indexOf("AUTOTUNE_BALANCE") != -1) {
        // Run PID autotuning
        Serial.println("Starting balance system autotuning...");
        if (currentMode != BALANCE) {
            // ensure safe stance before autotune
            initLegs();
            delay(500);
        }

        if (balanceSystem.autotunePID()) {
            Serial.println("Autotuning completed successfully!");
        } else {
            Serial.println("Autotuning failed!");
        }
        return;
    } else if (incoming.indexOf("STAND_UP") != -1) {
        currentMode = STAND_UP;
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
                        int percentage = getBatteryPercentage();
                        String response = "BATTERY:" + String(percentage);
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

    // Initialize gyroscope
    myICM.begin(WIRE_PORT, AD0_VAL);
    
    Serial.print("Initializing ICM-20948... ");
    
    if (myICM.status != ICM_20948_Stat_Ok) {
        Serial.println("ERROR: ICM-20948 not connected!");
    } else {
        Serial.println("OK");
        
        // Initialize balance system
        if (balanceSystem.begin()) {
            Serial.println("Balance system ready!");
        } else {
            Serial.println("ERROR: Balance system initialization failed!");
        }
    }

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
    switch (currentMode) {
        case MOVE_FORWARD:
            moveForward();
            break;
        case IDLE:
            initLegs();
            break;
        case LAY_DOWN:
            layDown();
            currentMode = LAID_DOWN; // hold laid down posture until commanded otherwise
            break;
        case LAID_DOWN:
            // Keep servos at last commanded positions; do nothing
            delay(20);
            break;
        case STAND_UP:
            standUp();
            currentMode = IDLE;
            break;
        case BALANCE:
            // Handle balance mode
            static bool balanceEntered = false;
            if (!balanceEntered) {
                balanceSystem.enterBalanceMode();
                balanceEntered = true;
                Serial.println("Balance mode activated!");
            }
            
            // Update balance control loop
            balanceSystem.updateBalance();
            
            // Add a small delay to prevent overwhelming the system
            delay(10);
            break;
        default:
            initLegs();
            break;
    }
    
    // Handle mode transitions
    static RobotMode lastMode = IDLE;
    static bool balanceWasActive = false;
    
    // If we are not in BALANCE mode, do not touch balance system
    if (lastMode == BALANCE && currentMode != BALANCE && balanceWasActive) {
        balanceSystem.exitBalanceMode();
        balanceWasActive = false;
        Serial.println("Exited balance mode");
    }
    if (currentMode == BALANCE) {
        balanceWasActive = true;
    }
    
    lastMode = currentMode;
}