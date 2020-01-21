#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pitches.h"
#include "themes.h"
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

// OLED FeatherWing buttons map to different pins depending on board:
#if defined(ESP8266)
#define BUTTON_A  0
#define BUTTON_B 16
#define BUTTON_C  2
#elif defined(ESP32)
#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
#define BUTTON_A PA15
#define BUTTON_B PC7
#define BUTTON_C PC5
#elif defined(TEENSYDUINO)
#define BUTTON_A  4
#define BUTTON_B  3
#define BUTTON_C  8
#elif defined(ARDUINO_FEATHER52832)
#define BUTTON_A 31
#define BUTTON_B 30
#define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840 and 328p
#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5
#endif

#define BAUD_RATE 9600

#define OLED_PIN 12
#define MOTOR_PIN 13
#define SPEAKER_PIN 14
#define NEO_PIN 15

#define MOTOR_IN false

#define NUMPIXELS      8
#define PIN_PIXELS     15 // Use pin 15 for NeoPixels
#define MAX_INTENSITY  80

#define HOLD_OFF_TIME_TARGET 100

#define MILESTONE 1
#define CHECK_INTERVAL 10

// Tell it how many pixels, and which pin to use to send signals.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

//Change this to match your local settings
const char* ssid     = "Basement";
const char* password = "2013942380";
const char* host = "meyourdu.herokuapp.com";

int prev_num_users = 0;
int curr_num_users = 0;
int loop_count = 0;

bool start_play = false;
bool led_froze = false;
bool led_off = false;
bool led_off_set = false;
bool led_start_toggle = false;
int hold_off_time = 0;

//##############**Default Chiptune Sound Effect**##############//
// notes in the melody:
int melody_1[8] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int note_durations_1[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};
int length_1 = 8;
//###########End of Default Chiptune Sound Effect#############//

//##############**Success Chiptune Sound Effect**##############//
int melody_2[5] = {
  NOTE_C4, 0, NOTE_B3, NOTE_D4, NOTE_F4
};

int note_durations_2[] = {
  4, 8, 8, 8, 2
};
int length_2 = 5;
//###########End of Success Chiptune Sound Effect#############//

//##############**"HE IS A PIRATE" Theme song of Pirates of caribbean**##############//
int melody_pirates[] = {
  NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
  NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
  NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
  NOTE_A3, NOTE_C4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_F4,
  NOTE_F4, NOTE_G4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_C4, NOTE_C4, NOTE_D4,
  0,       NOTE_A3, NOTE_C4, NOTE_B3, NOTE_D4, NOTE_B3, NOTE_E4, NOTE_F4,
  NOTE_F4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4,
  NOTE_D4, 0,       0,       NOTE_A3, NOTE_C4, NOTE_D4, NOTE_D4, NOTE_D4,
  NOTE_F4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A4,
  NOTE_G4, NOTE_A4, NOTE_D4, 0,       NOTE_D4, NOTE_E3, NOTE_F4, NOTE_F4,
  NOTE_G4, NOTE_A4, NOTE_D4, 0,       NOTE_D4, NOTE_F4, NOTE_E4, NOTE_E4,
  NOTE_F4, NOTE_D4
};
int note_durations_pirates[] = {
  4, 8, 4, 8, 4, 8, 8, 8, 8, 4, 8, 4, 8, 4, 8, 8, 8, 8, 4, 8, 4, 8,
  4, 8, 8, 8, 8, 4, 4, 8, 8, 4, 4, 8, 8, 4, 4, 8, 8,
  8, 4, 8, 8, 8, 4, 4, 8, 8, 4, 4, 8, 8, 4, 4, 8, 4,
  4, 8, 8, 8, 8, 4, 4, 8, 8, 4, 4, 8, 8, 4, 4, 8, 8,
  8, 4, 8, 8, 8, 4, 4, 4, 8, 4, 8, 8, 8, 4, 4, 8, 8
};
int length_pirates = sizeof(melody_pirates) / sizeof(int);
//###########End of He is a Pirate song#############//

