/*
 * esp8266_mqtt.h
 *
 *  Created on: 26 сент. 2022 г.
 *      Author: kiril
 */

#ifndef MQTT_INC_ESP8266_MQTT_H_
#define MQTT_INC_ESP8266_MQTT_H_

#include "stm32f4xx_hal.h"

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))

//Сервер подключения MQTT
extern uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password);
//Подписка на сообщения MQTT
extern uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether);
//Выпуск сообщения MQTT
extern uint8_t MQTT_PublishData(char *topic, char *message, uint8_t qos);
//MQTT отправляет пакет сердцебиения
extern void MQTT_SentHeart(void);


#endif /* MQTT_INC_ESP8266_MQTT_H_ */
