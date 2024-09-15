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
#define UART_BUFFER_SIZE 3000

uint8_t rxBuffer[UART_BUFFER_SIZE];
uint8_t rxData;
uint16_t rxIndex = 0;
volatile uint8_t sentence_ready = 0;
volatile uint8_t parsing_complete = 0;
GNGGA_Data gngga_data;
GNRMC_Data gnrmc_data;
GNGLL_Data gngll_data;
GNVTG_Data gnvtk_data;
GSA_Talker_Data gsa_talker_gp, gsa_talker_gl, gsa_talker_ga, gsa_talker_gb;
volatile uint8_t data_ready = 0;
volatile uint32_t command_counter = 0;
/* Global structure to hold all satellite data for the current PPS */
SatelliteData satellite_data;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void parse_data(void);
void reset_satellite_data(void);
void GNSS_data_to_PC(void);
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
		if (satellite_data.num_satellites > 0)
		{
			GNSS_data_to_PC();  // Transmit to PC only if valid satellite data
			GPS_data_debug();
		}

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
            memset(&rxBuffer[rxIndex + 1], 0, UART_BUFFER_SIZE - rxIndex - 1);
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




void reset_satellite_data(void) {
    // Iterate over all satellites and reset SNR values where the second signal is missing
    for (int i = 0; i < satellite_data.num_satellites; i++) {
        SatelliteInfo *sat = &satellite_data.satellites[i];

        // Handle GPS (L1, L5)
        if (strncmp(sat->talker_id, "GP", 2) == 0) {
            if (sat->num_signals == 1 && strcmp(sat->signals[0].signal_id, "L1") == 0) {
                strncpy(sat->signals[1].signal_id, "L5", sizeof(sat->signals[1].signal_id));
                sat->signals[1].snr = -1;
                sat->num_signals = 2;
            } else if (sat->num_signals == 1 && strcmp(sat->signals[0].signal_id, "L5") == 0) {
                strncpy(sat->signals[1].signal_id, "L1", sizeof(sat->signals[1].signal_id));
                sat->signals[1].snr = -1;
                sat->num_signals = 2;
            }
        }
        // Handle Galileo (E5a, E1)
        else if (strncmp(sat->talker_id, "GA", 2) == 0) {
            if (sat->num_signals == 1 && strcmp(sat->signals[0].signal_id, "E5a") == 0) {
                strncpy(sat->signals[1].signal_id, "E1", sizeof(sat->signals[1].signal_id));
                sat->signals[1].snr = -1;
                sat->num_signals = 2;
            } else if (sat->num_signals == 1 && strcmp(sat->signals[0].signal_id, "E1") == 0) {
                strncpy(sat->signals[1].signal_id, "E5a", sizeof(sat->signals[1].signal_id));
                sat->signals[1].snr = -1;
                sat->num_signals = 2;
            }
        }
        // Handle BDS (B1I, B2a)
        else if (strncmp(sat->talker_id, "GB", 2) == 0) {
            if (sat->num_signals == 1 && strcmp(sat->signals[0].signal_id, "B1I") == 0) {
                strncpy(sat->signals[1].signal_id, "B2a", sizeof(sat->signals[1].signal_id));
                sat->signals[1].snr = -1;
                sat->num_signals = 2;
            } else if (sat->num_signals == 1 && strcmp(sat->signals[0].signal_id, "B2a") == 0) {
                strncpy(sat->signals[1].signal_id, "B1I", sizeof(sat->signals[1].signal_id));
                sat->signals[1].snr = -1;
                sat->num_signals = 2;
            }
        }
        // GLONASS only has L1
        else if (strncmp(sat->talker_id, "GL", 2) == 0) {
            // Ensure that GLONASS only has one signal
            if (sat->num_signals == 1 && strcmp(sat->signals[0].signal_id, "L1") == 0) {
                sat->signals[1].snr = -1;
            }
        }
    }

    // Free the satellite data
    if (satellite_data.satellites) {
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
    else if (strncmp((char *)rxBuffer, "$GNGSA", 6) == 0)
    {
        char talker_id_str[2] = {0};
        int comma_count = 0;
        int buffer_len = strlen((char *)rxBuffer);
        for (int i = 0; i < buffer_len; i++)
        {
            if (rxBuffer[i] == ',')
            {
                comma_count++;
            }
            if (rxBuffer[i] == '*' && comma_count >= 16) // The GSA sentence has 16 commas
            {
                if (i > 1 && rxBuffer[i - 1] != ',')
                {
                    talker_id_str[0] = rxBuffer[i - 1]; // Store the last character before '*'
                    break;
                }
            }
        }
        int talker_id = atoi(talker_id_str);
        switch (talker_id)
        {
            case 1:  // GPS
                parseGNGSA((char *)rxBuffer, &gsa_talker_gp);
                break;
            case 2:  // GLONASS
                parseGNGSA((char *)rxBuffer, &gsa_talker_gl);
                break;
            case 3:  // Galileo
                parseGNGSA((char *)rxBuffer, &gsa_talker_ga);
                break;
            case 4:  // Beidou (BDS)
                parseGNGSA((char *)rxBuffer, &gsa_talker_gb);
                break;
            default:
                break;
        }
    }

    else if (strncmp((char *)rxBuffer, "$GPGSV", 6) == 0 || strncmp((char *)rxBuffer, "$GLGSV", 6) == 0 ||
             strncmp((char *)rxBuffer, "$GAGSV", 6) == 0 || strncmp((char *)rxBuffer, "$GBGSV", 6) == 0)
    {
        parseGSV((char *)rxBuffer, &gsv_data, &satellite_data);
    }
    //else if (strncmp((char *)rxBuffer, "$GQGSV,1,1,00,8", 15) == 0)
    else if (strncmp((char *)rxBuffer, "$GQGSV", 6) == 0)
    {

        parsing_complete = 1;
    }

    sentence_ready = 0;
    HAL_GPIO_WritePin(GPIOE, LD2_Pin, GPIO_PIN_RESET);
}
void GNSS_data_to_PC(void)
{
    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
    char buffer[2048];
    int offset = 0;
    int checksum = 0;

    // Transmit GNSS position data (GNRMC)
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "$%s,%s,%s,%s,%s,%s,%s",
                       gnrmc_data.utc_time,       // UTC Time
                       gnrmc_data.latitude,       // Latitude
                       gnrmc_data.ns_indicator,   // N/S Indicator
                       gnrmc_data.longitude,      // Longitude
                       gnrmc_data.ew_indicator,   // E/W Indicator
                       gnrmc_data.speed_over_ground, // Speed over ground
                       gnrmc_data.date);          // Date

    // Transmit number of satellites in view
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%d", satellite_data.num_satellites);

    // Transmit satellite data (talker_id, sat_id, elevation, azimuth, SignalID, SNR, SignalID, SNR)
    for (int i = 0; i < satellite_data.num_satellites; i++) {
        SatelliteInfo *sat = &satellite_data.satellites[i];
        int snr_1 = sat->signals[0].snr;
        int snr_2 = -1; // Default SNR value for second signal if not present

        const char *signal_1_id = sat->signals[0].signal_id;
        const char *signal_2_id = ""; // Set appropriate second signal ID

        // Determine the second signal ID based on the GNSS system
        if (strncmp(sat->talker_id, "GP", 2) == 0) {
            signal_2_id = "L5";
        } else if (strncmp(sat->talker_id, "GA", 2) == 0) {
            signal_2_id = "E1";
        } else if (strncmp(sat->talker_id, "GB", 2) == 0) {
            signal_2_id = "B2a";
        } else if (strncmp(sat->talker_id, "GL", 2) == 0) {
            // GLONASS only has L1, so no second signal for GLONASS
            signal_2_id = "L1";
        }

        // If the satellite has a second signal, update SNR
        if (sat->num_signals > 1) {
            snr_2 = sat->signals[1].snr;
        }

        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           ",%s,%d,%d,%d,%s,%d,%s,%d",
                           sat->talker_id,          // Talker ID (e.g., GP, GL)
                           sat->sat_id,             // Satellite ID
                           sat->elevation,          // Elevation
                           sat->azimuth,            // Azimuth
                           signal_1_id,             // First Signal ID (e.g., L1, L5, etc.)
                           snr_1,                   // First SNR
                           signal_2_id,             // Second Signal ID (set as per GNSS system)
                           snr_2);                  // Second SNR (-1 if missing)
    }

    // Transmit GSA data (GP, GL, GA, GB)
    GSA_Talker_Data *gsa_talkers[] = { &gsa_talker_gp, &gsa_talker_gl, &gsa_talker_ga, &gsa_talker_gb };
    const char *talker_ids[] = { "GP", "GL", "GA", "GB" };
    int total_active_satellites = 0;

    // Calculate total active satellites
    for (int i = 0; i < 4; i++) {
        total_active_satellites += gsa_talkers[i]->active_satellites;
    }

    // Transmit total active satellites
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%d", total_active_satellites);

    // Transmit active satellites data and DOP values for each system
    for (int i = 0; i < 4; i++) {
        GSA_Talker_Data *gsa = gsa_talkers[i];
        if (gsa->active_satellites > 0) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%s", talker_ids[i]);
            for (int j = 0; j < gsa->active_satellites; j++) {
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%d", atoi(gsa->sat_ids[j]));
            }
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%s,%s,%s", gsa->pdop, gsa->hdop, gsa->vdop);
        }
    }

    // Calculate checksum
    for (int i = 1; i < offset; i++) {
        checksum ^= buffer[i];  // XOR for checksum
    }

    // Append checksum and newline
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "*%02X\r\n", checksum);

    // Transmit the buffer to the PC via UART
    HAL_UART_Transmit(&huart3, (uint8_t *)buffer, offset, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
}

