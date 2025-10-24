#include "ICM_20948.h" 
#include <Arduino.h>
#include <lx16a-servo.h>
#include "Constants.h"
#include "TripodGait.h"
#include "WaveGait.h"
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
    String cleanCommand = "";
    for (int i = 0; i < incoming.length(); i++) {
        char c = incoming.charAt(i);
        if (c >= 32 && c <= 126) {
            cleanCommand += c;
        }
    }
    incoming = cleanCommand;
}


int getSwitchIndexForLeg(int legBase) {
    switch(legBase) {
        case 0:  return 0;
        case 3:  return 1;
        case 6:  return 2;
        case 9:  return 3;
        case 12: return 4;
        case 15: return 5;
        default: return -1;
    }
}

bool isLegGrounded(int legBase) {
    int switchIndex = getSwitchIndexForLeg(legBase);
    if (switchIndex < 0) return false;
    
    int switchPin = SWITCH_PINS[switchIndex];
    
    int pressedCount = 0;
    const int READINGS = 5;
    
    for (int i = 0; i < READINGS; i++) {
        if (digitalRead(switchPin) == 1) {
            pressedCount++;
        }
        delayMicroseconds(200);
    }
    
    return pressedCount >= 3;
}

void initLegs() {
    for (int i = 0; i < 18; i += 3) {
        if(servos[i]->pos_read() != COXA_DEFAULT) {
            servos[i]->move_time(COXA_DEFAULT, 200);
        }
        if(servos[i+1]->pos_read() != FEMUR_DOWN) {
            servos[i+1]->move_time(FEMUR_DOWN, 200);
        }
        if(servos[i+2]->pos_read() != TIBIA_DOWN) {
            servos[i+2]->move_time(TIBIA_DOWN, 200);
        }
    }

    delay(1000);

    const int32_t TIBIA_STEP_SIZE = 80;
    const int32_t FEMUR_STEP_SIZE = 60;
    const int STEP_DELAY = 25;
    const int MAX_STEPS = 40;
    
    for (int legBase = 0; legBase < 18; legBase += 3) {
        if (isLegGrounded(legBase)) {
            continue;
        }
        
        int32_t currentFemur = servos[legBase+1]->pos_read();
        int32_t currentTibia = servos[legBase+2]->pos_read();
        
        int32_t femurDirection = (currentFemur < FEMUR_DOWN) ? 1 : -1;
        int32_t tibiaDirection = (currentTibia < TIBIA_DOWN) ? 1 : -1;
        
        
        for (int step = 0; step < MAX_STEPS; step++) {
            if (isLegGrounded(legBase)) {
                Serial.print("Leg ");
                Serial.print(legBase);
                Serial.print(" grounded at femur=");
                Serial.print(currentFemur);
                Serial.print(", tibia=");
                Serial.println(currentTibia);
                break;
            }
            
            int32_t nextFemur = currentFemur + (femurDirection * FEMUR_STEP_SIZE);
            int32_t nextTibia = currentTibia + (tibiaDirection * TIBIA_STEP_SIZE);
            
            if (femurDirection > 0) {
                if (nextFemur > FEMUR_DOWN + 800) {
                    nextFemur = FEMUR_DOWN + 800;
                }
            } else {
                if (nextFemur < FEMUR_DOWN - 800) {
                    nextFemur = FEMUR_DOWN - 800;
                }
            }
            
            if (tibiaDirection < 0) {
                if (nextTibia < TIBIA_DOWN - 1000) {
                    nextTibia = TIBIA_DOWN - 1000;
                }
            } else {
                if (nextTibia > TIBIA_DOWN + 1000) {
                    nextTibia = TIBIA_DOWN + 1000;
                }
            }
            
            servos[legBase+1]->move_time(nextFemur, STEP_DELAY);
            servos[legBase+2]->move_time(nextTibia, STEP_DELAY);
            
            currentFemur = nextFemur;
            currentTibia = nextTibia;
            
            delay(STEP_DELAY);
            
            bool femurLimitReached = (femurDirection > 0 && nextFemur >= FEMUR_DOWN + 800) ||
                                     (femurDirection < 0 && nextFemur <= FEMUR_DOWN - 800);
            bool tibiaLimitReached = (tibiaDirection < 0 && nextTibia <= TIBIA_DOWN - 1000) ||
                                      (tibiaDirection > 0 && nextTibia >= TIBIA_DOWN + 1000);
            
            if (femurLimitReached && tibiaLimitReached) {
                Serial.print("Leg ");
                Serial.print(legBase);
                Serial.println(" reached safety limits without grounding!");
                break;
            }
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
        default:
            tripodGait.move();
            break;
    }
}

void moveBackward() {
    switch(currentGait) {
        case TRIPOD: 
            tripodGait.moveBackward();
            break;
        case WAVE:
            waveGait.moveBackward();
            break;
        default:
            tripodGait.moveBackward();
            break;
    }
}

void layDown() {
    Serial.println("Laying down posture (gentle, hold)");
    const int32_t targetCoxa = COXA_DEFAULT;
    const int32_t targetFemurStage1 = (FEMUR_UP - 500);
    const int32_t targetFemurStage2 = FEMUR_UP - 250;
    const int32_t targetTibiaStage1 = TIBIA_UP + 120;
    const int32_t targetTibiaStage2 = TIBIA_UP + 260;
    for (int base = 0; base < 18; base += 3) {
        servos[base]->move_time(targetCoxa, 380);
    }
    delay(420);
    for (int base = 0; base < 18; base += 3) {
        servos[base + 1]->move_time(targetFemurStage1, 420);
        servos[base + 2]->move_time(targetTibiaStage1, 420);
    }
    delay(450);
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
    for (int base = 0; base < 18; base += 3) {
        servos[base]->move_time(targetCoxa, 380);
    }
    delay(400);
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

    const int32_t femurLift = (FEMUR_UP - 280);
    const int32_t tibiaUnbend1 = (TIBIA_UP + 110);
    const int32_t tibiaUnbend2 = (TIBIA_UP + 50);
    const int32_t femurApproach = (FEMUR_DOWN + 180);
    const int32_t tibiaStance = TIBIA_DOWN;
    const int32_t femurStance = FEMUR_DOWN;

    servos[15 + 1]->move_time(FEMUR_UP - 250, 420);
    servos[15 + 2]->move_time(TIBIA_UP + 120, 420);
    delay(440);
    moveTripod(TRIPOD1_LEGS, femurLift, tibiaUnbend1, 420);
    delay(440);
    moveTripodTibiaOnly(TRIPOD1_LEGS, tibiaUnbend2, 440);
    delay(460);
    moveTripod(TRIPOD1_LEGS, femurApproach, tibiaStance, 480);
    delay(500);
    moveTripodFemurOnly(TRIPOD1_LEGS, femurStance, 500);
    delay(520);

    moveTripod(TRIPOD2_LEGS, femurLift, tibiaUnbend1, 420);
    delay(440);
    moveTripodTibiaOnly(TRIPOD2_LEGS, tibiaUnbend2, 440);
    delay(460);
    moveTripod(TRIPOD2_LEGS, femurApproach, tibiaStance, 480);
    delay(500);
    moveTripodFemurOnly(TRIPOD2_LEGS, femurStance, 500);
    delay(520);

    initLegs();

    Serial.println("Stand up complete");
}

void rotateLeft() {
    tripodGait.rotateInPlace(RIGHT);  
}

void rotateRight() {
    tripodGait.rotateInPlace(LEFT);
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

void dance() {
    Serial.println("Let's dance!");
    Serial.println("Dance move: Body Wave");
    for (int wave = 0; wave < 2; wave++) {
        for (int i = 0; i < 6; i++) {
            int legBase = WAVE_ORDER[i];
            servos[legBase + 1]->move_time(FEMUR_UP - 200, 200);
            servos[legBase + 2]->move_time(TIBIA_UP + 150, 200);
            delay(150);
            servos[legBase + 1]->move_time(FEMUR_DOWN, 200);
            servos[legBase + 2]->move_time(TIBIA_DOWN, 200);
            delay(100);
        }
    }
    Serial.println("Dance move: Twist");
    for (int twist = 0; twist < 4; twist++) {
        for (int base = 0; base < 18; base += 3) {
            int32_t target = (base % 6 == 0) ? COXA_FORWARD : COXA_BACKWARD;
            servos[base]->move_time(target, 300);
        }
        delay(350);
        for (int base = 0; base < 18; base += 3) {
            int32_t target = (base % 6 == 0) ? COXA_BACKWARD : COXA_FORWARD;
            servos[base]->move_time(target, 300);
        }
        delay(350);
    }
    for (int base = 0; base < 18; base += 3) {
        servos[base]->move_time(COXA_DEFAULT, 300);
    }
    delay(350);
    Serial.println("Dance move: Bounce");
    for (int bounce = 0; bounce < 5; bounce++) {
        for (int base = 0; base < 18; base += 3) {
            servos[base + 1]->move_time(FEMUR_UP - 300, 200);
            servos[base + 2]->move_time(TIBIA_UP + 200, 200);
        }
        delay(250);
        for (int base = 0; base < 18; base += 3) {
            servos[base + 1]->move_time(FEMUR_DOWN, 200);
            servos[base + 2]->move_time(TIBIA_DOWN, 200);
        }
        delay(250);
    }
    Serial.println("Dance move: Tripod Rock");
    for (int rock = 0; rock < 4; rock++) {
        for (int i = 0; i < 3; i++) {
            int base = TRIPOD1_LEGS[i];
            servos[base + 1]->move_time(FEMUR_UP - 250, 250);
            servos[base + 2]->move_time(TIBIA_UP + 180, 250);
        }
        delay(300);
        for (int i = 0; i < 3; i++) {
            int base = TRIPOD1_LEGS[i];
            servos[base + 1]->move_time(FEMUR_DOWN, 250);
            servos[base + 2]->move_time(TIBIA_DOWN, 250);
        }
        for (int i = 0; i < 3; i++) {
            int base = TRIPOD2_LEGS[i];
            servos[base + 1]->move_time(FEMUR_UP - 250, 250);
            servos[base + 2]->move_time(TIBIA_UP + 180, 250);
        }
        delay(300);
        for (int i = 0; i < 3; i++) {
            int base = TRIPOD2_LEGS[i];
            servos[base + 1]->move_time(FEMUR_DOWN, 250);
            servos[base + 2]->move_time(TIBIA_DOWN, 250);
        }
        delay(300);
    }
    Serial.println("Dance move: Shimmy");
    for (int shimmy = 0; shimmy < 8; shimmy++) {
        for (int base = 0; base < 18; base += 3) {
            int32_t offset = (shimmy % 2 == 0) ? 150 : -150;
            servos[base]->move_time(COXA_DEFAULT + offset, 120);
        }
        delay(150);
    }
    for (int base = 0; base < 18; base += 3) {
        servos[base]->move_time(COXA_DEFAULT, 300);
    }
    delay(350);
    Serial.println("Dance move: Finale");
    for (int i = 0; i < 6; i++) {
        int legBase = WAVE_ORDER[i];
        servos[legBase + 1]->move_time(FEMUR_UP - 150, 180);
        servos[legBase + 2]->move_time(TIBIA_UP + 120, 180);
        delay(120);
    }
    delay(200);
    for (int base = 0; base < 18; base += 3) {
        servos[base + 1]->move_time(FEMUR_DOWN, 400);
        servos[base + 2]->move_time(TIBIA_DOWN, 400);
    }
    delay(450);
    for (int base = 0; base < 18; base += 3) {
        if (base == 0 || base == 3) {
            servos[base + 1]->move_time(FEMUR_DOWN - 200, 500);
        } else if (base == 12 || base == 15) {
            servos[base + 1]->move_time(FEMUR_DOWN + 200, 500);
        }
    }
    delay(800);
    initLegs();
    Serial.println("Dance complete!");
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
        currentMode = MOVE_BACKWARD;
    } else if (incoming.indexOf("LEFT") != -1) {
        moveLeft();
    } else if (incoming.indexOf("RIGHT") != -1) {
        moveRight();
    } else if (incoming.indexOf("STAND") != -1) {
        currentMode = IDLE;
    } else if (incoming.indexOf("LAY_DOWN") != -1) {
        currentMode = LAY_DOWN;
    } else if (incoming.indexOf("DANCE") != -1) {
        currentMode = DANCE;
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
    } else if (incoming.indexOf("STAND_UP") != -1) {
        currentMode = STAND_UP;
    } else if (incoming.indexOf("PING") != -1) {
        Serial.println("Keep-alive ping received");
        return;
    } else {
        Serial.println("Unknown command: " + incoming);
    }
}

void wifiListenTask(void* parameter) {
    for (;;) {
        WiFiClient newClient = server.available();
        if (newClient) {
            Serial.println("New client connected");
            
            if (newClient.connected()) {
                if (newClient.available()) {
                    String incoming = newClient.readStringUntil('\n');
                    incoming.trim();
                    
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
                        delay(10); 
                        newClient.stop(); 
                    } else if (incoming.length() > 0) {
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

        if (clientConnected && persistentClient.connected()) {
            if (persistentClient.available()) {
                String incoming = persistentClient.readStringUntil('\n');
                incoming.trim();
                
                while (incoming.length() > 0 && incoming.charAt(0) == '\0') {
                    incoming = incoming.substring(1);
                }

                if (incoming.length() > 0) {
                    handleIncoming(incoming);
                    persistentClient.println("OK");
                }
            }
            
            if (persistentClient.available() == 1) {
                byte pingByte = persistentClient.read();
                if (pingByte == 0) {
                    persistentClient.write((uint8_t)0);
                }
            }
        }

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

    for (int i = 0; i < 6; i++) {
        pinMode(SWITCH_PINS[i], INPUT);  // Use pull-down for digital switches
    }
    Serial.println("Switch pins initialized for ground detection (digital mode)");

    initLegs();

    // Initialize gyroscope
    myICM.begin(WIRE_PORT, AD0_VAL);
    
    
    if (myICM.status != ICM_20948_Stat_Ok) {
        Serial.println("ERROR: ICM-20948 not connected!");
    } else {
        Serial.println("OK");
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
        case MOVE_BACKWARD:
            moveBackward();
            break;
        case IDLE:
            initLegs();
            break;
        case LAY_DOWN:
            layDown();
            currentMode = NONE;
            break;
        case STAND_UP:
            standUp();
            currentMode = IDLE;
            break;
        case DANCE:
            dance();
            break;
        case BALANCE:
        case NONE:
            break;
        default:
            initLegs();
            break;
    }
     
}