/*
 * esp8266_at.h
 *
 *  Created on: 26 сент. 2022 г.
 *      Author: kiril
 */

#ifndef ESP8266_INC_ESP8266_AT_H_
#define ESP8266_INC_ESP8266_AT_H_

#include "stm32f4xx_hal.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

extern uint8_t usart1_txbuf[256];
extern uint8_t usart1_rxbuf[512];
extern uint8_t usart1_rxone[1];
extern uint8_t usart1_rxcounter;

extern uint8_t ESP8266_Init(void);
extern void ESP8266_Restore(void);

extern void ESP8266_ATSendBuf(uint8_t* buf,uint16_t len);		//Отправить данные указанной длины на ESP8266
extern void ESP8266_ATSendString(char* str);							//Отправить строку в модуль ESP8266
extern void ESP8266_ExitUnvarnishedTrans(void);							//ESP8266 Выйти из режима прозрачной передачи
extern uint8_t ESP8266_ConnectAP(char* ssid,char* pswd);		//ESP8266 подключиться к точке доступа
extern uint8_t ESP8266_ConnectServer(char* mode,char* ip,uint16_t port);	//Подключитесь к серверу, используя указанный протокол (TCP/UDP)

#endif /* ESP8266_INC_ESP8266_AT_H_ */