void GPS_data_debug(void)  // for debugging
{
    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);  // Debugging LED ON
    char buffer[2048];
    int offset = 0;
    int checksum = 0;

    // Start building the GNSS data message
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "$");

    // Transmit the timestamp (UTC time)
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s", gnrmc_data.utc_time);  // UTC Time

    // Transmit the number of satellites in view
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",SatInView:%d", satellite_data.num_satellites);

    // Group satellites by their talker ID, similar to how it's done for ActiveSat
    // GP (GPS satellites in view)
    int gp_found = 0;
    for (int i = 0; i < satellite_data.num_satellites; i++) {
        if (strncmp(satellite_data.satellites[i].talker_id, "GP", 2) == 0) {
            if (!gp_found) {
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",GP");
                gp_found = 1;
            }
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%d", satellite_data.satellites[i].sat_id);
        }
    }

    // GL (GLONASS satellites in view)
    int gl_found = 0;
    for (int i = 0; i < satellite_data.num_satellites; i++) {
        if (strncmp(satellite_data.satellites[i].talker_id, "GL", 2) == 0) {
            if (!gl_found) {
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",GL");
                gl_found = 1;
            }
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%d", satellite_data.satellites[i].sat_id);
        }
    }

    // GA (Galileo satellites in view)
    int ga_found = 0;
    for (int i = 0; i < satellite_data.num_satellites; i++) {
        if (strncmp(satellite_data.satellites[i].talker_id, "GA", 2) == 0) {
            if (!ga_found) {
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",GA");
                ga_found = 1;
            }
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%d", satellite_data.satellites[i].sat_id);
        }
    }

    // GB (Beidou satellites in view)
    int gb_found = 0;
    for (int i = 0; i < satellite_data.num_satellites; i++) {
        if (strncmp(satellite_data.satellites[i].talker_id, "GB", 2) == 0) {
            if (!gb_found) {
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",GB");
                gb_found = 1;
            }
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%d", satellite_data.satellites[i].sat_id);
        }
    }

    // Now append the GSA data (number of active satellites across different constellations)
    // Calculate the total number of active satellites across all systems
    int total_active_satellites = gsa_talker_gp.active_satellites +
                                  gsa_talker_gl.active_satellites +
                                  gsa_talker_ga.active_satellites +
                                  gsa_talker_gb.active_satellites;

    // Transmit the total number of active satellites
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",ActiveSat:%d", total_active_satellites);

    // Transmit active satellite IDs for each constellation (GP, GL, GA, GB)

    // Transmit GP (GPS) active satellite IDs
    if (gsa_talker_gp.active_satellites > 0) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",GP");
        for (int i = 0; i < gsa_talker_gp.active_satellites; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%s", gsa_talker_gp.sat_ids[i]);
        }
    }

    // Transmit GL (GLONASS) active satellite IDs
    if (gsa_talker_gl.active_satellites > 0) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",GL");
        for (int i = 0; i < gsa_talker_gl.active_satellites; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%s", gsa_talker_gl.sat_ids[i]);
        }
    }

    // Transmit GA (Galileo) active satellite IDs
    if (gsa_talker_ga.active_satellites > 0) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",GA");
        for (int i = 0; i < gsa_talker_ga.active_satellites; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%s", gsa_talker_ga.sat_ids[i]);
        }
    }

    // Transmit GB (Beidou) active satellite IDs
    if (gsa_talker_gb.active_satellites > 0) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",GB");
        for (int i = 0; i < gsa_talker_gb.active_satellites; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",%s", gsa_talker_gb.sat_ids[i]);
        }
    }

    // Calculate checksum (excluding the initial '$')
    for (int i = 1; i < offset; i++) {
        checksum ^= buffer[i];
    }

    // Append checksum and newline
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "*%02X\r\n", checksum);

    // Transmit the buffer to the PC
    HAL_UART_Transmit(&huart5, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

    // Toggle LED off after transmission
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