void setup() {
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    // Wait for serial port to connect. Needed for native USB
    delay(100);
  }

  Serial.println("OLED FeatherWing test");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  //Set text settings
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("Setting Up Hardware...");
  display.display();
  new_screen(2000, 1000);

  Serial.println("IO test");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  pinMode(OLED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  // Initialize NeoPixel library
  pixels.begin();
  display.print("Hardware Setup Complete!");
  display.display();

  new_screen(2000, 2000);

  display.println("Connecting to ");
  display.print("MeYouRDU");
  display.display();
  delay(1000);

  //  while (WiFi.status() != WL_CONNECTED) {
  //    display.print(".");
  //    display.display();
  //    delay(100);
  //  }

  new_screen(2000, 1000);

  display.println("WiFi connected");
  display.println("IP address: ");
  display.println(WiFi.localIP());
  display.display();

  new_screen(2000, 1000);
  display_users();
}

void loop() {
  int r = random(256);
  int g = random(256);
  int b = random(256);

  for (int intensity = 0 ; intensity < MAX_INTENSITY ; intensity++) {
    for (int i = 0 ; i < NUMPIXELS ; i++) {
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(r / 100 * intensity, g / 100 * intensity, b / 100 * intensity));
    }

    check_buttons();
    display.display();
    if (!led_froze && !led_off) pixels.show();
    delay(20);
  }

  for (int intensity = MAX_INTENSITY ; intensity >= 0 ; intensity--) {
    for (int i = 0 ; i < NUMPIXELS ; i++) {
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(r / 100 * intensity, g / 100 * intensity, b / 100 * intensity));
    }

    check_buttons();
    display.display();
    if (!led_froze && !led_off) pixels.show();
    delay(20);
  }

  if (loop_count >= CHECK_INTERVAL) {
    check_num_users();
    loop_count = 0;
  } else {
    loop_count++;
  }

}

//WHERE INPUT LOGIC IS CHECKED====================================================
void check_buttons() {
  if (!led_off) {
    if (!digitalRead(BUTTON_A)) {
      start_play = true;
      if (MOTOR_IN) digitalWrite(MOTOR_PIN, HIGH);

    } else {
      if (MOTOR_IN) digitalWrite(MOTOR_PIN, LOW);
      if (start_play) {
        display.print("New Users Detected!");
        start_play = false;

        for (int i = 0; i < 3; i++) {
          RunningLights(random(256), random(256), random(256), 75);
        }

        new_screen(500, 500);
        display_users();

        int song_choice = random(3);
        switch (song_choice) {
          case 0:
            play_melody(JingleBells_notes, JingleBells_duration, length_jinglebells);
            break;
          case 1:
            play_melody(MerryChristmas_notes, MerryChristmas_duration, length_merrychristmas);
            break;
          case 2:
            play_melody(Santa_notes, Santa_duration, length_santa);
            break;
        }

        for (int i = 0; i < 3; i++) {
          RunningLights(0xff, 0, 0, 50);      // red
          RunningLights(0xff, 0xff, 0xff, 50); // white
          RunningLights(0, 0, 0xff, 50);      // blue
        }
      }
    }
  }

  if (!digitalRead(BUTTON_B)) {
    display.print("Testing WIFI");
    new_screen(2000, 1000);
    example_wifi_test();
    display_users();
  }

  if (!digitalRead(BUTTON_C)) {
    if (led_off_set) {
      led_off_set = false;
      led_off = false;
      led_froze = true;
    }
    led_start_toggle = true;
    hold_off_time++;
    Serial.println(hold_off_time);
    if (hold_off_time > HOLD_OFF_TIME_TARGET) {
      led_off = true;
      for (int i = 0 ; i < NUMPIXELS ; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
      pixels.show();
      hold_off_time = 0;
    }
  } else {
    if (led_start_toggle) {
      if (led_off) led_off_set = true;
      if (!led_off) {
        led_froze = !led_froze;
        //if(led_on) led_off = false;
        led_start_toggle = false;
      }
    }
  }

  yield();
}

void new_screen(int pre_time, int post_time) {
  delay(pre_time);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.display();
  delay(post_time);
}

void display_users() {
  display.setTextSize(2);
  display.print("Current \nUsers: ");
  display.print(curr_num_users);
  display.display();
}

void play_melody(int input_melody[], int input_note_durations[], int melody_length) {
  int num_notes = melody_length;//(sizeof(input_melody)/sizeof(int));

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < num_notes; thisNote++) {
    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int curr_note_duration = 1000 / input_note_durations[thisNote];
    tone(SPEAKER_PIN, input_melody[thisNote], curr_note_duration);
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = curr_note_duration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(SPEAKER_PIN);
  }
}

