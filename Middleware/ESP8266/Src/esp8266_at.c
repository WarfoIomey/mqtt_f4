/*
 * esp8266_at.c
 *
 *  Created on: 26 сент. 2022 г.
 *      Author: kiril
 */

#include "esp8266_at.h"

//usart1 массив отправки и получения
uint8_t usart1_txbuf[256];
uint8_t usart1_rxbuf[512];
uint8_t usart1_rxone[1];
uint8_t usart1_rxcounter;


//Последовательный порт 1 отправляет байт
static void USART1_SendOneByte(uint8_t val)
{
	((UART_HandleTypeDef *)&huart1)->Instance->DR = ((uint16_t)val & (uint16_t)0x01FF);
	while((((UART_HandleTypeDef *)&huart1)->Instance->SR&0X40)==0);//Дождитесь завершения доставки
}


//Отправьте данные фиксированной длины на ESP8266
void ESP8266_ATSendBuf(uint8_t* buf,uint16_t len)
{
	memset(usart1_rxbuf,0, 256);

	//Установите общее количество принятых последовательных портов равным 0 перед каждой передачей, чтобы получать
	usart1_rxcounter = 0;

	//Доставка фиксированной длины
	HAL_UART_Transmit(&huart1,(uint8_t *)buf,len,0xFFFF);
}

//Отправить строку в ESP8266
void ESP8266_ATSendString(char* str)
{
  memset(usart1_rxbuf,0, 256);

	//Установите общее количество принятых последовательных портов равным 0 перед каждой передачей, чтобы получать
	usart1_rxcounter = 0;

	//Способ отправки 1
	//while(*str)		USART1_SendOneByte(*str++);

	//Способ отправки 2
	HAL_UART_Transmit(&huart1,(uint8_t *)str,strlen(str),0xFFFF);
}

//Выход из прозрачной передачи
void ESP8266_ExitUnvarnishedTrans(void)
{
	ESP8266_ATSendString("+++");
	HAL_Delay(50);
	ESP8266_ATSendString("+++");
	HAL_Delay(50);
}

//Найти, содержит ли строка другую строку
uint8_t FindStr(char* dest,char* src,uint16_t retry_nms)
{
	retry_nms/=10;                   //Перерыв

	while(strstr(dest,src)==0 && retry_nms--)//Подождите, пока последовательный порт примет или завершит работу по истечении тайм-аута
	{
		HAL_Delay(10);
	}

	if(retry_nms) return 1;

	return 0;
}

/**
* Функция: Проверьте, является ли ESP8266 нормальным
 *Параметр: Отсутствует
 * Возвращаемое значение: статус возврата ESP8266
 * Не-008266 нормальный
 * 008266 Возникла проблема
 */
uint8_t ESP8266_Check(void)
{
	uint8_t check_cnt=5;
	while(check_cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); 	 //Очистить буфер приема
		ESP8266_ATSendString("AT\r\n");     		 			//Отправить ПО команде рукопожатия
		if(FindStr((char*)usart1_rxbuf,"OK",200) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/**
 *Функция: Инициализация ESP8266
 *Параметр: Отсутствует
 * Возвращаемое значение: результат инициализации, отличное от 0 - успешная инициализация, 0 - сбой
 */
uint8_t ESP8266_Init(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	//Очистить массивы отправки и получения
	memset(usart1_txbuf,0,sizeof(usart1_txbuf));
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));

	ESP8266_ExitUnvarnishedTrans();		//Выход из прозрачной передачи
	HAL_Delay(500);
	ESP8266_ATSendString("AT+RST\r\n");
	HAL_Delay(800);
	if(ESP8266_Check()==0)              //Используйте команду AT, чтобы проверить, существует ли ESP8266
	{
		return 0;
	}

	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));    //Очистить буфер приема
	ESP8266_ATSendString("ATE0\r\n");     	//Отключить эхо
	if(FindStr((char*)usart1_rxbuf,"OK",500)==0)  //Неудачная настройка
	{
			return 0;
	}
	return 1;                         //Успешно настроен
}

/**
* Функция: Восстановление заводских настроек
 *Параметр: Отсутствует
 * Возвращаемое значение: Нет
 * Описание: В это время все пользовательские настройки в ESP8266 будут потеряны и восстановлены до заводского состояния
 */
void ESP8266_Restore(void)
{
	ESP8266_ExitUnvarnishedTrans();          	//Выход из прозрачной передачи
  HAL_Delay(500);
	ESP8266_ATSendString("AT+RESTORE\r\n");		//Сброс к заводским настройкам
}

