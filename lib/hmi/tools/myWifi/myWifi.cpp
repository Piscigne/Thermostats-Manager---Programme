#include "tools/myWifi/myWifi.hpp"

extern THM	  ThmUnit;
extern myDio* pDio;

myWifi::myWifi()
{
}

void myWifi::init()
{
	if(ThmUnit.Net.Static)
	{
		IPAddress IP;	IP.fromString(  ThmUnit.Net.Address);
		IPAddress MASK;	MASK.fromString(ThmUnit.Net.Mask);
		IPAddress GATE;	GATE.fromString(ThmUnit.Net.Gateway);
		IPAddress DNS1;	DNS1.fromString(ThmUnit.Net.Dns1);
		IPAddress DNS2;	DNS2.fromString(ThmUnit.Net.Dns2);
		WiFi.config(IP, GATE, MASK, DNS1, DNS2);
	}
	WiFi.begin(ThmUnit.Wifi.Ssid, ThmUnit.Wifi.Pass);
	Serial.printf("\nConnecting to %s (%s)\n", ThmUnit.Wifi.Ssid, ThmUnit.Wifi.Pass);
	while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(100); } Serial.println("");
	Serial.printf("WIFI Connected to %s IP: %s\n", getSsid(), getIp());

	ThmUnit.Status = STATUS_WIFI_OK; pDio->updateStatus();
}

const char* myWifi::getIp()
{
	return WiFi.localIP().toString().c_str();
}

const char* myWifi::getSsid()
{
	return ThmUnit.Wifi.Ssid;
}

int8_t myWifi::getRssi()
{
	int Rssi = WiFi.RSSI();
	if(		Rssi == WIFI_RSSI_NULL)	ThmUnit.Wifi.State = WIFI_STATE_STOP;
	else if(Rssi <= WIFI_RSSI_NULL)	ThmUnit.Wifi.State = WIFI_RSSI4;
	else if(Rssi <= WIFI_RSSI_4)	ThmUnit.Wifi.State = WIFI_RSSI4;
	else if(Rssi <= WIFI_RSSI_3)	ThmUnit.Wifi.State = WIFI_RSSI3;
	else if(Rssi <= WIFI_RSSI_2)	ThmUnit.Wifi.State = WIFI_RSSI2;
	else if(Rssi <= WIFI_RSSI_1)	ThmUnit.Wifi.State = WIFI_RSSI1;
	else if(Rssi <= WIFI_RSSI_0)	ThmUnit.Wifi.State = WIFI_RSSI0;
	else							ThmUnit.Wifi.State = WIFI_STATE_ERR;
	return ThmUnit.Wifi.Rssi;
}

void myWifi::Reconnect(void)
{
	while(WiFi.status() != WL_CONNECTED)
	{
		ThmUnit.Status = STATUS_ERR;
		Serial.print("WIFI connection... ");
		if(WiFi.reconnect())
		{
			Serial.println("connected");
			ThmUnit.Status = STATUS_WIFI_OK;
		}
		else
		{
			Serial.printf("failed, try again in 5 seconds\n");
			delay(5000);
		}
	}
}

void myWifi::loop(void)
{
	Reconnect();
}
