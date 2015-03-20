#include <Arduino.h>
#include <stdio.h>
#include <SuperSerial.h>


//This is around the maximum that the RS485 chip can support 
//since it has slew rate limiting
#define BAUD_RATE 100000

#define MAX489_RE 13
#define MAX489_DE 12

//A pullup on A1 will be used to indicate a master device
#define MASTER_SWITCH A1

#define MASTER_DEV_ID 1

typedef enum {MASTER, SLAVE} device_role_t;
device_role_t device_role = MASTER;

//The address of this board
byte device_id;

SuperSerial SuperSerial;


void slaveRxCallback(byte sender, byte command, byte dataLength, void *data, typeOfPacket packetType) {
	switch(command) {
		case 'S':
			{
				char chRx = ((char *)data)[0];
				Serial.print(chRx);
			} break;
	}
}


void setup()
{
	Serial.begin(38400);
	while (!Serial){}
	pinMode(MASTER_SWITCH, INPUT);
	//Check a pin to detect whether this device is a master or a slave
	if (digitalRead(MASTER_SWITCH)) {
		device_role = MASTER;
		device_id = 1;
	}
	else {
		device_role = SLAVE;
		device_id = 2;
	}

	if (device_role == SLAVE) {
		Serial.println("Slave Device");
		//The callback will be called when a packet is received.
		SuperSerial.registerCallback(slaveRxCallback);
		SuperSerial.begin(&Serial1, BAUD_RATE, device_id, MASTER_DEV_ID, MAX489_RE, MAX489_DE);

	}
	else {
		//Wait for a Serial connection
		while(!Serial){}
		Serial.print("Master Device");
		//Discard any unsolicited packets received by the master.
		SuperSerial.begin(&Serial1, BAUD_RATE, device_id, MASTER_DEV_ID, MAX489_RE, MAX489_DE);
	}



}

void loop()
{
	char ch;

	switch(device_role) {
		case MASTER:
			{
				while (Serial.available() > 0) 
				{
					ch = Serial.read(); // Read a character
					SuperSerial.send(2, 'S', 1, &ch);
				}
			} break;
		case SLAVE:
			{
				SuperSerial.process();
			} break;
	}
}

