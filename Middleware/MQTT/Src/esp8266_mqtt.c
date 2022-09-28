/*
 * esp8266_mqtt.c
 *
 *  Created on: 26 сент. 2022 г.
 *      Author: kiril
 */
#include "esp8266_mqtt.h"
#include "esp8266_at.h"

//Соединение прошло успешно, сервер ответил 20 02 00 00
//Клиент активно отключает e0 00
const uint8_t parket_connetAck[] = {0x20,0x02,0x00,0x00};
const uint8_t parket_disconnet[] = {0xe0,0x00};
const uint8_t parket_heart[] = {0xc0,0x00};
const uint8_t parket_heart_reply[] = {0xc0,0x00};
const uint8_t parket_subAck[] = {0x90,0x03};

volatile uint16_t MQTT_TxLen;

//MQTT отправляет данные
void MQTT_SendBuf(uint8_t *buf,uint16_t len)
{
	ESP8266_ATSendBuf(buf,len);
}

//Отправить пакет сердцебиения
void MQTT_SentHeart(void)
{
	MQTT_SendBuf((uint8_t *)parket_heart,sizeof(parket_heart));
}

//MQTT безоговорочно отключен
void MQTT_Disconnect()
{
	MQTT_SendBuf((uint8_t *)parket_disconnet,sizeof(parket_disconnet));
}

//Инициализация MQTT
void MQTT_Init(uint8_t *prx,uint16_t rxlen,uint8_t *ptx,uint16_t txlen)
{
	memset(usart1_txbuf,0,sizeof(usart1_txbuf)); //Очистить буфер отправки
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //Очистить буфер приема

	//Безоговорочно проявите инициативу по отключению первым
	MQTT_Disconnect();HAL_Delay(100);
	MQTT_Disconnect();HAL_Delay(100);
}

//Функция упаковки для сервера подключения MQTT
uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password)
{
	int ClientIDLen = strlen(ClientID);
	int UsernameLen = strlen(Username);
	int PasswordLen = strlen(Password);
	int DataLen;
	MQTT_TxLen=0;
	//Заголовок переменной + полезная нагрузка Каждое поле содержит идентификатор длиной в два байта
  DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);

  //Фиксированный заголовок
   //Тип управляющего сообщения
  usart1_txbuf[MQTT_TxLen++] = 0x10;		//Тип сообщения MQTT ПОДКЛЮЧИТЬСЯ
  //Оставшаяся длина (без учета фиксированной головки)
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// если для кодирования требуется больше данных, установите верхний бит этого байта
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		usart1_txbuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );

	//Заголовок переменной
	 //Имя протокола
	usart1_txbuf[MQTT_TxLen++] = 0;        		// Protocol Name Length MSB
	usart1_txbuf[MQTT_TxLen++] = 4;        		// Protocol Name Length LSB
	usart1_txbuf[MQTT_TxLen++] = 'M';        	// ASCII Code for M
	usart1_txbuf[MQTT_TxLen++] = 'Q';        	// ASCII Code for Q
	usart1_txbuf[MQTT_TxLen++] = 'T';        	// ASCII Code for T
	usart1_txbuf[MQTT_TxLen++] = 'T';        	// ASCII Code for T
	//Уровень протокола
	usart1_txbuf[MQTT_TxLen++] = 4;        		// MQTT Protocol version = 4
	//Флаг подключения
	usart1_txbuf[MQTT_TxLen++] = 0xc2;        	// conn flags
	usart1_txbuf[MQTT_TxLen++] = 0;        		// Keep-alive Time Length MSB
	usart1_txbuf[MQTT_TxLen++] = 60;        	// Keep-alive Time Length LSB  60S心跳包

	usart1_txbuf[MQTT_TxLen++] = BYTE1(ClientIDLen);// Client ID length MSB
	usart1_txbuf[MQTT_TxLen++] = BYTE0(ClientIDLen);// Client ID length LSB
	memcpy(&usart1_txbuf[MQTT_TxLen],ClientID,ClientIDLen);
	MQTT_TxLen += ClientIDLen;

	if(UsernameLen > 0)
	{
		usart1_txbuf[MQTT_TxLen++] = BYTE1(UsernameLen);		//username length MSB
		usart1_txbuf[MQTT_TxLen++] = BYTE0(UsernameLen);    	//username length LSB
		memcpy(&usart1_txbuf[MQTT_TxLen],Username,UsernameLen);
		MQTT_TxLen += UsernameLen;
	}

	if(PasswordLen > 0)
	{
		usart1_txbuf[MQTT_TxLen++] = BYTE1(PasswordLen);		//password length MSB
		usart1_txbuf[MQTT_TxLen++] = BYTE0(PasswordLen);    	//password length LSB
		memcpy(&usart1_txbuf[MQTT_TxLen],Password,PasswordLen);
		MQTT_TxLen += PasswordLen;
	}

	uint8_t cnt=2;
	uint8_t wait;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));
		MQTT_SendBuf(usart1_txbuf,MQTT_TxLen);
		wait=30;//Подождите 3 секунды
		while(wait--)
		{
			//CONNECT
			if(usart1_rxbuf[0]==parket_connetAck[0] && usart1_rxbuf[1]==parket_connetAck[1]) //Успешное подключение
			{
				return 1;//Соединение успешно
			}
			HAL_Delay(100);
		}
	}
	return 0;
}

