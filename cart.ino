#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"


#define TX_PIN D4 //6 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN D3 //5 // Arduino receive   GREEN WIRE   labeled TX on printer


SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor


#define SS_PIN D8
#define RST_PIN D0






MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);


bool shampooAdded = false;
bool cantonAdded = false;
bool wipesAdded = false;
bool noodlesAdded = false;
bool biscuitAdded = false;
bool soapAdded = false;
// Add similar boolean variables for other products...


struct ScannedItem {
  String name;
  float price;
};


ScannedItem scannedItems[10]; // Assuming a maximum of 10 items can be scanned by the admin
int itemCount = 0;


float totalAmount = 0.0;


void setup() {
  Serial.begin(9600);
  mySerial.begin(9600); // Initialize the thermal printer's serial communication


  // Initialization of SPI, RFID, LCD, and other setup code...


  printer.begin();


  // Your existing initialization code...
  SPI.begin();
  mfrc522.PCD_Init();
   lcd.init();
    lcd.backlight();
  // Display the initial message on the LCD
  if (!mfrc522.PCD_PerformSelfTest()) {
    Serial.println("RFID connection failed");
    lcd.setCursor(0, 0);
    lcd.print("Closed");
  } else {
    Serial.println("RFID connected and working");
    lcd.setCursor(0, 0);
    lcd.print("Happy Shopping");
  }
  delay(2000);
  lcd.clear();
  displayTotalAmount();
}


void printReceipt() {
  float total = 0.0;
  printer.setSize('L');
  printer.println("Admin Receipt");
  printer.feed(1);


  for (int i = 0; i < itemCount; ++i) {
    // Only print items that have positive prices and have not been removed
    if (scannedItems[i].price > 0) {
      printer.setSize('M');
      printer.println(scannedItems[i].name);
      printer.setSize('S');
      printer.println("Price: " + String(scannedItems[i].price));
      total += scannedItems[i].price;
      printer.feed(1);
    }
  }


  printer.setSize('L');
  printer.println("Total: " + String(total));
  printer.feed(4); // Adjust as needed for spacing
  printer.sleep(); // Enter low-power mode
}


void displayTotalAmount() {
  lcd.setCursor(0, 1);
  lcd.print("Total: " + String(totalAmount));
}


void loop() {
  displayTotalAmount();


  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String cardUID = "";


    for (byte i = 0; i < mfrc522.uid.size; i++) {
      cardUID += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
    }


    Serial.print("Card UID: ");
    Serial.println(cardUID);


    // Admin card logic for printing
    if (cardUID.equals("2195a626")) {
      Serial.println("Admin card detected. Printing receipt.");
      printReceipt();
    } else if (cardUID.equals("61611626") || cardUID.equals("db49f7f6")) {
      handleProduct("Shampoo", 10.00, shampooAdded);
    } else if (cardUID.equals("d37e151e") || cardUID.equals("fbe27ff3")) {
      handleProduct("Canton", 15.00, cantonAdded);
    } else if (cardUID.equals("b1a8995e")|| cardUID.equals("616c846b")) {
      handleProduct("Soap", 25.00, soapAdded);
    } else if (cardUID.equals("f19f8a6b")|| cardUID.equals("5bf083f3")) {
      handleProduct("Noodles", 15.00, noodlesAdded);
    } else if (cardUID.equals("eb1176f3")|| cardUID.equals("f1ff655e")) {
      handleProduct("Wipes", 30.00, wipesAdded);
    } else if (cardUID.equals("91755b5e")|| cardUID.equals("8b1c85f3")) {
      handleProduct("Biscuit", 15.00, biscuitAdded);
    }
    // Add similar logic for other products...


    // Your existing LED, buzzer, and RFID handling code...
    Serial.println("Processing RFID card...");
   
    delay(500);
   


    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(3000);
    lcd.clear();
    displayTotalAmount();
  }
}


void handleProduct(String productName, float productPrice, bool &productAdded) {
  // Check if the product is already in the list
  for (int i = 0; i < itemCount; ++i) {
    if (scannedItems[i].name.equals(productName)) {
      // Product is already in the list
      if (scannedItems[i].price > 0 && !productAdded) {
        // Product is present and needs to be removed
        Serial.println(productName + " removed.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(productName + " removed");
        totalAmount -= productPrice;
        scannedItems[i].price = -productPrice; // Mark as removed
        return;
      } else if (scannedItems[i].price < 0 && productAdded) {
        // Product was removed and needs to be added again
        Serial.println(productName + " added.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(productName + " added");
        scannedItems[i].price = productPrice; // Add it again
        totalAmount += productPrice;
        return;
      }
    }
  }


  // If the product is not found in the list, add it
  Serial.println(productName + " added.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(productName + " added");
  scannedItems[itemCount] = {productName, productAdded ? -productPrice : productPrice};
  totalAmount += productAdded ? -productPrice : productPrice;
  itemCount++;
}
