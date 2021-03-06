# TTGO T-CALL ESP32 SMS Receive Send
This repository contains code to receive and send sms with the TTGO T-Call development board (ESP32 + SIM800L)
# Getting Started
1. Download repository
2. Open "ttgo_sms_receive_send.ino" file with the Arduino IDE.
3. Choose the development board and compile the program.
4. Open the Serial monitor at 115200 rate.
5. Send a SMS with any Text to your SIM card number from your own phone.
6. Take a look at the seriel monitor. You should see here all the content of the sms and reactions.

Congrats now you can control your TTGO with SMS's.

# Hardware requirements
1. Connect the USB port of your computer to the TTGO T-Call board
2. Place a Sim card in the ttgo device.

# Hardware Setup:
If you have TTGO T-Call the following instructions are not required:
1. Connect the SIM800L RX to Pin 17(TX) of the ESP32.
2. Connect the SIM800L TX to Pin 16(RX) of the ESP32.
3. Of case we need ground to ground.

# Connections schema
![Wiring schema of ESP32 and SIM800L](./wiring_schema.png)

# Preview:
![Screenshot of the phone sms communication ESP32 and SIM800L](./IMG_5500.PNG)
