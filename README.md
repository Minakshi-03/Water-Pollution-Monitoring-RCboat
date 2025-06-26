# Water-Pollution-Monitoring-RCboat
Water Pollution Monitoring Remote Control Boat measures the pH, turbidity, temperature of the water reservoirs using different sensors and it enables to trace the location of polluted water bodies too.
This project features a remote-controlled boat powered by the ESP32, equipped with sensors to monitor water quality in real time. It measures temperature (DS18B20), pH (Gravity Analog), and turbidity, along with GPS-based location tracking. Sensor data is transmitted over Wi-Fi and visualized on a web interface, enabling remote monitoring of pollution levels. Ideal for lakes, rivers, and reservoirs, the system aids in environmental research and pollution control.
# Water Quality Monitoring System - ESP32

This project uses an ESP32 microcontroller to monitor water quality parameters like:

- Temperature using DS18B20 sensor
- pH level
- Turbidity
- GPS location using TinyGPS++
- Motor control via web interface
- WiFi-based location using Google Geolocation API

 Features
- Live web dashboard with HTML and CSS
- Motor control (start/stop)
- Visual alerts when sensor values are out of safe range

Hardware
- ESP32-WROOM
- DS18B20 (temperature sensor)
- Analog pH sensor
- Turbidity sensor
- NEO-6M GPS module
- L298 Motor Driver

Setup
1. Add your Wi-Fi credentials and Google API key in the `.ino` file
2. Upload the code to ESP32 via Arduino IDE
3. Open IP address shown in Serial Monitor to access the web interface


