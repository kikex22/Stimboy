// este busca implementar la tens de la misma forma que la ems pero con trenes de pulsos etc
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>

// Definir el pin de salida PWM
#define PWM_PIN  25  // Pin donde se generará la señal PWM
unsigned long t;
unsigned int tOn, tOff;

// Valores fijos para el modo EMS (en milisegundos)
/////const unsigned int FIXED_TON_EMS = 150;  // Duración del pulso en milisegundos para EMS
//const unsigned int FIXED_FREQ_EMS = 3.33;  // Frecuencia en Hz para EMS
// Definición de los pines
const int SPOS = 32;  // Pin de salida positivo
const int SNEG = 33;  // Pin de salida negativo

typedef struct struct_message {
  float hi;
  float mpu; 
  float ytest; 
  char mensaje[32];
  bool pwmstate;   // Estado del PWM (encendido/apagado)
  int value;       // Valor del ciclo de trabajo (0-255)
  int duration;    // Duración en milisegundos
  int frequency;   // Frecuencia PWM en Hz 
  int prueba;  // Frecuencia PWM en Hz 
  int ton_ems;
  int freq_ems;
  bool emsState ;
  bool tensState; //estado de tens on/off
  int ton_tens;
  int freq_tens;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

const int pwmChannel = 0;        // Canal PWM
int pwmFrequency = 100;          // Frecuencia PWM en Hz (se ajustará desde el transmisor)
const int pwmResolution = 8;     // Resolución PWM (8 bits -> 0-255)

bool pwmState = false;           // Estado del PWM (encendido/apagado)
int pwmValue = 0;                // Ciclo de trabajo PWM
unsigned long pwmDuration = 0;   // Duración en milisegundos
bool startTENS = false;          // Bandera para reiniciar el TENS
bool emsState = false;
bool tensState= false;           // Estado del EMS (encendido/apagado)
// Variables de tiempo para los pulsos
unsigned long previousMillis = 0;
unsigned long startTime = 0;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println("Datos recibidos:");
  Serial.print("Prueba: "); Serial.println(myData.prueba);
  Serial.print("Mensaje: "); Serial.println(myData.mensaje);
  Serial.print("Estado PWM: "); Serial.println(myData.pwmstate);
  Serial.print("Ciclo de trabajo: "); Serial.println(myData.value);
  Serial.print("Duración: "); Serial.println(myData.duration);
  Serial.print("Frecuencia: "); Serial.println(myData.frequency);
  Serial.print("Estado EMS: "); Serial.println(myData.emsState);
  Serial.print("freq ems "); Serial.println(myData.freq_ems);
  Serial.print("ton "); Serial.println(myData.ton_ems);
  

  pwmState = myData.pwmstate;
  pwmValue = myData.value;
  pwmDuration = myData.duration;
  pwmFrequency = myData.frequency;
  emsState = myData.emsState;  //  Estado del Ems (encendido/apagado)
  tensState=myData.tensState ;       // Estado del Tens (encendido/apagado)
  //free = myData.freq_ems;
 // ton= myData.ton_ems;
  ledcSetup(pwmChannel, pwmFrequency, pwmResolution);
  ledcAttachPin(PWM_PIN, pwmChannel);

  if (pwmState) {
    ledcWrite(pwmChannel, pwmValue); // Encender el PWM con el ciclo de trabajo recibido
    startTime = millis(); // Guardar el tiempo de inicio
  } else {
    // Apagar PWM
    ledcWrite(pwmChannel, 0);
  }
   if (emsState) {
    // Si EMS está activado, generar pulsos en los pines SPOS y SNEG
    tOn = myData.ton_ems ;
    tOff = (1000 /  myData.freq_ems) - (2 * tOn);
  }
  if (tensState){
  // Si EMS está activado, generar pulsos en los pines SPOS y SNEG
    tOn = myData.ton_tens ;
    tOff = (1000 /  myData.freq_tens) - (2 * tOn);
  }
}

void setup() {
   // Definición de entradas y salidas
  pinMode(SPOS, OUTPUT);
  pinMode(SNEG, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  ledcSetup(pwmChannel, pwmFrequency, pwmResolution);
  ledcAttachPin(PWM_PIN, pwmChannel);
}

void loop() {
  // Controlar el EMS basado en el estado emsState
  if (emsState) {
    t = millis();
    while ((millis() - t) < 60000) { // Generar pulsos EMS por 60 segundos
      pulsos(tOn, tOff);
    }
    delay(10000); // Espera de 10 segundos entre los ciclos EMS
  }
  if (tensState) {
    t= millis();
    while ((millis()-t)< 4000)
    {pulsos(tOn, tOff);}
    delay (10000);
  }

  // Controlar el PWM basado en el estado y duración
  if (pwmState && (millis() - startTime) >= pwmDuration) {
    // Apagar PWM después de la duración
    ledcWrite(pwmChannel, 0);
    pwmState = false;
  }
   // Reiniciar el TENS si se ha vuelto a activar la bandera startTENS
  if (startTENS && !pwmState) {
    ledcWrite(pwmChannel, pwmValue);
    startTime = millis(); // Reiniciar el tiempo de inicio
    pwmState = true;
    startTENS = false; // Desactivar la bandera después de reiniciar
  }
}
void pulsos(unsigned int tOn, unsigned int tOff) { // función de pulsos
  digitalWrite(SPOS, HIGH); // secuencia de pulsos
  delay(tOn);
  digitalWrite(SPOS, LOW);
  digitalWrite(SNEG, HIGH);
  delay(tOn);
  digitalWrite(SNEG, LOW);
  delay(tOff);
}
