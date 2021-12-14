#include <ESP8266WiFi.h>
#include <TridentTD_LineNotify.h>
#include <SoftwareSerial.h>

#define SSID        "Pop"
#define PASSWORD    "Pop62010948"
#define LINE_TOKEN  "wliVrQ09eSNb8EJslVficLJEit3aoyATGcBtlKWaZiM"

SoftwareSerial NodeSerial(D2, D3); // RX | TX
String Data = "";
char currentPayload[11][30];
char currentCmd[30];

const char pre_define_valid_cmd[5][10] = {
  "EXC",
  "STA",
  "HAZ",
  "VUH",
  "UHT",
};

bool is_valid(void) {
  for (int i = 0; i < 5; i++) {
    if (strcmp(pre_define_valid_cmd[i], currentCmd) == 0) {
      return true;
    }
  }
  return false;
}

// Extract CMD and payload frame
void extract_payload(char data[]) {
  // Populate an array (Reset)
  for (int i = 0; i < 11; i++)
  {
    strcpy(currentPayload[i], "Empty");
  }

  int currentIndex = 0;
  // Extract the first token
  char * token = strtok(data, " ");
  // loop through the string to extract all other tokens
  while ( token != NULL ) {
    char stringBuffer[30];
    sprintf(stringBuffer, "%s" , token);
    strcpy(currentPayload[currentIndex], stringBuffer);
    currentIndex++;
    token = strtok(NULL, " ");
  }

  // Store current command
  strcpy(currentCmd, currentPayload[0]);

  // ! For debug only
  for (int i = 0; i < 11; i++) {
    Serial.println(currentPayload[i]);
  }
}

// Command branch
void decode_cmd(void) {
  if (strcmp(currentCmd, "EXC") == 0) {
    Serial.println("Execute EXC");
    notify_exceed_pm();
  }
  else if (strcmp(currentCmd, "STA") == 0) {
    Serial.println("Execute STA");
    notify_push_start_string();
  }
  else if (strcmp(currentCmd, "HAZ") == 0) {
    Serial.println("Execute HAZ");
    notify_push_haz_string();
  }
  else if (strcmp(currentCmd, "VUH") == 0) {
    Serial.println("Execute VUH");
    notify_push_vuh_string();
  }
  else if (strcmp(currentCmd, "UHT") == 0) {
    Serial.println("Execute UHT");
    notify_push_uht_string();
  }
}

void setup() {
  Serial.begin(115200);
  NodeSerial.begin(115200);
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());

  LINE.setToken(LINE_TOKEN);

  Serial.println();
  Serial.println("NodeMCU/ESP8266 Run");
  Serial.println(LINE.getVersion());

}

void send_pm_table(void) {
  char stringBuffer[400];
  sprintf(stringBuffer, "\nCurrent PM Value (microgram per cubic meter)\n\nPM1.0 = %s\nPM2.5 = %s\nPM4.0 = %s\nPM10 = %s" , currentPayload[1], currentPayload[2], currentPayload[3], currentPayload[4]);
  LINE.notify(stringBuffer);
}

void notify_exceed_pm(void) {
  LINE.notifySticker("\nALERT! Current PM Value is exeed recommended nomal consumption value.\n\nExposure to fine particles can cause short-term health effects such as eye, nose, throat and lung irritation, coughing, sneezing, runny nose and shortness of breath.\n\nExposure in long-term could pose danger to your health.", 11537, 52002749);
  send_pm_table();
}

void notify_push_start_string(void) {
  LINE.notify("\nSPS30 is now wake-up! You will receive notification about PM Value from now on.");
}

void notify_push_haz_string(void){
  LINE.notify("\nNow the air is Hazardous.");
}

void notify_push_vuh_string(void){
  LINE.notify("\nNow the air is Very Unhealty.");
}

void notify_push_uht_string(void){
  LINE.notify("\nNow the air is Unhealthy.");
}


void loop() {
  //  Serial.println("Waiting for incoming frame");
  while (NodeSerial.available())
  {
    char character = NodeSerial.read();
    //      Serial.print("Incoming character -> ");
    //      Serial.println(character);
    if (character != '\n') {
      Data.concat(character);
    }
    else
    {
      Serial.print("**** Received: ");
      Serial.println(Data);

      // Convert String to char*
      // Length (with one extra character for the null terminator)
      int str_len = Data.length() + 1;
      // Prepare the character array (the buffer)
      char char_array[str_len];
      // Copy it over
      Data.toCharArray(char_array, str_len);

      extract_payload(char_array);
      if (is_valid()) {
        Serial.println("This frame is valid, Start decode now...");
        decode_cmd();
      }
      else {
        Serial.println("This frame is not valid");
      }

      Data = "";
    }
  }
}