//Функция упаковки данных подписки MQTT/отмены подписки
//тема тема
//уровень сообщения qos
//следует ли подписаться/отказаться от пакета запросов на подписку
uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether)
{
	MQTT_TxLen=0;
	int topiclen = strlen(topic);

	int DataLen = 2 + (topiclen+2) + (whether?1:0);//Длина заголовка переменной (2 байта) плюс длина полезной нагрузки
	//Фиксированный заголовок
	 //Тип управляющего сообщения
	if(whether) usart1_txbuf[MQTT_TxLen++] = 0x82; //Тип сообщения и подписка на логотип
	else	usart1_txbuf[MQTT_TxLen++] = 0xA2;    //Отказаться от подписки

	//Оставшаяся длина
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// если для кодирования требуется больше данных, установите верхний бит этого байта
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		usart1_txbuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );

	//Заголовок переменной
	usart1_txbuf[MQTT_TxLen++] = 0;				//Идентификатор сообщения MSB
	usart1_txbuf[MQTT_TxLen++] = 0x01;           //Идентификатор сообщения LSB
	//Полезные нагрузки
	usart1_txbuf[MQTT_TxLen++] = BYTE1(topiclen);//Длина темы MSB
	usart1_txbuf[MQTT_TxLen++] = BYTE0(topiclen);//Длина темы LSB
	memcpy(&usart1_txbuf[MQTT_TxLen],topic,topiclen);
	MQTT_TxLen += topiclen;

	if(whether)
	{
		usart1_txbuf[MQTT_TxLen++] = qos;//Уровень QoS
	}

	uint8_t cnt=2;
	uint8_t wait;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));
		MQTT_SendBuf(usart1_txbuf,MQTT_TxLen);
		wait=30;//Подождите 3 секунды
		while(wait--)
		{
			if(usart1_rxbuf[0]==parket_subAck[0] && usart1_rxbuf[1]==parket_subAck[1]) //Подписался успешно
			{
				return 1;//Успешно подписан
			}
			HAL_Delay(100);
		}
	}
	if(cnt) return 1;	//Успешно подписан
	return 0;
}

//Функция упаковки данных публикации MQTT
//тема тема
//сообщение сообщение
//уровень сообщения qos
uint8_t MQTT_PublishData(char *topic, char *message, uint8_t qos)
{
	int topicLength = strlen(topic);
	int messageLength = strlen(message);
	static uint16_t id=0;
	int DataLen;
	MQTT_TxLen=0;
	//Длина полезной нагрузки рассчитывается следующим образом: Вычтите длину заголовка переменной из значения поля оставшейся длины в фиксированном заголовке
	 //Когда QOS равно 0, идентификатор отсутствует
	 //Длина данных Имя субъекта идентификатор пакета полезная нагрузка
	if(qos)	DataLen = (2+topicLength) + 2 + messageLength;
	else	DataLen = (2+topicLength) + messageLength;
	//Фиксированный заголовок
	 //Тип управляющего сообщения
	usart1_txbuf[MQTT_TxLen++] = 0x30;    // ПУБЛИКАЦИЯ типа сообщения MQTT
	//Оставшаяся длина
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// если для кодирования требуется больше данных, установите верхний бит этого байта
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		usart1_txbuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );

	usart1_txbuf[MQTT_TxLen++] = BYTE1(topicLength);//Длина темы MSB
	usart1_txbuf[MQTT_TxLen++] = BYTE0(topicLength);//Длина темы LSB
	memcpy(&usart1_txbuf[MQTT_TxLen],topic,topicLength);//Копировать тему
	MQTT_TxLen += topicLength;

	//Идентификатор сообщения
	if(qos)
	{
			usart1_txbuf[MQTT_TxLen++] = BYTE1(id);
			usart1_txbuf[MQTT_TxLen++] = BYTE0(id);
			id++;
	}
	memcpy(&usart1_txbuf[MQTT_TxLen],message,messageLength);
  MQTT_TxLen += messageLength;

	MQTT_SendBuf(usart1_txbuf,MQTT_TxLen);
  return MQTT_TxLen;
}