void celebrate() {
    new_screen(100, 500);
    
    display.print("New Users Detected!");

    for (int i = 0; i < 3; i++) {
      RunningLights(random(256), random(256), random(256), 75);
    }

    new_screen(500, 500);
    display_users();

    int song_choice = random(3);
    switch (song_choice) {
      case 0:
        play_melody(JingleBells_notes, JingleBells_duration, length_jinglebells);
        break;
      case 1:
        play_melody(MerryChristmas_notes, MerryChristmas_duration, length_merrychristmas);
        break;
      case 2:
        play_melody(Santa_notes, Santa_duration, length_santa);
        break;
    }

    for (int i = 0; i < 3; i++) {
      RunningLights(0xff, 0, 0, 50);      // red
      RunningLights(0xff, 0xff, 0xff, 50); // white
      RunningLights(0, 0, 0xff, 50);      // blue
    }

}

void check_num_users() {
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    display.println("Connection failed!");
    display.display();
    return;
  }

  // We now create a URI for the request
  String url = "/users/count";
  //  Serial.print("Requesting URL: ");

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  delay(500);

  int total_users = prev_num_users;
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    total_users = line.toInt();
  }

  prev_num_users = curr_num_users;
  curr_num_users = total_users;

  if (curr_num_users >= prev_num_users + MILESTONE) celebrate();
  else display_users();

}

void example_wifi_test() {
  new_screen(500, 500);

  display.print("Connecting to ");
  display.println(host);
  delay(1000);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    display.println("Connection failed!");
    display.display();
    return;
  }

  // We now create a URI for the request
  String url = "/users/count";
  Serial.print("Requesting URL: ");
  display.println("Requesting URL: ");
  display.println(url);
  display.display();

  new_screen(2000, 1000);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  delay(500);

  int total_users = 0;
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    total_users = line.toInt();
    display.println(line);
    display.println(total_users);
    display.display();
    new_screen(1000, 500);
  }
  new_screen(500, 500);

  curr_num_users = total_users;
  display.println(total_users);
  display.println(curr_num_users);
  display.display();

  new_screen(6000, 1000);

  display.println("Closing connection...");
  display.display();

  new_screen(4000, 1000);

  display.println("WIFI test complete!");
  display.display();

  new_screen(4000, 1000);

}

//LED EFFECTS (probs best to put in separate header) ====================================================
void pulse_pixels() {
  // Pulsing in random colors
  int r = random(256);
  int g = random(256);
  int b = random(256);

  for (int intensity = 0 ; intensity < MAX_INTENSITY ; intensity++) {
    for (int i = 0 ; i < NUMPIXELS ; i++) {
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(r / 100 * intensity, g / 100 * intensity, b / 100 * intensity));
    }

    pixels.show();
    delay(20);
  }

  for (int intensity = MAX_INTENSITY ; intensity >= 0 ; intensity--) {
    for (int i = 0 ; i < NUMPIXELS ; i++) {
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(r / 100 * intensity, g / 100 * intensity, b / 100 * intensity));
    }

    pixels.show();
    delay(20);
  }
  // delay(500);
}

void theaterChaseRainbow(int SpeedDelay) {
  byte *c;

  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < NUMPIXELS; i = i + 3) {
        c = Wheel( (i + j) % 255);
        pixels.setPixelColor(i + q, pixels.Color(*c, *(c + 1), *(c + 2))); //turn every third pixel on
      }
      pixels.show();
      delay(SpeedDelay);

      for (int i = 0; i < NUMPIXELS; i = i + 3) {
        pixels.setPixelColor(i + q, pixels.Color(0, 0, 0));      //turn every third pixel off
      }
    }
  }
}

void colorWipe(byte red, byte green, byte blue, int SpeedDelay) {
  for (uint16_t i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
    pixels.show();
    delay(SpeedDelay);
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];

  if (WheelPos < 85) {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  } else {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }

  return c;
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position = 0;

  for (int j = 0; j < NUMPIXELS * 2; j++)
  {
    Position++; // = 0; //Position + Rate;
    for (int i = 0; i < NUMPIXELS; i++) {
      // sine wave, 3 offset waves make a rainbow!
      //float level = sin(i+Position) * 127 + 128;
      //setPixel(i,level,0,0);
      //float level = sin(i+Position) * 127 + 128;
      pixels.setPixelColor(i, pixels.Color(((sin(i + Position) * 127 + 128) / 255)*red,
                                           ((sin(i + Position) * 127 + 128) / 255)*green,
                                           ((sin(i + Position) * 127 + 128) / 255)*blue));
    }

    pixels.show();
    delay(WaveDelay);
  }
}

