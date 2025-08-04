#include "ICM_20948.h" 
#include <Arduino.h>
#include <lx16a-servo.h>

#include <WiFi.h>

#define WIRE_PORT Wire
#define AD0_VAL 0

ICM_20948_I2C myICM;
LX16ABus servoBus;
LX16AServo* servos[18];

const char* SSID     = "hex";
const char* PASSWORD = "krolika123";

WiFiClient persistentClient;
bool clientConnected = false;

// Joint positions - adjusted for better stability and movement
const int32_t COXA_DEFAULT  = 12000;
const int32_t COXA_FORWARD  = 14500;  // Much more forward movement
const int32_t COXA_BACKWARD = 9500;   // Much more backward movement

const int32_t FEMUR_DOWN    = 17000;
const int32_t FEMUR_UP      = 18500; 

const int32_t TIBIA_DOWN    = 7000;   // Better ground contact
const int32_t TIBIA_UP      = 10000;   // Less aggressive lift

// Proper tripod gait pattern
// Tripod 1: Front Left, Middle Right, Back Left
// Tripod 2: Front Right, Middle Left, Back Right
const int TRIPOD1_LEGS[] = {15, 12, 6};  // Front Left, Middle Right, Back Left
const int TRIPOD2_LEGS[] = {0, 3, 9};    // Front Right, Middle Left, Back Right

// Timing constants - faster movement
const int MOVE_TIME  = 300;   // Faster movement
const int LIFT_TIME  = 150;   // Faster lift
const int PUSH_TIME  = 350;   // Faster push
const int LOWER_TIME = 250;   // Faster lower

const int SHORT_DELAY = 50;   // Much shorter delays

const int PORT = 8080;

WiFiServer server(PORT);

enum Gait{
    TRIPOD,
    WAVE,
    RIPPLE
};

Gait currentGait = TRIPOD;

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

    // Initialize all legs to neutral position
    for (int i = 0; i < 18; i += 3) {
        servos[i]->move_time(COXA_DEFAULT, 1500);
        servos[i+1]->move_time(FEMUR_DOWN, 1500);
        servos[i+2]->move_time(TIBIA_DOWN, 1500);
    }

    delay(1500);
    Serial.println("Ready to walk!");
}

bool isRightSide(int base) {
    return base == 3 || base == 6 || base == 15;  // Right side legs
}

void moveLeg(int base, int32_t coxa, int32_t femur, int32_t tibia, int time = MOVE_TIME) {
    servos[base]->move_time(coxa, time);
    servos[base+1]->move_time(femur, time);
    servos[base+2]->move_time(tibia, time);
}



void waveGait() {
    const int WAVE_ORDER[] = {15, 3, 6, 9, 12, 0};  // FL, ML, BL, BR, MR, FR
    const int BODY_PUSH_DELTA = 800;

    // ⚡ Faster timings
    const int FAST_LIFT_TIME  = 100;
    const int FAST_MOVE_TIME  = 120;
    const int FAST_LOWER_TIME = 100;
    const int FAST_PUSH_TIME  = 100;
    const int FAST_DELAY      = 30;

    for (int step = 0; step < 6; step++) {
        int leg = WAVE_ORDER[step];

        // === LIFT ===
        Serial.printf("Lifting Leg %d\n", leg);
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, FAST_LIFT_TIME);
        delay(FAST_LIFT_TIME + FAST_DELAY);

        // === SWING ===
        int coxa_target = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Swinging Forward Leg %d\n", leg);
        moveLeg(leg, coxa_target, FEMUR_UP, TIBIA_UP, FAST_MOVE_TIME);
        delay(FAST_MOVE_TIME + FAST_DELAY);

        // === LOWER ===
        Serial.printf("Lowering Leg %d\n", leg);
        moveLeg(leg, coxa_target, FEMUR_DOWN, TIBIA_DOWN, FAST_LOWER_TIME);
        delay(FAST_LOWER_TIME + FAST_DELAY);

        // === BODY PUSH ===
        Serial.println("Shifting body forward");
        for (int j = 0; j < 6; j++) {
            int support_leg = WAVE_ORDER[j];
            if (support_leg != leg) {
                int32_t current_coxa = servos[support_leg]->pos_read();
                int32_t shift = isRightSide(support_leg) ? -BODY_PUSH_DELTA : BODY_PUSH_DELTA;
                int32_t new_coxa = constrain(current_coxa + shift, COXA_BACKWARD, COXA_FORWARD);
                moveLeg(support_leg, new_coxa, FEMUR_DOWN, TIBIA_DOWN, FAST_PUSH_TIME);
            }
        }

        delay(FAST_PUSH_TIME + FAST_DELAY);
    }
}





void tripodGait() {
    // === PHASE 1: Tripod 1 swings forward, Tripod 2 in stance ===
    Serial.println("Phase 1: Tripod 1 swings, Tripod 2 stance");

    // Step 1: Lift Tripod 1
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        Serial.printf("Lifting Leg %d\n", leg);
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 2: Swing Tripod 1 forward
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Swinging Forward Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 3: Lower Tripod 1
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Lowering Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 4: Push Tripod 2 backward
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_BACKWARD : COXA_FORWARD;
        Serial.printf("Pushing Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // === PHASE 2: Tripod 2 swings forward, Tripod 1 in stance ===
    Serial.println("Phase 2: Tripod 2 swings, Tripod 1 stance");

    // Step 1: Lift Tripod 2
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        Serial.printf("Lifting Leg %d\n", leg);
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 2: Swing Tripod 2 forward
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Swinging Forward Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 3: Lower Tripod 2
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Lowering Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 4: Push Tripod 1 backward
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_BACKWARD : COXA_FORWARD;
        Serial.printf("Pushing Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);
}

void loop() {
        // Check if a new client is trying to connect
    if (!clientConnected) {
        WiFiClient newClient = server.available();
        if (newClient) {
            persistentClient = newClient;
            clientConnected = true;
            Serial.println("Client Connected!");
        }
    }

    // If connected, see if client has sent any data
    if (clientConnected && persistentClient.connected()) {
        while (persistentClient.available()) {
            String incoming = persistentClient.readStringUntil('\n');  // read until newline
            incoming.trim();

            Serial.print("Received: ");
            Serial.println(incoming);

            // HANDLE COMMANDS

            // Optional: send back a response
            persistentClient.println("OK");
        }
    }

    // If client disconnected, cleanup
    if (clientConnected && !persistentClient.connected()) {
        clientConnected = false;
        persistentClient.stop();
        Serial.println("Client Disconnected.");
    }
}
