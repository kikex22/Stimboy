// MK13 niveles 2 3 4 5 ems 
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  // Dirección I2C de la pantalla

#define PIN_Y  33
#define BUTTON_PIN 26
#define Verde 27
#define ledrojo 12
#define ledblanco 13
bool Startmenu  = true;

int menuIndex = 0; 
bool inSubMenu = false; // Indica si estamos dentro del submenu
bool inSubSubMenu = false; // Indica si estamos dentro del sub-submenú
int subMenuIndex = 0; // Índice del submenu
bool buttonPressed = false; // Índice del menú seleccionado
bool botonverde = false;
uint8_t broadcastAddress1[] = {0xC8, 0x2E, 0x18, 0xF8, 0x31, 0xD0}; 
uint8_t broadcastAddress2[]={0xA0, 0xDD, 0x6C, 0x0F, 0x14, 0xB4}; //A0:DD:6C:0F:14:B4 E4:65:B8:7A:B1:C0 10:06:1C:F6:25:B0 A0:DD:6C:0F:14:B4
typedef struct struct_message {
  float hi;
  float mpu; 
  float ytest; 
  char mensaje[32];
  bool pwmstate;   // Estado del PWM (encendido/apagado)
  int value;       // Valor del ciclo de trabajo (0-255)
  int duration;    // Duración en milisegundos
  int frequency;   // Frecuencia PWM en Hz 
  int prueba;
  int ton_ems;
  int freq_ems;
  bool emsState ;
  bool tensState; //estado de tens on/off
  int ton_tens;
  int freq_tens;
 // int prueba2;  // Frecuencia PWM en Hz
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Wire.begin();
  lcd.init();             // Cambiado de lcd.init() a lcd.begin()
  lcd.backlight();
  lcd.clear();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(Verde, INPUT_PULLUP);
  pinMode(ledrojo, OUTPUT);
  pinMode(ledblanco, OUTPUT);
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

 esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Configuración del peer que recibe datos
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);  // Dirección del ESP32 que recibe los datos
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add first peer");
    return;
  } else {
    Serial.println("Success: Added first peer");
  } 

  // Configuración del peer que envía datos
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);  // Dirección del ESP32 que envía los datos

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add second peer");
    return;
  } else {
    Serial.println("Success: Added second peer");
  }

}

