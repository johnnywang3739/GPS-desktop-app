/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "dma.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "GPSUtils.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define UART_BUFFER_SIZE 1024

uint8_t rxBuffer[UART_BUFFER_SIZE];
uint8_t rxData;
uint16_t rxIndex = 0;
volatile uint8_t sentence_ready = 0;
volatile uint8_t parsing_complete = 0;
GNGGA_Data gngga_data;
GNRMC_Data gnrmc_data;
GNGLL_Data gngll_data;
GNVTG_Data gnvtk_data;
volatile uint8_t data_ready = 0;
volatile uint32_t command_counter = 0;

/* Global structure to hold all satellite data for the current PPS */
SatelliteData satellite_data;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void parse_data(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int fd, char* ptr, int len) {
  HAL_StatusTypeDef hstatus;

  if (fd == 1 || fd == 2) {
    hstatus = HAL_UART_Transmit(&huart3, (uint8_t *) ptr, len, HAL_MAX_DELAY);
    if (hstatus == HAL_OK)
      return len;
    else
      return -1;
  }
  return -1;
}
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
  MX_USB_OTG_FS_PCD_Init();
  MX_USART3_UART_Init();
  MX_USART2_UART_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
//  HAL_UART_Receive_IT(&huart2, &rxData, 1);
  HAL_UART_Receive_DMA(&huart2, &rxData, 1);
  reset_satellite_data();
  HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	if (sentence_ready)
	{
	  parse_data();
	  sentence_ready = 0;
	}

	if(parsing_complete)
	{
	  GNSS_data_to_PC();
	  parsing_complete = 0;
	  reset_satellite_data();  // Reset satellite data
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 24;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (rxData == '\n')
        {
            sentence_ready = 1;
            rxBuffer[rxIndex] = '\0';
            rxIndex = 0;
        }
        else
        {
            rxBuffer[rxIndex++] = rxData;
            if (rxIndex >= UART_BUFFER_SIZE)
            {
                rxIndex = 0;
            }
        }
        HAL_UART_Receive_DMA(&huart2, &rxData, 1);
    }
}

void displayGNSSData(void) {

	HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);

    char buffer[512];

    snprintf(buffer, sizeof(buffer),
             "GNGGA: UTC Time: %s, Lat: %s %s, Lon: %s %s, Alt: %s %s, Chksum: %s\r\n",
             gngga_data.utc_time, gngga_data.latitude, gngga_data.ns_indicator,
             gngga_data.longitude, gngga_data.ew_indicator, gngga_data.altitude,
             gngga_data.alt_units, gngga_data.checksum);
    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

    snprintf(buffer, sizeof(buffer),
             "GNRMC: UTC Time: %s, Status: %s, Lat: %s %s, Lon: %s %s, Speed: %s, Date: %s, Chksum: %s\r\n",
             gnrmc_data.utc_time, gnrmc_data.status, gnrmc_data.latitude, gnrmc_data.ns_indicator,
             gnrmc_data.longitude, gnrmc_data.ew_indicator, gnrmc_data.speed_over_ground,
             gnrmc_data.date, gnrmc_data.checksum);
    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

    snprintf(buffer, sizeof(buffer),
             "GNGLL: Lat: %s %s, Lon: %s %s, UTC Time: %s, Status: %s, Chksum: %s\r\n",
             gngll_data.latitude, gngll_data.ns_indicator, gngll_data.longitude, gngll_data.ew_indicator,
             gngll_data.utc_time, gngll_data.status, gngll_data.checksum);
    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

    snprintf(buffer, sizeof(buffer),
             "GNVTG: COGT: %s %s, COGM: %s %s, SOGN: %s %s, SOGK: %s %s, Mode Ind: %s, Chksum: %s\r\n",
             gnvtk_data.cogt, gnvtk_data.cogt_indicator, gnvtk_data.cogm, gnvtk_data.cogm_indicator,
             gnvtk_data.sogn, gnvtk_data.sogn_indicator, gnvtk_data.sogk, gnvtk_data.sogk_indicator,
             gnvtk_data.mode_indicator, gnvtk_data.checksum);
    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
}


