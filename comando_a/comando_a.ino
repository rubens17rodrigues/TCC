#include <XBee.h>
#include <SoftwareSerial.h>

/*
   Tabela dos sensores:
   Código   | Sensor
   0        | Atuador A recuado
   1        | Atuador A avançado
*/

char code;
bool code_received = false;

// Estabelece os pinos 4 e 5 para comunicação serial do XBee.
SoftwareSerial xbee_serial(4,5);

// Instancia objetos que seram usados na comunicação XBee
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

void setup() {
    // Define a taxa de transmissão usado pelo XBee e a o serial usado
    xbee_serial.begin(9600);
    xbee.setSerial(xbee_serial);

    // Inicia a comunicação serial
    Serial.begin(9600);

    pinMode(7, OUTPUT);
}

void loop() {
    status_xbee();
    if (code_received) {
        // atribui a mensagem do pacote a code 
        code = rx.getData(0);

        operation ();
        code_received = false;
    }
}

void operation() {
    if (code == 0) {
        digitalWrite(7, LOW);
        Serial.println("publicar mqtt: B-");
    } else if (code == 1) {
        // B+
        digitalWrite(7, HIGH);
        Serial.println("publicar mqtt: B+");
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

