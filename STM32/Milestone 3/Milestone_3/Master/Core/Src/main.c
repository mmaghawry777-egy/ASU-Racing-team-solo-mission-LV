/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Master - STM32F103C8 SPI Telemetry
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

uint8_t rxChar;

uint16_t caseID;

/* Case ID: 2 bytes */
uint8_t spiTxID[2];
uint8_t spiRxID[2];

/* Telemetry: 3 bytes = 24 bits */
uint8_t spiTxData[3];
uint8_t spiRxData[3];

uint16_t voltageRaw;
uint16_t speedRaw;

float voltage;
float speed;

char uartBuf[150];

/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);


/* USER CODE BEGIN 0 */

void UART_Print(char *str)
{
    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)str,
        strlen(str),
        HAL_MAX_DELAY
    );
}


/* Send Case ID to Slave */
HAL_StatusTypeDef SPI_SendCaseID(uint16_t id)
{
    spiTxID[0] = (uint8_t)(id >> 8);
    spiTxID[1] = (uint8_t)(id & 0xFF);

    spiRxID[0] = 0;
    spiRxID[1] = 0;

    /* CS LOW */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    HAL_StatusTypeDef status =
        HAL_SPI_TransmitReceive(
            &hspi1,
            spiTxID,
            spiRxID,
            2,
            500
        );

    /* CS HIGH */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

    return status;
}


/* Receive 24-bit telemetry from Slave */
HAL_StatusTypeDef SPI_ReceiveTelemetry(void)
{
    spiTxData[0] = 0x00;
    spiTxData[1] = 0x00;
    spiTxData[2] = 0x00;

    spiRxData[0] = 0x00;
    spiRxData[1] = 0x00;
    spiRxData[2] = 0x00;

    /* CS LOW */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    HAL_StatusTypeDef status =
        HAL_SPI_TransmitReceive(
            &hspi1,
            spiTxData,
            spiRxData,
            3,
            500
        );

    /* CS HIGH */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

    return status;
}


/* USER CODE END 0 */


int main(void)
{
    /* Reset peripherals, initialize Flash and SysTick */
    HAL_Init();

    /* Configure system clock */
    SystemClock_Config();

    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_USART1_UART_Init();

    /* USER CODE BEGIN 2 */

    /* CS starts HIGH = Slave not selected */
    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_0,
        GPIO_PIN_SET
    );
    HAL_Delay(100);
    UART_Print("\r\n");
    UART_Print("================================\r\n");
    UART_Print("       STM32 TELEMETRY SYSTEM\r\n");
    UART_Print("================================\r\n");
    UART_Print("Enter Case ID (1 - 5): ");

    /* USER CODE END 2 */


    while (1)
    {

        if (HAL_UART_Receive(
                &huart1,
                &rxChar,
                1,
                HAL_MAX_DELAY
            ) == HAL_OK)
        {


            if (rxChar >= '1' && rxChar <= '5')
            {

                caseID =
                    0x1000 +
                    (uint16_t)(rxChar - '0');


                if (SPI_SendCaseID(caseID) != HAL_OK)
                {
                    UART_Print(
                        "\r\n[ERROR] SPI Case ID transmission failed!\r\n"
                    );

                    UART_Print(
                        "Enter Case ID (1 - 5): "
                    );

                    continue;
                }


                HAL_Delay(10);



                if (SPI_ReceiveTelemetry() != HAL_OK)
                {
                    UART_Print(
                        "\r\n[ERROR] SPI telemetry reception failed!\r\n"
                    );

                    UART_Print(
                        "Enter Case ID (1 - 5): "
                    );

                    continue;
                }




                voltageRaw =
                    ((uint16_t)spiRxData[0] << 4) |
                    ((spiRxData[1] >> 4) & 0x0F);


                speedRaw =
                    ((uint16_t)(spiRxData[1] & 0x0F) << 8) |
                    spiRxData[2];


                voltage =
                    (float)voltageRaw / 10.0f;

                speed =
                    (float)speedRaw;




                sprintf(
                    uartBuf,

                    "\r\nSelected Case ID: 0x%04X\r\n"
                    "Voltage: %.1f V | Speed: %.0f km/h\r\n"
                    "\r\nEnter Case ID (1 - 5): ",

                    caseID,
                    voltage,
                    speed
                );


                UART_Print(uartBuf);
            }

            else
            {

                if (rxChar != '\r' && rxChar != '\n')
                {
                    UART_Print(
                        "\r\nInvalid Choice! "
                        "Enter a number between 1 and 5: "
                    );
                }
            }
        }
    }
}


/**
  * @brief System Clock Configuration
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }


    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_HSI;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV1;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;


    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_0
        ) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief SPI1 Initialization Function
  */
static void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;

    hspi1.Init.Mode =
        SPI_MODE_MASTER;

    hspi1.Init.Direction =
        SPI_DIRECTION_2LINES;

    hspi1.Init.DataSize =
        SPI_DATASIZE_8BIT;

    hspi1.Init.CLKPolarity =
        SPI_POLARITY_LOW;

    hspi1.Init.CLKPhase =
        SPI_PHASE_1EDGE;

    /*
     * We manually control CS using PB0
     */
    hspi1.Init.NSS =
        SPI_NSS_SOFT;

    hspi1.Init.BaudRatePrescaler =
        SPI_BAUDRATEPRESCALER_64;

    hspi1.Init.FirstBit =
        SPI_FIRSTBIT_MSB;

    hspi1.Init.TIMode =
        SPI_TIMODE_DISABLE;

    hspi1.Init.CRCCalculation =
        SPI_CRCCALCULATION_DISABLE;

    hspi1.Init.CRCPolynomial =
        10;


    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief USART1 Initialization Function
  */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;

    huart1.Init.BaudRate =
        9600;

    huart1.Init.WordLength =
        UART_WORDLENGTH_8B;

    huart1.Init.StopBits =
        UART_STOPBITS_1;

    huart1.Init.Parity =
        UART_PARITY_NONE;

    huart1.Init.Mode =
        UART_MODE_TX_RX;

    huart1.Init.HwFlowCtl =
        UART_HWCONTROL_NONE;

    huart1.Init.OverSampling =
        UART_OVERSAMPLING_16;


    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};


    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();


    /* PB0 = Chip Select */

    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_0,
        GPIO_PIN_SET
    );


    GPIO_InitStruct.Pin =
        GPIO_PIN_0;

    GPIO_InitStruct.Mode =
        GPIO_MODE_OUTPUT_PP;

    GPIO_InitStruct.Pull =
        GPIO_NOPULL;

    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_LOW;


    HAL_GPIO_Init(
        GPIOB,
        &GPIO_InitStruct
    );
}


/**
  * @brief Error Handler
  */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}
