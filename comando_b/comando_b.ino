#include <XBee.h>
#include <SoftwareSerial.h>

#include <MQTT.h>
#include <Ethernet.h>

byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x64, 0x51};
byte ip[] = {10, 32, 83, 220};

EthernetClient net;
MQTTClient client;

/*
   Tabela dos sensores:
   Código   | Sensor
   0        | Atuador A recuado
   1        | Atuador A avançado
*/

char code;
bool code_received = false;
bool flag1 = false;
bool flag2 = false;

unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long relay_timer = 2000;


const byte relay = 8;

// Estabelece os pinos 4 e 5 para comunicação serial do XBee.
// SoftwareSerial xbee_serial(4,5);

// Instancia objetos que seram usados na comunicação XBee
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

void setup() {
    // Define a taxa de transmissão usado pelo XBee e a o serial usado
    // xbee_serial.begin(9600);
    // xbee.setSerial(xbee_serial);

    // Inicia a comunicação serial
    Serial.begin(9600);
    xbee.setSerial(Serial);

    // Configurações para o uso do MQTT
    Ethernet.begin(mac, ip);
    //client.begin("rubens.cloud.shiftr.io", net);
    client.begin("10.32.83.224", net);
    client.onMessage(messageReceived);
    connect();

    pinMode(relay, OUTPUT);
    digitalWrite(relay, LOW);

}   

void loop() {

   client.loop();
   if (!client.connected()) {
       connect();
   }

    status_xbee();
    if (code_received) {
        // atribui a mensagem do pacote a code 
        code = rx.getData(0);
        operation_xbee();
        code_received = false;
    }

    if (flag1) {
        if ( millis() - timer1 > relay_timer) {
            digitalWrite(relay, HIGH);
            flag1 = false;
        }
    }

    if (flag2) {
        if ( millis() - timer2 > relay_timer) {
            digitalWrite(relay, LOW);
            flag2 = false;
        }
    }

}

void connect() {
    Serial.print("Conectando...");
    while (!client.connect("Atuador B", "rubens", "pYEml6K8Q321WxGB")){
        Serial.print(".");
        delay("1000");
    }
    Serial.println("\nConectado!");

    client.subscribe("/Atuador_A/#");
}

void messageReceived(String &topic, String &payload) {
    Serial.println("Entrada: " + topic + " - " + payload);
    
    if (payload == "A+") {
        flag1 = true;
        timer1 = millis();
    } else if (payload == "A-") {
        flag2 = true;
        timer2 = millis();
    }

}

void operation_xbee() {
    if (code == 0) {
        client.publish("/Atuador_B", "B-");
    } else if (code == 1) {
        client.publish("/Atuador_B", "B+");
    }
}

void status_xbee() {
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
        // Recebeu algo

        if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
            // Atribui o pacote ao objeto rx 
            xbee.getResponse().getZBRxResponse(rx);
            code_received = true;

            if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
                // O emissor recebeu um ACK
                Serial.println("Sucesso: ACK entregue");
            } else {
                // Recebemo algo mas o emissor n recebeu um ACK
                Serial.println("Falha: ACK não foi entregue.");
            }
        } else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
            xbee.getResponse().getModemStatusResponse(msr);
            // O XBee local envia respostas em certos eventos
         
            if (msr.getStatus() == ASSOCIATED) {
                Serial.println("Status : Conectado");
            } else if (msr.getStatus() == DISASSOCIATED) {
                Serial.println("Status : Desconectado");
            } else {
                Serial.println("Status : Desconhecido.");
            }
        } else {
            Serial.println("Recebeu algo, mas nada do que esperavamos.");
        }
    }
}

