int VRx=32;
int VRy=33;


#include <esp_now.h>
#include <WiFi.h>

int Y=0;
int X=0;

//24:dc:c3:a7:41:dc carro
//34:86:5d:aa:cf:5c controlo

uint8_t broadcastAddress_carro[] = {0x24, 0xdc, 0xc3, 0xa7, 0x41, 0xdc};

typedef struct struct_message {
  int X_1;
  int Y_1;
};
struct_message myData;
esp_now_peer_info_t peerInfo;
// callback when data is sent
struct dataSend;  // Declara a estrutura dataSend
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //char macStr[18];
  //snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
  //         mac_addr[0], mac_addr[1]);
  //Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void setup() {
  // put your setup code here, to run once:
  pinMode(VRx,INPUT);
  pinMode(VRy,INPUT);


  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress_carro, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  
  // Add peer        
  //if (esp_now_add_peer(&peerInfo) != ESP_OK){
  //  Serial.println("Failed to add peer");
  //  return;
  //}

}

void loop() {
  // put your main code here, to run repeatedly:
  Y=analogRead(VRy);
  X=analogRead(VRx);
  Serial.print("Y = ");Serial.println(Y);
  Serial.print("X = ");Serial.println(X);
  delay(80);
  myData.X_1= X;
  myData.Y_1= Y;
  esp_err_t result = esp_now_send(broadcastAddress_carro, (uint8_t *) &myData, sizeof(myData));
  //delay(1000);
}
