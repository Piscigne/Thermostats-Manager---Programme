#include "Tools/myMqtt/myMqtt.hpp"

extern THM		ThmUnit;
extern THM_DATA	ThmData[THERMOSTAT_NBR];

extern myMqtt*	pMqtt;

WiFiClient	 espClient;
PubSubClient mqttClient(espClient);

StaticJsonDocument<512> mqttJson;

void callback(char* topic, byte* payload, unsigned int length)
{
	pMqtt->Callback(topic, payload, length);
}

void myMqtt::init()
{
	Serial.printf("MQTT Init: %s:%i\n", ThmUnit.Mqtt.Server, ThmUnit.Mqtt.Port);
	mqttClient.setServer(ThmUnit.Mqtt.Server, ThmUnit.Mqtt.Port);
	mqttClient.setCallback(callback);
	TopicOff = strlen(ThmUnit.Mqtt.Path)-1;
}

void myMqtt::setThm(char* msg, int thm)
{
	int Index = thm-1;
//Serial.printf("myMQTT setThm [%i] msg [%s]\n", Index, msg);
	deserializeJson(mqttJson, msg);

	strlcpy(ThmData[Index].Label, mqttJson[F("Label")], sizeof(ThmData[Index].Label));	//Serial.printf("Label[%i] [%s] - ", Index, ThmData[Index].Label);
	ThmData[Index].Temp	  = float(mqttJson[F("Temp")]);
	ThmData[Index].Target = float(mqttJson[F("Target")]);
	ThmData[Index].State  = bool( mqttJson[F("Actif")]);

	char Mode[8];
	strlcpy(Mode, mqttJson[F("Mode")], sizeof(Mode));									//Serial.printf("Mode [%s]\n", Mode);
	if(		!strcmp(Mode, "CONF"))	ThmData[Index].Mode = THM_MODE_CONF;
	else if(!strcmp(Mode, "ON"))	ThmData[Index].Mode = THM_MODE_ON;
	else if(!strcmp(Mode, "ECO"))	ThmData[Index].Mode = THM_MODE_ECO;
	else if(!strcmp(Mode, "HG"))	ThmData[Index].Mode = THM_MODE_HG;
	else							ThmData[Index].Mode = THM_MODE_OFF;

	ThmData[Index].Targets[THM_TARGET_CONF]	= float(mqttJson[F("Targets")][F("CONF")]);
	ThmData[Index].Targets[THM_TARGET_ON]	= float(mqttJson[F("Targets")][F("ON")]);
	ThmData[Index].Targets[THM_TARGET_ECO]	= float(mqttJson[F("Targets")][F("ECO")]);
	ThmData[Index].Targets[THM_TARGET_HG]	= float(mqttJson[F("Targets")][F("HG")]);
	ThmData[Index].Targets[THM_TARGET_OFF]	= float(mqttJson[F("Targets")][F("OFF")]);
}

void myMqtt::setTempExt(char* msg)
{
	deserializeJson(mqttJson, msg);
	ThmUnit.TempExtern = float(mqttJson[F("ExtTemp")]);
	ThmUnit.TempExtMax = float(mqttJson[F("ExtMax")]);
	ThmUnit.TempExtMin = float(mqttJson[F("ExtMin")]);
}

void myMqtt::setpresence(char* msg)
{
	deserializeJson(mqttJson, msg);
	ThmUnit.Presence  = bool(mqttJson[F("Presence")]);
}

void myMqtt::Callback(char* topic, byte* payload, unsigned int length)
{
	int TopicLen = strlen(topic)-TopicOff;
	char Msg[length]; strncpy(Msg,	 (char*)payload, length);	Msg[length]		= '\0';	// Serial.printf("myMQTT Msg[%i]: %s - %s\n", TopicLen, topic, Msg);
//Serial.printf("myMQTT topic [%S] payload [%s] length [%i] TopicLen [%i] TopivOff [%i]\n", topic, Msg, length, TopicLen, TopicOff);
	char Topic[16];	  strncpy(Topic, topic+TopicOff, TopicLen);	Topic[TopicLen] = '\0';	// Serial.printf("myMQTT Topic[%i]: %s\n",  length, Topic);

	if(!strncmp(Topic, "THM",  3))	setThm(	    Msg, (int)(Topic[3]-48));
	if(!strncmp(Topic, "EXT",  3))	setTempExt( Msg);
	if(!strncmp(Topic, "HOME", 4))	setpresence(Msg);
}

void myMqtt::Reconnect(void)
{
	while(!mqttClient.connected())
	{
		Serial.print("MQTT connection... ");
		if(mqttClient.connect("ThermostatsClientID"))
		{
			Serial.println("connected");
			Serial.printf("MQTT suscribe: %s\n", ThmUnit.Mqtt.Path);
			mqttClient.subscribe(ThmUnit.Mqtt.Path);
			mqttClient.publish("Piscigne/THMS/INFO", "THERMOSTATS connected to MQTT");
			ThmUnit.Status = STATUS_MQTT_OK;
		}
		else
		{
			Serial.printf("failed, rc=%i try again in 5 seconds\n", mqttClient.state());
			delay(5000);
		}
	}
}

void myMqtt::loop(void)
{
	Reconnect();
	mqttClient.loop();
}