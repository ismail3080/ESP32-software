#define WIFI_NETWORK "My network"
#define WIFI_PASSWORD "My password"
#define WIFI_TIMEOUT 20000

void connectToNetwork() {
    Serial.print("Connecting to widi");
    WIFI.mode(WIFI_STA);
    WIFI.begin(WIFI_NETWORK, WIFI_PASSWORD);
    unsigned long start = millis();
    while (WIFI.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT) {
        Serial.print(".");
        delay(100);
    }       
        if (WIFI.status() == WL_CONNECTED) {
            Serial.println("Connected");
        } else {
            Serial.println("Failed to connect");
            Serial.println(Wifi.localIP());
        }   
    }
void setup() {
    Serial.begin(9600);
    connectToNetwork();
}
void loop() {
}