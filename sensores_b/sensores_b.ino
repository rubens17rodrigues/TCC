#include <XBee.h>
#include <SoftwareSerial.h>

// Cria um objeto XBee e o payload que será enviado
XBee xbee = XBee();
char payload [1];


// Define o endereço do XBee recebedor
// Cria o objeto ZigBee que será enviado
// Cria um objeto para receber monitorar o status do pacote
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x40c1b208);
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// Define os pinos que serão atribuidos aos sensores
const byte sensor_0 = 2;
const byte sensor_1 = 3;

// Define uma flag para o envio do pacote
bool send = false;

void setup() {
    // Inicia o serial e o XBee
    Serial.begin(9600);
    xbee.setSerial (Serial);

    // Define os pinos dos sensores como entradas
    pinMode(sensor_0, INPUT_PULLUP);
    pinMode(sensor_1, INPUT_PULLUP);

    // Trata os sensores como interrupções sensiveis a borda de descida
    attachInterrupt(digitalPinToInterrupt(sensor_0), set_0, FALLING);
    attachInterrupt(digitalPinToInterrupt(sensor_1), set_1, FALLING);

}

void loop() {
    if (send) {
        // Envia o pacote ZigBee
        xbee.send(zbTx);
        delivery_status();
        send = false;
    }
}


void delivery_status() { 
    if (xbee.readPacket(500)) {
        // Deveria ser um pacote de status             	
        if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
             xbee.getResponse().getZBTxStatusResponse(txStatus);

            // recebemos um status de entrega
            if (txStatus.getDeliveryStatus() == SUCCESS) {
                Serial.println("\t Sucesso: \t ACK recebido");
            } else {
                // Não recebemos um ACK o outro dispositivo esta ligado?
                Serial.println("\t Erro: \t ACK não recebido.");
            }
        }   
    } else {
        // O XBee local não criou o pacote tx a tempo -- não deveria acontecer.
        Serial.println("O Xbee local esta conectado?");
    }
}

void set_0() {
    // Tratamento de debounce do botão
    static unsigned long lastInterrupt = 0;
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterrupt > 200){
        payload[0] = 0;
        send = true;
    }
    lastInterrupt = interruptTime;
}

void set_1() {
    // Tratamento de debounce do botão
    static unsigned long lastInterrupt = 0;
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterrupt > 200){
        payload[0] = 1;
        send = true;
    }
    lastInterrupt = interruptTime;
}