void loop() {
 // myData.prueba2= 200;
  
  int yVal = analogRead(PIN_Y);
  int mappedValue = mapJoystickValue(yVal);

  if (Startmenu ) {
    //lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Bienvenido al");
    lcd.setCursor(5, 1);
    //lcd.clear();
    lcd.print(" STIMBOY    ");
    lcd.setCursor(0,3);
    lcd.print("Presione boton verde");

    // Espera a que se presione el botón para ingresar al menú principal
    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(200); // Debounce
      Startmenu = false; // Oculta el mensaje de bienvenida
    }
    return; // Salir del loop hasta que se presione el botón
  }


  if (mappedValue >= 220) { 
    if (!inSubMenu && !inSubSubMenu) {
      menuIndex--;
      if (menuIndex < 0) menuIndex = 2;
    } else if (inSubMenu && !inSubSubMenu) {
      subMenuIndex--;
      if (subMenuIndex < 0) {
        if (menuIndex == 1) subMenuIndex = 4; // Ajustar para TENS con nivel 4
        if (menuIndex == 0) subMenuIndex= 4; //Ajuste para EMS
        else subMenuIndex = 4; // Ajustar para otros menús
      }
    }
    delay(250); 
  } else if (mappedValue <= 60) { 
    if (!inSubMenu && !inSubSubMenu) {
      menuIndex++;
      if (menuIndex > 2) menuIndex = 0;
    } else if (inSubMenu && !inSubSubMenu) {
      subMenuIndex++;
      if (menuIndex == 0) {  // EMS
        if (subMenuIndex > 4) subMenuIndex = 0; // Ajustar para EMS con nivel 4
      } else if (menuIndex == 1) {  // TENS
        if (subMenuIndex > 4) subMenuIndex = 0; // Ajustar para TENS con nivel 4
      } else {
        if (subMenuIndex > 2) subMenuIndex = 0; // Ajustar para otros menús
      }
    }
    delay(250); 
  }

  if (digitalRead(BUTTON_PIN) == LOW) {
    buttonPressed = true;
  } else {
    buttonPressed = false;
  }
   
  if (digitalRead(Verde) == LOW) {
    botonverde = true;
  } else {
    botonverde = false;
  }
   
  if (buttonPressed) {
    if (!inSubMenu && !inSubSubMenu) {
      inSubMenu = true;
    } else if (inSubMenu && !inSubSubMenu) {
      if (menuIndex == 0 && subMenuIndex == 0){ // Nivel 1 del EMS
        inSubSubMenu = true;
        myData.freq_ems = 3.33;
        myData.ton_ems= 150;
        myData.emsState= true;
        esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));


     }else if (menuIndex==0 && subMenuIndex ==1) { //Nivel 2 del EMS
        inSubSubMenu =true;
        // falta ifo de my data del ems
        myData.freq_ems = 1;
        myData.ton_ems= 500;
        myData.emsState= true;
        esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

      }else if (menuIndex==0 && subMenuIndex ==2) { //Nivel 3 del EMS
        inSubSubMenu =true;
        // falta ifo de my data del ems
        myData.freq_ems = 5;
        myData.ton_ems= 100;
        myData.emsState= true;
        esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));


      }  else if (menuIndex==0 && subMenuIndex ==3) { //Nivel 4 del EMS
        inSubSubMenu =true;
        // falta ifo de my data del ems
        myData.freq_ems = 10;
        myData.ton_ems= 50;
        myData.emsState= true;
        esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));


      } else if (menuIndex==0 && subMenuIndex ==4) { //Nivel 5 del EMS
        inSubSubMenu =true;
        // falta ifo de my data del ems
        myData.freq_ems = 50;
        myData.ton_ems= 5;
        myData.tensState= true;
        esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));


      } 
      if (menuIndex == 1 && subMenuIndex == 0) { // Si estamos en el Nivel 1 del menú TENS
        inSubSubMenu = true;
       myData.freq_tens = 20;
        myData.ton_tens= 25;
        myData.emsState= true;
       esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));
    } else if (menuIndex == 1 && subMenuIndex == 1) { // Si estamos en el Nivel 2 del menú TENS
        inSubSubMenu = true;
        myData.pwmstate = true;
        myData.frequency = 30;    // Nueva frecuencia para Nivel 2
        myData.value = 250;       // Nuevo ciclo de trabajo para Nivel 2
        myData.duration = 3000;  // Nueva duración para Nivel 2
        esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));
    } else if (menuIndex == 1 && subMenuIndex == 2) { // Si estamos en el Nivel 3 del menú TENS
        inSubSubMenu = true;
        myData.pwmstate = true;
        myData.frequency = 50;    // Nueva frecuencia para Nivel 2
        myData.value = 250;       // Nuevo ciclo de trabajo para Nivel 2
        myData.duration = 4000;  // Nueva duración para Nivel 2
        esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));
    } else if (menuIndex == 1 && subMenuIndex == 3 ) { // nivel 4 tens
       inSubSubMenu = true ; 
       myData.pwmstate = true; 
       myData.frequency = 80;
       myData.value = 250;
       myData.duration = 10000;
       esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));
    }else if (menuIndex == 1 && subMenuIndex == 4 ) { // nivel 5 tens
       inSubSubMenu = true ; 
       myData.pwmstate = true; 
       myData.frequency = 120;
       myData.value = 250;
       myData.duration = 20000;
       esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));}

      


    }
    delay(200); // Pausa para evitar múltiples selecciones rápidas
    
  } else if (botonverde) {
    if (inSubSubMenu) {
      myData.pwmstate = false;
      esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));
      inSubSubMenu = false;
      inSubMenu = true; // Volver al submenú
    } else if (inSubMenu) {
      inSubMenu = false; // Volver al menú principal
    }
    delay(250); // Pausa para evitar múltiples selecciones rápidas
  }

  // Mostrar el menú en la pantalla LCD
  lcd.clear();
  
  if (!inSubMenu && !inSubSubMenu) {
    lcd.setCursor(0, 0);
    lcd.print("Seleccione:");

    switch (menuIndex) {
      case 0:
        lcd.setCursor(0, 1);
        lcd.print("> EMS  ");
        break;
      case 1:
        lcd.setCursor(0, 1);
        lcd.print("> TENS  ");
        break;
      case 2:
        lcd.setCursor(0, 1);
        lcd.print("> MEDIR ANGULO  ");
        break;
    }
  } else if (inSubMenu && !inSubSubMenu) {
    lcd.setCursor(0, 0);
    lcd.print("Seleccione:");
    
    switch (menuIndex) {
      case 0: // Menu EMS
      switch (subMenuIndex) {
        case 0: 
         lcd.setCursor(0, 1);
          lcd.print("> Nivel 1");
          lcd.setCursor(0, 2);
          lcd.print("  Nivel 2");
          lcd.setCursor(0, 3);
          lcd.print("  Nivel 3");
          break;
        case 1:
            lcd.setCursor(0, 1);
            lcd.print("  Nivel 1");
            lcd.setCursor(0, 2);
            lcd.print("> Nivel 2");
            lcd.setCursor(0, 3);
            lcd.print("  Nivel 3");
            break;
          case 2:
            lcd.setCursor(0, 1);
            lcd.print("  Nivel 1");
            lcd.setCursor(0, 2);
            lcd.print("  Nivel 2");
            lcd.setCursor(0, 3);
            lcd.print("> Nivel 3");
            break;
          case 3:
            lcd.setCursor(0, 1);
            lcd.print("  Nivel 2");
            lcd.setCursor(0, 2);
            lcd.print("  Nivel 3");
            lcd.setCursor(0, 3);
            lcd.print("> Nivel 4");
            break;
          case 4:
            lcd.setCursor(0, 1);
            lcd.print("  Nivel 3");
            lcd.setCursor(0, 2);
            lcd.print("  Nivel 4");
            lcd.setCursor(0, 3);
            lcd.print("> Nivel 5");
            break;
          default:
            lcd.setCursor(4, 1);
            lcd.print("  Nivel 4");
            lcd.setCursor(0, 2);
            lcd.print("  Nivel 5");
            lcd.setCursor(0, 3);
            lcd.print("  Nivel 6");
            break;
        }
        break;
      case 1: // Menu TENS
        switch (subMenuIndex) {
          case 0:
            lcd.setCursor(0, 1);
            lcd.print("> Nivel 1");
            lcd.setCursor(0, 2);
            lcd.print("  Nivel 2");
            lcd.setCursor(0, 3);
            lcd.print("  Nivel 3");
            break;
          case 1:
            lcd.setCursor(0, 1);
            lcd.print("  Nivel 1");
            lcd.setCursor(0, 2);
            lcd.print("> Nivel 2");
            lcd.setCursor(0, 3);
            lcd.print("  Nivel 3");
            break;
          case 2:
            lcd.setCursor(0, 1);
            lcd.print("  Nivel 1");
            lcd.setCursor(0, 2);
            lcd.print("  Nivel 2");
            lcd.setCursor(0, 3);
            lcd.print("> Nivel 3");
            break;
          case 3:
            lcd.setCursor(0, 1);
            lcd.print("  Nivel 2");
            lcd.setCursor(0, 2);
            lcd.print("  Nivel 3");
            lcd.setCursor(0, 3);
            lcd.print("> Nivel 4");
            break;
          case 4:
            lcd.setCursor(0, 1);
            lcd.print("  Nivel 3");
            lcd.setCursor(0, 2);
            lcd.print("  Nivel 4");
            lcd.setCursor(0, 3);
            lcd.print("> Nivel 5");
            break;
          default:
            lcd.setCursor(4, 1);
            lcd.print("  Nivel 4");
            lcd.setCursor(0, 2);
            lcd.print("  Nivel 5");
            lcd.setCursor(0, 3);
            lcd.print("  Nivel 6");
            break;
        }
        break;
      case 2: // Menu Medir Ángulo
        lcd.setCursor(0, 1);
        lcd.print("Su angulo es: ");
        lcd.setCursor(0, 2);
        lcd.print(myData.mpu);
        lcd.setCursor(6, 2);
        lcd.print((char)223); // Símbolo de grado
        break;
    }
  } if (inSubSubMenu) {
  if (menuIndex == 1 && subMenuIndex == 0) {  // Nivel 1
    lcd.setCursor(0, 0);
    lcd.print("Frec: 5 hz");
    lcd.setCursor(0,1);
    lcd.print("Pulsos: 300 ms");

    // Limpiar el struct antes de asignar nuevos valores
    memset(&myData, 0, sizeof(myData));

    myData.pwmstate = true;
    myData.frequency = 20;
    myData.value = 250;    // 50% de ciclo de trabajo
    myData.duration = 1000; // 10 segundos de duración

    // Enviar comandos para encender el TENS en el nivel 1
    esp_err_t result = esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK) {
      Serial.println("TENS ACTIVO - Nivel 1");
    } else {
      Serial.println("TENS NO ACTIVO - Nivel 1");
    }

  } else if (menuIndex == 1 && subMenuIndex == 1) {  // Nivel 2
    lcd.setCursor(0, 0);
    lcd.print("Frec: 20 hz");
    lcd.setCursor(0,1);
    lcd.print("Pulsos: 1000 ms");

    // Limpiar el struct antes de asignar nuevos valores
    memset(&myData, 0, sizeof(myData));

    myData.pwmstate = true;
    myData.frequency = 10;  // Cambiar la frecuencia para el nivel 2
    myData.value = 250;     // Cambiar el ciclo de trabajo para el nivel 2
    myData.duration = 700; // Cambiar la duración para el nivel 2

    // Enviar comandos para encender el TENS en el nivel 2
    esp_err_t result = esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK) {
      Serial.println("TENS ACTIVO - Nivel 2");
    } else {
      Serial.println("TENS NO ACTIVO - Nivel 2");
    }
  }  else if (menuIndex == 1 && subMenuIndex == 2) {  // Nivel 3
    lcd.setCursor(0, 0);
    lcd.print("Frec: 15 hz");
    lcd.setCursor(0,1);
    lcd.print("Pulsos: 1000 ms");

    // Limpiar el struct antes de asignar nuevos valores
    memset(&myData, 0, sizeof(myData));

    myData.pwmstate = true;
    myData.frequency = 15;  // Cambiar la frecuencia para el nivel 3
    myData.value = 200;     // Cambiar el ciclo de trabajo para el nivel 3
    myData.duration = 1000; // Cambiar la duración para el nivel 3

    // Enviar comandos para encender el TENS en el nivel 3
    esp_err_t result = esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK) {
      Serial.println("TENS ACTIVO - Nivel 3");
    } else {
      Serial.println("TENS NO ACTIVO - Nivel 3");
    }
   } else if (menuIndex == 1 && subMenuIndex == 3) {  // Nivel 4
    lcd.setCursor(0, 0);
    lcd.print("Frec: 80 hz");
    lcd.setCursor(0,2);
    lcd.print("Pulsos: 10000 ms");

    // Limpiar el struct antes de asignar nuevos valores
    memset(&myData, 0, sizeof(myData));

    myData.pwmstate = true;
    myData.frequency = 80;  // Cambiar la frecuencia para el nivel 4
    myData.value = 250;     // Cambiar el ciclo de trabajo para el nivel 4
    myData.duration = 10000; // Cambiar la duración para el nivel 4

    // Enviar comandos para encender el TENS en el nivel 3
    esp_err_t result = esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK) {
      Serial.println("TENS ACTIVO - Nivel 4");
    } else {
      Serial.println("TENS NO ACTIVO - Nivel 4");
    }
   } else if (menuIndex == 1 && subMenuIndex == 4) {  // Nivel 5
    lcd.setCursor(0, 4);
    lcd.print("Frec: 120 hz");
    lcd.setCursor(0,0);
    lcd.print("Dur:20 s");

    // Limpiar el struct antes de asignar nuevos valores
    memset(&myData, 0, sizeof(myData));

    myData.pwmstate = true;
    myData.frequency = 120;  // Cambiar la frecuencia para el nivel 5
    myData.value = 255;     // Cambiar el ciclo de trabajo para el nivel 5
    myData.duration = 20000; // Cambiar la duración para el nivel 5

    // Enviar comandos para encender el TENS en el nivel 5
    esp_err_t result = esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK) {
      Serial.println("TENS ACTIVO - Nivel 5");
    } else {
      Serial.println("TENS NO ACTIVO - Nivel 5");
    }
   } else if (menuIndex == 0 && subMenuIndex == 0) {  // Nivel 1
    lcd.setCursor(0, 0);
    lcd.print("Frec: 3hz");
    lcd.setCursor(0,1);
    lcd.print("Pulsos: 150ms");
    memset(&myData, 0, sizeof(myData));
    myData.freq_ems = 3.33;
    myData.ton_ems = 150;
    myData.emsState= true; 
    esp_err_t result = esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

   } else if (menuIndex == 0 && subMenuIndex == 1) {  // Nivel 2
    lcd.setCursor(0, 0);
    lcd.print("Frec: 1 hz");
    lcd.setCursor(0,1);
    lcd.print("Pulsos: 500 ms");
    memset(&myData, 0, sizeof(myData));
    myData.freq_ems = 1;
    myData.ton_ems= 500;
    myData.emsState= true;
    esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

  } else if (menuIndex == 0 && subMenuIndex ==2 ) { // nivel 3
    lcd.setCursor(0, 0);
    lcd.print("Frec: 5 hz");
    lcd.setCursor(0,1);
    lcd.print("Pulsos: 100 ms");
    memset(&myData, 0, sizeof(myData));
    myData.freq_ems = 5;
    myData.ton_ems= 100;
    myData.emsState= true;
    esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

  }else if (menuIndex == 0 && subMenuIndex ==3 ) { // nivel 4
    lcd.setCursor(0, 0);
    lcd.print("Frec: 10 hz");
    lcd.setCursor(0,1);
    lcd.print("Pulsos: 50 ms");
    memset(&myData, 0, sizeof(myData));
    myData.freq_ems = 10;
    myData.ton_ems= 50;
    myData.emsState= true;
    esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));

  }else if (menuIndex == 0 && subMenuIndex ==4 ) { // nivel 5
    lcd.setCursor(0, 0);
    lcd.print("Frec: 20 hz");
    lcd.setCursor(0,1);
    lcd.print("Pulsos: 25 ms");
    memset(&myData, 0, sizeof(myData));
    myData.freq_ems = 20;
    myData.ton_ems= 25;
    myData.emsState= true;
    esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));
  }
  delay(500);
}
 

 


  esp_err_t result = esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData));
  //esp_err_t result3 = esp_now_send(broadcastAddress2, (uint8_t *)&myData, sizeof(myData));
  
   
  //if (result == ESP_OK) {
 //   Serial.println("Sent with success");
  //} else {
 //   Serial.println("Error sending the data");
 // }

 // myData.hi = 100;
 // Serial.print(myData.mpu);

  delay(100);
}

int mapJoystickValue(int value) {
  if (value >= 2200) {
    value = map(value, 2200, 4095, 127, 254);
  } else if (value <= 1800) {
    value = map(value, 1800, 0, 127, 0);  
  } else {
    value = 127;
  }

  return value;
}