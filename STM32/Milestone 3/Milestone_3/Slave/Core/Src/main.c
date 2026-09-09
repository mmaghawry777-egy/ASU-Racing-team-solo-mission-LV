/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Slave - STM32F103C8 SPI Telemetry
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include <string.h>

/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi1;


/* USER CODE BEGIN PV */


/*
 * Each case contains:
 *
 * case_id
 * voltage_raw
 * speed
 *
 * Voltage is stored x10:
 *
 * 12.3 V -> 123
 * 14.2 V -> 142
 * etc.
 */

typedef struct
{
    uint16_t case_id;
    uint16_t voltage_raw;
    uint16_t speed;

} TelemetryCase;


/*
 * Required Milestone 3 cases
 */

TelemetryCase case_table[5] =
{
    {0x1001, 123, 200},
    {0x1002, 142, 150},
    {0x1003, 138, 100},
    {0x1004, 115, 220},
    {0x1005, 109, 280}
};


/*
 * Received Case ID
 *
 * Only first 2 bytes are used.
 */

uint8_t rxID[2];


/*
 * Dummy receive buffer for Cycle 2
 */

uint8_t dummyRx[3];


/*
 * Response buffer:
 *
 * 3 bytes = 24 bits
 *
 * Byte 0 = Voltage bits 11..4
 * Byte 1 = Voltage bits 3..0 + Speed bits 11..8
 * Byte 2 = Speed bits 7..0
 */

uint8_t txTelemetry[3];


/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);


/* USER CODE BEGIN 0 */


/*
 * Find the Case ID and prepare the 24-bit response.
 */

void PrepareTelemetry(uint16_t requestedID)
{
    /*
     * Default response = zero
     */

    txTelemetry[0] = 0x00;
    txTelemetry[1] = 0x00;
    txTelemetry[2] = 0x00;


    /*
     * Search the five predefined cases
     */

    for (int i = 0; i < 5; i++)
    {
        if (case_table[i].case_id == requestedID)
        {
            uint16_t voltageRaw =
                case_table[i].voltage_raw;

            uint16_t speed =
                case_table[i].speed;


            /*
             * Pack 12-bit Voltage + 12-bit Speed
             *
             * Voltage:
             *
             * VVVVVVVVVVVV
             *
             * Speed:
             *
             * SSSSSSSSSSSS
             *
             * Combined:
             *
             * VVVVVVVV VVVVSSSS SSSSSSSS
             */


            /* Voltage bits 11..4 */

            txTelemetry[0] =
                (uint8_t)((voltageRaw >> 4) & 0xFF);


            /* Voltage bits 3..0 */

            txTelemetry[1] =
                (uint8_t)((voltageRaw & 0x0F) << 4);


            /* Speed bits 11..8 */

            txTelemetry[1] |=
                (uint8_t)((speed >> 8) & 0x0F);


            /* Speed bits 7..0 */

            txTelemetry[2] =
                (uint8_t)(speed & 0xFF);


            /*
             * Case found, stop searching.
             */

            break;
        }
    }
}


/* USER CODE END 0 */


int main(void)
{
    /* Reset peripherals and initialize Flash/Systick */

    HAL_Init();


    /* Configure system clock */

    SystemClock_Config();


    /* Initialize peripherals */

    MX_GPIO_Init();
    MX_SPI1_Init();


    /*
     * IMPORTANT:
     *
     * Slave NSS must be HARDWARE INPUT.
     *
     * PB0 from Master connects to PA4 of Slave.
     */

    hspi1.Init.NSS =
        SPI_NSS_HARD_INPUT;

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }


    /*
     * Start with empty telemetry.
     */

    txTelemetry[0] = 0x00;
    txTelemetry[1] = 0x00;
    txTelemetry[2] = 0x00;


    while (1)
    {

        /* =========================================================
         *
         * CYCLE 1
         *
         * Master sends:
         *
         *       Case ID = 2 bytes
         *
         * Slave receives the Case ID.
         *
         * ========================================================= */


        memset(
            rxID,
            0,
            sizeof(rxID)
        );


        if (HAL_SPI_Receive(
                &hspi1,
                rxID,
                2,
                HAL_MAX_DELAY
            ) == HAL_OK)
        {

            /*
             * Reconstruct 16-bit Case ID
             *
             * Example:
             *
             * rxID[0] = 0x10
             * rxID[1] = 0x02
             *
             * requestedID = 0x1002
             */

            uint16_t requestedID =
                ((uint16_t)rxID[0] << 8) |
                rxID[1];


            /*
             * Search table and prepare
             * the response BEFORE Cycle 2.
             */

            PrepareTelemetry(requestedID);


            /*
             * Small delay gives Proteus/CPU
             * time to finish preparing response.
             */

            HAL_Delay(1);


            /* =====================================================
             *
             * CYCLE 2
             *
             * Master generates 3 clock bytes.
             *
             * Slave sends txTelemetry.
             *
             * ===================================================== */


            memset(
                dummyRx,
                0,
                sizeof(dummyRx)
            );


            HAL_SPI_TransmitReceive(
                &hspi1,
                txTelemetry,
                dummyRx,
                3,
                HAL_MAX_DELAY
            );
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


    if (HAL_RCC_OscConfig(
            &RCC_OscInitStruct
        ) != HAL_OK)
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
        SPI_MODE_SLAVE;


    hspi1.Init.Direction =
        SPI_DIRECTION_2LINES;


    hspi1.Init.DataSize =
        SPI_DATASIZE_8BIT;


    hspi1.Init.CLKPolarity =
        SPI_POLARITY_LOW;


    hspi1.Init.CLKPhase =
        SPI_PHASE_1EDGE;


    /*
     * This will be changed to
     * HARD_INPUT immediately after initialization.
     */

    hspi1.Init.NSS =
        SPI_NSS_SOFT;


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
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};


    __HAL_RCC_GPIOA_CLK_ENABLE();


    /*
     * PA4 = NSS / CS
     *
     * Master PB0 connects here.
     */

    GPIO_InitStruct.Pin =
        GPIO_PIN_4;

    GPIO_InitStruct.Mode =
        GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    HAL_GPIO_Init(
        GPIOA,
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
