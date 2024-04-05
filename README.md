# Velux windows - remote control

Control your Velux windows remotely from any browser on your local network.

## Requirements:
- ESP32 or equivalent
- Infrared LED
- Wifi network
- Velux wall switch with infra-red sensor
- Normal LED (optional)
- Temperature sensor (optional)

## How to run:
- Connect the infrared LED, the normal LED and the temperature sensor to your ESP32 according to the pins specified in the code.
- Change the WIFI credentials in the web_server_velux.ino file on line 9 and 10 according to your network credentials. 
- Deploy the web_server_velux.ino to your ESP32 using the Arduino IDE or equivalent.
- Place your ESP32 in front of your Velux wall switch so that the infrared LED on the ESP32 is aligned with the infrared sensor.
- Find the local IP address of your ESP32 (with router interface or network scanner, reserve the IP address for your ESP32 with DHCP for future use).
- Open the IP address in any browser, the user interface should now appear.
- Now you can control your Velux windows from any browser on your local network.
