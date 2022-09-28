/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "esp8266_at.h"
#include "esp8266_mqtt.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USER_MAIN_DEBUG

#ifdef USER_MAIN_DEBUG
/*#define user_main_printf(format, ...) printf( format "\r\n",##__VA_ARGS__)
#define user_main_info(format, ...) printf("【main】info:" format "\r\n",##__VA_ARGS__)
#define user_main_debug(format, ...) printf("【main】debug:" format "\r\n",##__VA_ARGS__)
#define user_main_error(format, ...) printf("【main】error:" format "\r\n",##__VA_ARGS__)
#else
#define user_main_printf(format, ...)
#define user_main_info(format, ...)
#define user_main_debug(format, ...)
#define user_main_error(format, ...)*/
#endif
//TP-Link-38DB_5G
//85018433
//Redmi Note 8 Pro
//ujyech4171
// Здесь можно внести коррективы в соответствии с вашим собственным Wi-Fi
#define WIFI_NAME "Redmi Note 8 Pro"
#define WIFI_PASSWD "ujyech4565"

//Вот конфигурация входа в облачный сервер Alibaba Cloud server
#define MQTT_BROKERADDRESS "srv2.clusterfly.ru"
#define MQTT_CLIENTID "user_36de9678_lamp"
#define MQTT_USARNAME "user_36de9678"
#define MQTT_PASSWD "pass_c5048650"
#define	MQTT_PUBLISH_TOPIC "user_36de9678/lamp/power"
#define MQTT_SUBSCRIBE_TOPIC "user_36de9678/lamp/power"

//Вот определение макроса задержки для операции основного цикла
#define LOOPTIME 30 	//Программный цикл время задержки цикла: 30 мс
#define COUNTER_RUNINFOSEND		(5000/LOOPTIME)		//Запрос на запуск последовательного порта: 5s
#define COUNTER_MQTTHEART     (5000/LOOPTIME)		//MQTT отправляет пакет сердцебиения: 5s
#define COUNTER_STATUSREPORT	(3000/LOOPTIME)		//Загрузка статуса: 3 секунды

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char mqtt_message[300];	//Кэш сообщений отчета MQTT
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Enter_ErrorMode(uint8_t mode);
void ES8266_MQTT_Init(void);
void deal_MQTT_message(uint8_t* buf,uint16_t len);
extern void initialise_monitor_handles(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	initialise_monitor_handles();
	HAL_UART_Receive_IT(&huart1,usart1_rxone,1);			//Откройте прерывание USART1 и получите сообщение о подписке
	ES8266_MQTT_Init();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	uint16_t Counter_RUNInfo_Send = 0;
//	uint16_t Counter_MQTT_Heart = 0;
	//uint16_t Counter_StatusReport = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(Counter_RUNInfo_Send++>0)
	  		{
	  			Counter_RUNInfo_Send = 0;
	  			printf("Программа запущена！\r\n");
	  		}
	  		HAL_Delay(LOOPTIME);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/******************************  USART1 Код прерывания приема  *****************************/

// ES8266 Последовательный порт привода для приема функции обработки прерываний
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)	//Определите, какой последовательный порт вызвал прерывание
	{
		//Поместите полученные данные в принимающий массив usart1 receiving
		usart1_rxbuf[usart1_rxcounter] = usart1_rxone[0];
		usart1_rxcounter++;	//Полученное количество +1

		//Повторно включите последовательный порт 1 для приема прерывания
		HAL_UART_Receive_IT(&huart1,usart1_rxone,1);
	}
}





/******************************  Введите код режима ошибки  *****************************/

//Войдите в режим ошибки и дождитесь ручного перезапуска
void Enter_ErrorMode(uint8_t mode)
{
	while(1)
	{
		switch(mode){
			case 0:printf("ESP8266 Сбой инициализации！\r\n");break;
			case 1:printf("ESP8266 Не удалось подключиться к точке доступа！\r\n");break;
			case 2:printf("ESP8266 Не удалось подключиться к облачному серверу！\r\n");break;
			case 3:printf("ESP8266 Сбой входа в систему Cloud MQTT！\r\n");break;
			case 4:printf("ESP8266 Ошибка в теме подписки на Cloud MQTT не удалась！\r\n");break;
			default:printf("Ничего\r\n");break;
		}
		printf("Пожалуйста, перезапустите совет по разработке\r\n");
		//HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin);
		HAL_Delay(200);
	}
}

/* ESP8266 Тестовый код MQTT */
void TEST_ES8266MQTT(void)
{
	uint8_t status=0;

	//инициализировать
	if(ESP8266_Init())
	{
		printf("ESP8266 Успешная инициализация！\r\n");
		status++;
	}
	else Enter_ErrorMode(0);

	//Подключение к точкам доступа
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			printf("ESP8266 Успешно подключился к точке доступа！\r\n");
			status++;
		}
		else Enter_ErrorMode(1);
	}

	//Подключение к облачному серверу интернета вещей
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			printf("ESP8266 Успешно подключено к облачному серверу！\r\n");
			status++;
		}
		else Enter_ErrorMode(2);
	}

	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			printf(" Cloud MQTT успешно вошел в систему！\r\n");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//Подписаться на темы
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			printf("ESP8266 Cloud MQTT успешно подписался на эту тему！\r\n");
		}
		else Enter_ErrorMode(4);
	}
}

/******************************  STM32 MQTT код  *****************************/

//Функция инициализации MQTT
void ES8266_MQTT_Init(void)
{
	uint8_t status=0;

	//Инициализация
	if(ESP8266_Init())
	{
		printf("ESP8266 Успешно！\r\n");
		status++;
	}
	else Enter_ErrorMode(0);

	//Подключение к точкам доступа
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			printf("ESP8266 Успешно подключился к точке доступа！\r\n");
			status++;
		}
		else Enter_ErrorMode(1);
	}

	//Подключение к облачному серверу интернета вещей
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,9994)!=0)
		{
			printf("ESP8266 Успешно подключено к облачному серверу！\r\n");
			status++;
		}
		else Enter_ErrorMode(2);
	}

	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			printf("ESP8266 Cloud MQTT успешно вошел в систему！\r\n");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//Подписаться на темы
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			printf("ESP8266 Cloud MQTT успешно подписался на эту тему！\r\n");
		}
		else Enter_ErrorMode(4);
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
