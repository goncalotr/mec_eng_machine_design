#include <ESP32Servo.h>
//#include <Servo.h>

Servo motor_pan;
Servo motor_till;
int pan_pin = 32;
int till_pin = 27;

//roda 1 - esquerda
int input1 = 22;
int input2 = 23;
int analog2 = 17;
int speed1 = 0;
//roda 2 - direita
int input3 = 18;
int input4 = 19;
int analog1 = 15;
int speed2 = 0;

//map pwm
int lower_ = 50;
int maximo = 255;

int X = 0;
int Y = 0;

int vy=0;
int vx=0;
// parte do setup para comunicaçao **********************************************************
#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
  int X_1;
  int Y_1;
  int Vy;
  int Vx;  
};
struct_message myData;
esp_now_peer_info_t peerInfo;
// callback when data is sent
struct dataSend;  // Declara a estrutura dataSend
void OnDataRecv (const esp_now_recv_info_t * esp_now_info, const uint8_t *incomingData, int data_len){//(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  /*Serial.println("recebendo informacao: ");
  Serial.print("Int X: ");
  Serial.println(myData.X_1);
  Serial.print("Int y: ");
  Serial.println(myData.Y_1);
  Serial.println(); */ 
  X = myData.X_1;
  Y = myData.Y_1;
  vy = myData.Vx;
  vx = myData.Vy;
}

void setup() {
  int pos_pan = 90;
  int pos_till = 90;
  pinMode(input1, OUTPUT);
  pinMode(input2, OUTPUT);
  pinMode(input3, OUTPUT);
  pinMode(input4, OUTPUT);
  pinMode(analog1, OUTPUT);
  pinMode(analog2, OUTPUT);
  motor_pan.attach(pan_pin);
  motor_till.attach(till_pin);
  motor_pan.write(90);
  motor_till.write(90);
  motor_till.setPeriodHertz(50);    // standard 50 hz servo
  motor_pan.setPeriodHertz(50);    // standard 50 hz servo
 // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

    // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    X = 1900;
    Y = 1900;
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

}



  int pos_pan = 90;
  int pos_till = 90;
void loop() {

  //delay(500);
  Serial.print("Pan ");Serial.println(pos_pan);
  Serial.print("Till ");Serial.println(pos_till);
  //int pos_pan;
  //int pos_till;
  Serial.println("void loop");
  if (X<1000){
    Serial.print("Pan inicial");Serial.println(pos_pan);
    pos_pan = pos_pan + 5;
    if (pos_pan > 180){
      pos_pan = 180;
    }else {}
    Serial.print("Pan ");Serial.println(pos_pan);
    delay(50);
    motor_pan.write(pos_pan);
  }
  if (X>2800){
    Serial.print("Pan inicial");Serial.println(pos_pan);
    pos_pan = pos_pan - 5;
    if (pos_pan < 0){
      pos_pan = 0;
    } else {}
    Serial.print("Pan ");Serial.println(pos_pan);
    delay(50);
    motor_pan.write(pos_pan);
  }

  if (Y>2800){
    pos_till = pos_till - 5;
    if (pos_till<0){
      pos_till = 0;
    }
    Serial.print("Till ");Serial.println(pos_till);
    delay(100);
    motor_till.write(pos_till);
  }

  if (Y<1000){
    pos_till = pos_till + 5;
    if (pos_till>180){
      pos_till = 180;
    }
    Serial.print("Till ");Serial.println(pos_till);
    delay(100);
    motor_till.write(pos_till);
  }
// controlo do carro
  Serial.print("vx: ");Serial.println(vx);
  Serial.print("vy: ");Serial.println(vy);
  if (vy>2800){
    digitalWrite(input1, HIGH);
    digitalWrite(input2, LOW);
    digitalWrite(input3, HIGH);
    digitalWrite(input4, LOW);

    Serial.println("VY frente");
    if (vx>2800){
      speed1 = map(vx,2800,4095,255,50);
      speed2 = 255;
      Serial.print("speed: ");Serial.println(speed1);
      analogWrite(analog1,speed1);
      analogWrite(analog2,speed2);
                                                                             
    }
    if (vx<1200){
      speed2 = map(vx,0,1200,50,255);
      speed1 = 255;
      Serial.print("speed2: ");Serial.println(speed2);
      analogWrite(analog1,speed1);
      analogWrite(analog2,speed2);
    }
    if (vx>1200){
      if (vx<2800){
        speed1 = 255;
        speed2 = 255;
      analogWrite(analog1,speed1);
      analogWrite(analog2,speed2);
      }
    }
    delay(100);
  }
  if (vy<2800){
    if (vx>1200){
      digitalWrite(input1, LOW);
      digitalWrite(input2, LOW);
      digitalWrite(input3, LOW);
      digitalWrite(input4, LOW);   
       
      Serial.println("VY PARADO");
    }
  }
  if (vy<1200){
    digitalWrite(input1, LOW);
    digitalWrite(input2, HIGH);
    digitalWrite(input3, LOW);
    digitalWrite(input4, HIGH); 

    Serial.println("VY tras");
    if (vx>2800){
      speed1 = map(vx,2800,4095,255,50);
      speed2 = 255;
      Serial.print("speed1: ");Serial.println(speed1);
      analogWrite(analog1,speed1);
      analogWrite(analog2,speed2);
    }
    if (vx<1200){
      speed2 = map(vx,0,1200,50,255);
      speed1 = 255;
      Serial.print("speed2: ");Serial.println(speed2);
      analogWrite(analog1,speed1);
      analogWrite(analog2,speed2);
    }
    if (vx>1200){
      if (vx<2800){
        speed1 = 255;
        speed2 = 255;
      analogWrite(analog1,speed1);
      analogWrite(analog2,speed2);
      }
    }
    delay(100);
  }
}