/**
 * Функция: Подключение к точкам доступа
 * параметр：
 * ssid: имя точки доступа
 *pwd: пароль точки доступа
 * Возвращаемое значение：
 * Результат подключения, отличное от 0 соединение было успешным, 0 соединение не удалось
 * означать：
 * Существуют следующие причины сбоя (связь UART и ESP8266 при нормальных обстоятельствах)
 * 1. Неверное имя Wi-FI и пароль
 * 2. Маршрутизатор подключает слишком много устройств и не может назначить IP-адрес ESP8266
 */
uint8_t ESP8266_ConnectAP(char* ssid,char* pswd)
{
	uint8_t cnt=5;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));
		ESP8266_ATSendString("AT+CWMODE_CUR=1\r\n");              //Установить в режим СТАНЦИИ
		if(FindStr((char*)usart1_rxbuf,"OK",200) != 0)
		{
			break;
		}
	}
	if(cnt == 0)
		return 0;

	cnt=2;
	while(cnt--)
	{
		memset(usart1_txbuf,0,sizeof(usart1_txbuf));//Очистить буфер отправки
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));//Очистить буфер приема
		sprintf((char*)usart1_txbuf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",ssid,pswd);//Подключитесь к целевой точке доступа
		ESP8266_ATSendString((char*)usart1_txbuf);
		if(FindStr((char*)usart1_rxbuf,"OK",8000)!=0)                      //Соединение выполнено успешно и присвоено IP-адресу
		{
			return 1;
		}
	}
	return 0;
}

//Включить режим прозрачной передачи
static uint8_t ESP8266_OpenTransmission(void)
{
	//Установить прозрачный режим передачи
	uint8_t cnt=2;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));
		ESP8266_ATSendString("AT+CIPMODE=1\r\n");
		if(FindStr((char*)usart1_rxbuf,"OK",200)!=0)
		{
			return 1;
		}
	}
	return 0;
}

/**
* Функция: Используйте указанный протокол (TCP/UDP) для подключения к серверу
 * параметр：
 *режим: тип протокола "TCP", "UDP"
 * ip: IP целевого сервера
 * порт: целевой - это номер порта сервера
 * Возвращаемое значение：
 * Результат подключения, отличное от 0 соединение было успешным, 0 соединение не удалось
 * означать：
 * Существуют следующие причины сбоя (связь UART и ESP8266 при нормальных обстоятельствах)
 * 1. Неверны IP-адрес и номер порта удаленного сервера
 * 2. Не подключен к точке доступа
 * 3. Добавление на стороне сервера запрещено (как правило, этого не происходит)
 */
uint8_t ESP8266_ConnectServer(char* mode,char* ip,uint16_t port)
{
	uint8_t cnt;

	ESP8266_ExitUnvarnishedTrans();                   //Для выхода из прозрачной передачи требуется несколько подключений
	HAL_Delay(500);

	//Подключиться к серверу
	cnt=2;
	while(cnt--)
	{
		memset(usart1_txbuf,0,sizeof(usart1_txbuf));//Очистить буфер отправки
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));//Очистить буфер приема
		sprintf((char*)usart1_txbuf,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",mode,ip,port);
		ESP8266_ATSendString((char*)usart1_txbuf);
		if(FindStr((char*)usart1_rxbuf,"CONNECT",8000) !=0 )
		{
			break;
		}
	}
	if(cnt == 0)
		return 0;

	//Установить прозрачный режим передачи
	if(ESP8266_OpenTransmission()==0) return 0;

	//Включить статус отправки
	cnt=2;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //Очистить буфер приема
		ESP8266_ATSendString("AT+CIPSEND\r\n");//Начать находиться в состоянии прозрачной передачи
		if(FindStr((char*)usart1_rxbuf,">",200)!=0)
		{
			return 1;
		}
	}
	return 0;
}

/**
 * Функция: Активное отключение от сервера
 *Параметр: Отсутствует
 * Возвращаемое значение：
 * Результат подключения, отсоединение, отличное от 0, выполнено успешно, отсоединение 0 завершается неудачей
 */
uint8_t DisconnectServer(void)
{
	uint8_t cnt;

	ESP8266_ExitUnvarnishedTrans();	//Выход из прозрачной передачи
	HAL_Delay(500);

	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //Очистить буфер приема
		ESP8266_ATSendString("AT+CIPCLOSE\r\n");//Закрыть ссылку

		if(FindStr((char*)usart1_rxbuf,"CLOSED",200)!=0)//Операция прошла успешно, и сервер был успешно отключен
		{
			break;
		}
	}
	if(cnt) return 1;
	return 0;
}
