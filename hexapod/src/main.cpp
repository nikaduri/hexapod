#include "ICM_20948.h" 
#include <Arduino.h>
#include <lx16a-servo.h>
#include <Constants.h>
#include "TripodGait.h"
#include "WaveGait.h"

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

enum Direction {
    BACKWARD,
    LEFT,
    RIGHT
};

enum GaitPattern {
    TRIPOD,
    WAVE,
    RIPPLE
};

GaitPattern currentGait = TRIPOD;

bool stop = true;


void initLegs() {
    for (int i = 0; i < 18; i += 3) {
        servos[i]->move_time(COXA_DEFAULT, 0);
        servos[i+1]->move_time(FEMUR_DOWN, 0);
        servos[i+2]->move_time(TIBIA_DOWN, 0);
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

void rotate(Direction a) {

}

void moveBackward() {
    rotate(BACKWARD);
    moveForward();
}

void moveLeft() {
    rotate(LEFT);
    moveForward();
}

void moveRight() {
    rotate(RIGHT);
    moveForward();
}

void stopMoving() {
    stop = true;
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
    } else if (incoming.indexOf("TRIPOD_GAIT") != -1) {
        currentGait = TRIPOD;
    } else if (incoming.indexOf("WAVE_GAIT") != -1) {
        currentGait = WAVE;
    } else if (incoming.indexOf("RIPPLE_GAIT") != -1) {
        currentGait = RIPPLE;
    } else {
        Serial.println("Unknown command: " + incoming);
    }
}

void wifiListenTask(void* parameter) {
    for (;;) {
        if (!clientConnected) {
            WiFiClient newClient = server.available();
            if (newClient) {
                persistentClient = newClient;
                clientConnected = true;
                Serial.println("Client Connected!");
            }
        }

        if (clientConnected && persistentClient.connected()) {
            if (persistentClient.available()) {
                String incoming = persistentClient.readStringUntil('\n');
                incoming.trim();

                handleIncoming(incoming);
                persistentClient.println("OK");
            }
        }

        if (clientConnected && !persistentClient.connected()) {
            clientConnected = false;
            persistentClient.stop();
            Serial.println("Client Disconnected.");
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);  // avoid busy-wait
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
