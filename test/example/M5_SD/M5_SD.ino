#include <Arduino.h>
#include <M5Unified.h>
#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {

  auto cfg = M5.config();
  cfg.clear_display = true;
  M5.begin(cfg);

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  while (false == SD.begin(GPIO_NUM_4, SPI, 25000000))
    {
      delay(500);
    }

  Serial.println("ok");

  if (!SD.begin()) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  uint8_t uid = 0x01;
  String filename = "/" + String(uid) + ".json";
  Serial.println(filename);
  myFile = SD.open(filename, FILE_WRITE);  //add "/"
  if (myFile) {
    Serial.print("Writing to json file...");
    myFile.println("{");
    myFile.println("\t\"fingerUserID\": \"" + String(uid) + "\", ");
    myFile.println("\t\"functionsUserID\": \"userid\", ");
    myFile.println("\t\"functionsUserName\": \"name\"");
    myFile.println("}");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }

  // re-open the file for reading:
  myFile = SD.open("/test.txt");  //add "/"
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void loop() {
  // nothing happens after setup
}