void reset_satellite_data(void)
{
    if (satellite_data.satellites)
    {
        free(satellite_data.satellites);
        satellite_data.satellites = NULL;
    }
    satellite_data.num_satellites = 0;
}
void parse_data(void)
{
    static GSV_Data gsv_data = {0};

    HAL_GPIO_WritePin(GPIOE, LD2_Pin, GPIO_PIN_SET);

    if (strncmp((char *)rxBuffer, "$GNGGA", 6) == 0)
    {
        parseGNGGA((char *)rxBuffer, &gngga_data);
    }
    else if (strncmp((char *)rxBuffer, "$GNRMC", 6) == 0)
    {
        parseGNRMC((char *)rxBuffer, &gnrmc_data);
    }
    else if (strncmp((char *)rxBuffer, "$GNGLL", 6) == 0)
    {
        parseGNGLL((char *)rxBuffer, &gngll_data);
    }
    else if (strncmp((char *)rxBuffer, "$GNVTG", 6) == 0)
    {
        parseGNVTG((char *)rxBuffer, &gnvtk_data);
    }
    else if (strncmp((char *)rxBuffer, "$GPGSV", 6) == 0 || strncmp((char *)rxBuffer, "$GLGSV", 6) == 0 ||
             strncmp((char *)rxBuffer, "$GAGSV", 6) == 0 || strncmp((char *)rxBuffer, "$GBGSV", 6) == 0)
    {
        parseGSV((char *)rxBuffer, &gsv_data, &satellite_data);
    }
    else if (strncmp((char *)rxBuffer, "$GQGSV,1,1,00,8", 15) == 0)
    {

        parsing_complete = 1;
    }

    sentence_ready = 0;
    HAL_GPIO_WritePin(GPIOE, LD2_Pin, GPIO_PIN_RESET);
}
//
//void GNSS_data_to_PC(void)
//{
//    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
//    char buffer[512];
//
//    printf("Sending GNSS Data to PC...\r\n");
//    printf("Number of Satellites: %d\r\n", satellite_data.num_satellites);
//
//    snprintf(buffer, sizeof(buffer),
//             "Satellites: %d\r\n",
//             satellite_data.num_satellites);
//    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
//
//    for (int i = 0; i < satellite_data.num_satellites; i++)
//    {
//        snprintf(buffer, sizeof(buffer),
//                 "TalkerID: %s, ID: %d, Elevation: %d, Azimuth: %d, SNR: %d\r\n",
//                 satellite_data.satellites[i].talker_id,
//                 satellite_data.satellites[i].sat_id,
//                 satellite_data.satellites[i].elevation,
//                 satellite_data.satellites[i].azimuth,
//                 satellite_data.satellites[i].snr);
//        HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
//    }
//
//    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
//}

//void GNSS_data_to_PC(void) {
//	HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
//    char buffer[256];
//
//    snprintf(buffer, sizeof(buffer),
//             "%s,%s,%s,%s,%s,%s,%s\r\n",
//             gnrmc_data.utc_time,   // UTC Time
//             gnrmc_data.latitude,   // Latitude
//             gnrmc_data.ns_indicator, // North/South Indicator
//             gnrmc_data.longitude,  // Longitude
//             gnrmc_data.ew_indicator, // East/West Indicator
//             gnrmc_data.speed_over_ground, // Speed over ground
//             gnrmc_data.date        // Date
//            );
//
//    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
//    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
//}
//void GNSS_data_to_PC(void)
//{
//    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
//    char buffer[512];
//
//    printf("Sending GNSS Data to PC...\r\n");
//    printf("Number of Satellites: %d\r\n", satellite_data.num_satellites);
//
//    snprintf(buffer, sizeof(buffer),
//             "Satellites: %d\r\n",
//             satellite_data.num_satellites);
//    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
//
//    for (int i = 0; i < satellite_data.num_satellites; i++)
//    {
//        snprintf(buffer, sizeof(buffer),
//                 "TalkerID: %s, ID: %d, Elevation: %d, Azimuth: %d, SNR: %d\r\n",
//                 satellite_data.satellites[i].talker_id,
//                 satellite_data.satellites[i].sat_id,
//                 satellite_data.satellites[i].elevation,
//                 satellite_data.satellites[i].azimuth,
//                 satellite_data.satellites[i].snr);
//        HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
//    }
//
//    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
//}

void GNSS_data_to_PC(void)
{
    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
    char buffer[2048];
    int offset = 0;
    int checksum = 0;

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "$");

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       "%s,%s,%s,%s,%s,%s,%s",
                       gnrmc_data.utc_time,   // UTC Time
                       gnrmc_data.latitude,   // Latitude
                       gnrmc_data.ns_indicator, // North/South Indicator
                       gnrmc_data.longitude,  // Longitude
                       gnrmc_data.ew_indicator, // East/West Indicator
                       gnrmc_data.speed_over_ground, // Speed over ground
                       gnrmc_data.date        // Date
                      );

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       ",%d",
                       satellite_data.num_satellites);

    for (int i = 0; i < satellite_data.num_satellites; i++)
    {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           ",%s,%d,%d,%d,%d",
                           satellite_data.satellites[i].talker_id,
                           satellite_data.satellites[i].sat_id,
                           satellite_data.satellites[i].elevation,
                           satellite_data.satellites[i].azimuth,
                           satellite_data.satellites[i].snr);
    }

    // Calculate checksum
    for (int i = 1; i < offset; i++) { // Start at 1 to skip the '$'
        checksum ^= buffer[i];
    }

    // Add checksum and end of line
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "*%02X&\r\n", checksum);

    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
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
