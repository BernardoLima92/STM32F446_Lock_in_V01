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
  *
  * Lock-in processing.
  * Generation of a reference signal with 128 points and 200Hz.12-bit DAC resolution. (4095 equals 3.3V.)
  * TIM8 counts from 0 to 4689. When it passes 1500, it triggers the analog reading. When it bursts, it updates the DAC.
  *
  * This program sends strings through the serial port. A labview program was created on the computer to receive this data and generate graphs.
  *
  * By Eng. Bernardo Caio
  * bernardo.caio92@gmail.com
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define Ns 128
#define Periodos 100
#define pntsVarredura 150

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

// Reference Singal r(t)
uint32_t LUT[Ns] = {983,1016,1049,1083,1116,1148,1181,1212,1244,1274,1304,1333,1362,1389,1415,1441,1465,1488,1510,1531,1550,1568,1584,1599,1613,1625,1636,1645,1652,1658,1662,1664,1665,1664,1662,1658,1652,1645,1636,1625,1613,1599,1584,1568,1550,1531,1510,1488,1465,1441,1415,1389,1362,1333,1304,1274,1244,1212,1181,1148,1116,1083,1049,1016,983,949,916,882,849,817,784,753,721,691,661,632,603,576,550,524,500,477,455,434,415,397,381,366,352,340,329,320,313,307,303,301,300,301,303,307,313,320,329,340,352,366,381,397,415,434,455,477,500,524,550,576,603,632,661,691,721,753,784,817,849,882,916,949};

uint32_t AdcRead[(Periodos*Ns)];


float resultado[(Periodos*Ns)];	// Array that temporarily stores 1st and 2nd Harmonics

float arg1 = 2 * 3.14159265358979323 * 1;
float arg2 = 2 * 3.14159265358979323 * 2;

float mod1h[pntsVarredura];
float mod2h[pntsVarredura];

float fase1h[pntsVarredura];
float fase2h[pntsVarredura];

float quad1h[pntsVarredura];
float quad2h[pntsVarredura];

bool flag = false;
bool transfer = false;

char recebido[10]; 	// buffer that stores the received string.
char resp1[4] = {'n', 'e', 'x', 't'};
char resp2[4] = {'a', 'b', 'c', 'd'};
char dados[8];

int ret;
int i = 0;
int j = 0;


float soma = 0;
float media[5];

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;

TIM_HandleTypeDef htim8;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM8_Init(void);
/* USER CODE BEGIN PFP */

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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */

  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)LUT, Ns, DAC_ALIGN_12B_R);
  HAL_ADC_Start_DMA(&hadc1, AdcRead, (Periodos*Ns));
  HAL_TIM_Base_Start(&htim8);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_UART_Receive(&huart2, recebido, 4, 100);
	  ret = strncmp(recebido, resp1, 4);

	  if (ret == 0){		// If Received string "next"?
		  recebido[0] = 'a';
		  flag = true;

	  }

	  ret = strncmp(recebido, resp2, 4);

	  	  if (ret == 0){		// If Received the string "abcd"?
	  		  recebido[0] = 'l';

/*
			HAL_UART_Transmit(&huart2, "1Hm FASE" , 8, 200);
			HAL_Delay(20);
			for (i = 0; i < pntsVarredura; i++){
				sprintf(dados, "%08f", fase1h[i]);
				HAL_UART_Transmit(&huart2, dados , 8, 200);
				HAL_Delay(10);
			}

			HAL_UART_Transmit(&huart2, "1Hm QUAD" , 8, 200);
			HAL_Delay(20);
			for (i = 0; i < pntsVarredura; i++){
				sprintf(dados, "%08f", quad1h[i]);
				HAL_UART_Transmit(&huart2, dados , 8, 200);
				HAL_Delay(10);
			}

			HAL_UART_Transmit(&huart2, "2Hm FASE" , 8, 200);
			HAL_Delay(20);
			for (i = 0; i < pntsVarredura; i++){
				sprintf(dados, "%08f", fase2h[i]);
				HAL_UART_Transmit(&huart2, dados , 8, 200);
				HAL_Delay(10);
			}

			HAL_UART_Transmit(&huart2, "2Hm QUAD" , 8, 200);
			HAL_Delay(20);
			for (i = 0; i < pntsVarredura; i++){
				sprintf(dados, "%08f", quad2h[i]);
				HAL_UART_Transmit(&huart2, dados , 8, 200);
				HAL_Delay(10);
			}

			HAL_UART_Transmit(&huart2, "1Hm MODL" , 8, 200);
			HAL_Delay(20);
			for (i = 0; i < pntsVarredura; i++){
				sprintf(dados, "%08f", mod1h[i]);
				HAL_UART_Transmit(&huart2, dados , 8, 200);
				HAL_Delay(10);
			}

*/
			HAL_UART_Transmit(&huart2, "2Hm MODL" , 8, 200);
			HAL_Delay(20);
			for (i = 0; i < pntsVarredura; i++){
				sprintf(dados, "%08f", mod2h[i]);
				HAL_UART_Transmit(&huart2, dados , 8, 200);
				HAL_Delay(10);
			}

			j = 0;

			HAL_UART_Transmit(&huart2, "Terminou", 8, 100);

			for (int k = 0; k < pntsVarredura; k++){	// Clearing all vectors
				fase1h[k] = 0;
				fase2h[k] = 0;
				quad1h[k] = 0;
				quad2h[k] = 0;
				mod1h[k] = 0;
				mod2h[k] = 0;
			}


	  	  }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 120;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_CC1;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T8_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 0;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 4689;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM8_Init 2 */

  /* USER CODE END TIM8_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1){

if (flag == true){
	flag = false;
/*
// Phase first harmonic calculation
for (i = 0; i < (Periodos*Ns); i++){
		resultado[i] = (AdcRead[i] * sin(arg1*i/Ns) );
	}
for (i = 0; i < (Periodos*Ns); i++){
		soma = soma + resultado[i];
	}
	fase1h[j] =  soma/(Periodos*Ns);
	soma = 0;

// First Harmonic Calculation in Quadrature
for (i = 0; i < (Periodos*Ns); i++){
		resultado[i] = (AdcRead[i] * cos(arg1*i/Ns) );
	}
for (i = 0; i < (Periodos*Ns); i++){
		soma = soma + resultado[i];
	}
	quad1h[j] =  soma/(Periodos*Ns);
	soma = 0;

// Calculation of the First Harmonic modulus
mod1h[j] = pow( (pow(fase1h[j],2) + pow(quad1h[j], 2)) , 0.5);

*/

// Second Harmonic Calculation in Phase
for (i = 0; i < (Periodos*Ns); i++){
		resultado[i] = (AdcRead[i] * sin(arg2*i/Ns) );
	}
for (i = 0; i < (Periodos*Ns); i++){
		soma = soma + resultado[i];
	}
	fase2h[j] =  soma/(Periodos*Ns);
	soma = 0;

// Second Harmonic Calculation in Quadrature
for (i = 0; i < (Periodos*Ns); i++){
		resultado[i] = (AdcRead[i] * cos(arg2*i/Ns) );
	}
for (i = 0; i < (Periodos*Ns); i++){
		soma = soma + resultado[i];
	}
	quad2h[j] =  soma/(Periodos*Ns);
	soma = 0;

// Calculation of the Second Harmonic modulus
mod2h[j] = pow( (pow(fase2h[j],2) + pow(quad2h[j], 2)) , 0.5);

j = j + 1;


HAL_UART_Transmit(&huart2, "stop", 4, 100);
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
