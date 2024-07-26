#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define RST_PIN   9
#define SS_PIN    10
#define GREEN_LED_PIN  3
#define RED_LED_PIN 2
#define BUZZER_PIN 6
#define BUTTON_PIN 4 // Pin del botón para registrar nuevos UIDs
#define SERVO_PIN 7 // Pin del servo

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myservo1;

String users[10]; // Lista de UIDs, inicializar con capacidad para 10 UIDs
int usersSize = 0; // Número de UIDs registrados actualmente

void setup() {
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Configurar botón con resistencia pull-up interna
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  myservo1.attach(SERVO_PIN); // Inicializar el servo
  myservo1.write(0); // Posición inicial del servo (cerrado)
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    registerNewUID();
    delay(1000); // Esperar un segundo para evitar registros múltiples
  }

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  String uid = getUID();
  Serial.print("UID tag: ");
  Serial.println(uid);

  if (checkUID(uid)) {
    digitalWrite(GREEN_LED_PIN, HIGH); // Activar LED verde
    tone(BUZZER_PIN, 200, 4000); // Tono de 200Hz por 4 segundos
    myservo1.write(90); // Abrir la puerta
    delay(4000); // Mantener la puerta abierta por 4 segundos
    digitalWrite(GREEN_LED_PIN, LOW); // Apagar LED verde
    myservo1.write(0); // Cerrar la puerta
  } else {
    digitalWrite(RED_LED_PIN, HIGH); // Activar LED rojo
    tone(BUZZER_PIN, 400, 2000); // Tono de 400Hz por 2 segundos
    delay(2000); // Mantener encendido el LED rojo por 2 segundos
    digitalWrite(RED_LED_PIN, LOW); // Apagar LED rojo
  }
  
  mfrc522.PICC_HaltA();
  delay(1000);
}

String getUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += mfrc522.uid.uidByte[i] < 0x10 ? "0" : "";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  return uid;
}

bool checkUID(String uid) {
  for (int i = 0; i < usersSize; i++) {
    if (users[i] == uid) {
      return true;
    }
  }
  return false;
}

void registerNewUID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  String uid = getUID();
  Serial.print("Registrando nuevo UID: ");
  Serial.println(uid);

  // Agregar el nuevo UID a la lista de usuarios si no está ya registrado
  if (!checkUID(uid) && usersSize < 10) {
    users[usersSize] = uid;
    usersSize++;
    Serial.println("UID registrado correctamente.");
    digitalWrite(GREEN_LED_PIN, HIGH); // Indicar éxito con el LED verde
    delay(1000);
    digitalWrite(GREEN_LED_PIN, LOW);
  } else {
    Serial.println("UID ya registrado o lista llena.");
    digitalWrite(RED_LED_PIN, HIGH); // Indicar error con el LED rojo
    delay(1000);
    digitalWrite(RED_LED_PIN, LOW);
  }
  
  mfrc522.PICC_HaltA();
}

