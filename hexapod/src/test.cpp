// #include <Arduino.h>
// #include <lx16a-servo.h>

// LX16ABus servoBus;
// LX16AServo* servos[18];
// static int32_t LOW_POS = 0.1 * 24000;
// static int32_t HIGH_POS = 0.6 * 24000;

// void setup() {
//     Serial.begin(115200);
//     servoBus.beginOnePinMode(&Serial2, 17);  
//     servoBus.debug(false);  
//     Serial.println("Beginning Servo Example");

//     for (int i = 0; i < 18; i++) {
//         servos[i] = new LX16AServo(&servoBus, i + 1);  // Servo IDs are 1 to 18
//     }
// }

// void loop() {
//     for (int i = 2; i < 18; i += 3) {  // Servos 3,6,9,... (0-based index)
//         servos[i]->move_time(LOW_POS,0);
//     }
//     delay(1000);
//     for (int i = 2; i < 18; i += 3) {
//         servos[i]->move_time(HIGH_POS,0);
//     }
//     delay(1000);
// }
