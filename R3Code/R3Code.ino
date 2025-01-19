#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(2, 3); // For fingerprint sensor
SoftwareSerial espSerial(8, 9); // For communication with ESP32 (Tx = 8, Rx = 9)

const int buttonPin1 = 4;
const int buttonPin2 = 5;
const int buttonPin3 = 6;
const int buzzer = 7;

int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int id = 0, previous_voter_id = 0, vote_taken = 0;
int party_1_count = 0, party_2_count = 0, party_3_count = 0;
String winner_name = "";

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);

  lcd.init();
  lcd.backlight();
  Serial.begin(9600);        // For debugging
  espSerial.begin(9600);     // For ESP32 communication
  mySerial.begin(57600);     // For fingerprint sensor communication

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains ");
  Serial.print(finger.templateCount);
  Serial.println(" templates");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Electronic");
  lcd.setCursor(0, 1);
  lcd.print("Voting Machine");
  delay(3000);
}

void loop() {
  vote_taken = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please place your");
  lcd.setCursor(0, 1);
  lcd.print("finger");
  delay(100);

  id = getFingerprintIDez();
  if (id > 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Your Voter ID");
    lcd.setCursor(0, 1);
    lcd.print(id);
    delay(2000);

    if (id == 4) {
      Serial.println("Admin ID Detected");
      sendVoteCountsToESP(); // Send vote counts to ESP32
      displayWinner();       // Display the winner
      while (1);             // Stop further operations
    }

    if (previous_voter_id != id) {
      handleVoting();
    } else {
      handleDuplicateVote();
    }
  }
}

void handleVoting() {
  do {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Give Your vote");
    lcd.setCursor(0, 1);
    lcd.print("Press Button");
    delay(500);

    previous_voter_id = id;
    buttonState1 = digitalRead(buttonPin1);
    delay(10);
    buttonState2 = digitalRead(buttonPin2);
    delay(10);
    buttonState3 = digitalRead(buttonPin3);
    delay(10);

    if (buttonState1 == HIGH) {
      party_1_count++;
      vote_taken = 1;
    } else if (buttonState2 == HIGH) {
      party_2_count++;
      vote_taken = 1;
    } else if (buttonState3 == HIGH) {
      party_3_count++;
      vote_taken = 1;
    } else {
      vote_taken = 0;
    }

    if (vote_taken == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thanks for your");
      lcd.setCursor(0, 1);
      lcd.print("vote");
      delay(200);
      digitalWrite(buzzer, HIGH);
      delay(1000);
      digitalWrite(buzzer, LOW);
      delay(1000);
    }
  } while (vote_taken == 0);
}

void handleDuplicateVote() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Duplicate Vote");
  lcd.setCursor(0, 1);
  lcd.print("Buzzer on");
  delay(2000);

  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
    delay(1000);
  }
}

void sendVoteCountsToESP() {
  String voteData = String(party_1_count) + "," + 
                    String(party_2_count) + "," + 
                    String(party_3_count) + "\n";

  espSerial.print(voteData);
  Serial.println("Vote data sent to ESP32: " + voteData);
}

void displayWinner() {
  if ((party_1_count > party_2_count) && (party_1_count > party_3_count)) {
    winner_name = "Party 1";
  } else if ((party_2_count > party_1_count) && (party_2_count > party_3_count)) {
    winner_name = "Party 2";
  } else if ((party_3_count > party_1_count) && (party_3_count > party_2_count)) {
    winner_name = "Party 3";
  } else {
    winner_name = "Tie";
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Winner Party");
  lcd.setCursor(0, 1);
  lcd.print(winner_name);
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}
