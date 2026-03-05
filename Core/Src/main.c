/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Clock/clock_config.h"
#include "Config/peripherals.h"
#include "Navigation/state_machine.h"

/**
  * @brief  Main program
  * @retval int
  */
int main(void)
{
    MPU_Config();
    CPU_CACHE_Enable();
    HAL_Init();
    SystemClock_Config();

    init_peripherals();
    onProgramStart();

    while (1)
    {
        onTick();
    }
}

/**
  * @brief  Error handler - turns on RED LED and halts
  * @retval None
  */
void Error_Handler(void)
{
    __disable_irq();
    BSP_LED_On(LED2);  /* LED2 = RED on H735G-DK */
    while (1) {}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    while (1) {}
}
#endif /* USE_FULL_ASSERT */