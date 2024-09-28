#include <SPI.h>
#include <DW1000.h>

const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS = 4;

const uint16_t NETWORK_ID = 10;
const uint8_t DEVICE_ADDRESS = 8;


volatile boolean received = false;
volatile boolean error = false;

String message;

void setup() {
  Serial.begin(9600);

  DW1000.begin(PIN_IRQ, PIN_RST);
  DW1000.select(PIN_SS);

  DW1000.newConfiguration();
  DW1000.setDefaults();
  DW1000.setDeviceAddress(DEVICE_ADDRESS);
  DW1000.setNetworkId(NETWORK_ID);

  // DW1000.setPreambleLength(DW1000.TX_PREAMBLE_LEN_256);
  // DW1000.setDataRate(DW1000.TRX_RATE_6800KBPS);
  // DW1000.setPulseFrequency(DW1000.TX_PULSE_FREQ_64MHZ);

  DW1000.commitConfiguration();
  DW1000.attachReceivedHandler(handleReceive);
  DW1000.attachReceiveFailedHandler(handleError);
  DW1000.attachErrorHandler(handleError);
  receiver();
}

void handleReceive() {
  received = true;
}

void handleError() {
  error = true;
}

void receiver() {
  DW1000.newReceive();
  DW1000.receivePermanently(true);
  DW1000.startReceive();
}

DW1000Time lastTimeDiff;

uint64_t maksTimestamp = 1099511627775;
DW1000Time maksTimestampTime = DW1000Time(static_cast<int64_t>(maksTimestamp));

DW1000Time previousTime;

DW1000Time receivePreviousTime;

DW1000Time synchronizedTime;
void loop() {

  if (received) {
    Serial.println("######  START  #########");

    DW1000Time receiveTimeValue;
    DW1000.getReceiveTimestamp(receiveTimeValue);
    DW1000.getData(message);

    DW1000Time packNum(atoll(message.c_str()));

    DW1000Time timeDifference = packNum - receivePreviousTime;
    
    receivePreviousTime = packNum;
    Serial.print("timeDifference : ");
    Serial.println(timeDifference.getAsMicroSeconds());

    Serial.print("Sender timestamp (nanoseconds): ");
    Serial.println(packNum);

    Serial.print("Receive timestamp (nanoseconds): ");
    Serial.println(receiveTimeValue);

    DW1000Time timeDiff;
    if (packNum.getTimestamp() >= receiveTimeValue.getTimestamp()) {
      timeDiff = DW1000Time(packNum.getTimestamp() - receiveTimeValue.getTimestamp());
    } else {
      timeDiff = DW1000Time(static_cast<int64_t>((maksTimestampTime.getTimestamp() - receiveTimeValue.getTimestamp()) + packNum.getTimestamp()));
    }

    DW1000Time timeDiffBetweenRuns;
    if (timeDiff.getTimestamp() >= lastTimeDiff.getTimestamp()) {
      timeDiffBetweenRuns = DW1000Time(static_cast<int64_t>(timeDiff.getTimestamp() - lastTimeDiff.getTimestamp()));
    } else {
      timeDiffBetweenRuns = DW1000Time(static_cast<int64_t>(lastTimeDiff.getTimestamp() - timeDiff.getTimestamp()));
    }

    Serial.print("Last diff: ");
    Serial.println(lastTimeDiff);

    lastTimeDiff = timeDiff;

    Serial.print("Time diff Master and Slave: ");
    Serial.println(timeDiff);

    Serial.print("Time diff between runs: ");
    Serial.println(timeDiffBetweenRuns);
    received = false;
  }
}

