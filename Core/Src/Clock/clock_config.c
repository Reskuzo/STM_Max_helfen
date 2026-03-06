/*
 * clock_config.c - H735G-DK adaptation
 * System: 520 MHz (PLLM=5, PLLN=104, PLLP=1, HSE=25MHz)
 * MPU: OctoSPI RAM @ 0x70000000, OctoSPI Flash @ 0x90000000
 * SAI4 PDM: overrides MX_SAI4_ClockConfig to set all PLL2 fields explicitly.
 */
#include "clock_config.h"
#include "main.h"

void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    HAL_StatusTypeDef  ret = HAL_OK;

    /* Supply configuration: Direct SMPS (required for H735G-DK) */
    HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

    /* VCORE = VOS0 (highest performance, required for 520 MHz) */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    /* HSE + PLL1: 25 MHz HSE, PLLM=5 → VCO_in=5 MHz, PLLN=104 → VCO=520 MHz, PLLP=1 → 520 MHz */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState       = RCC_HSI_OFF;
    RCC_OscInitStruct.CSIState       = RCC_CSI_OFF;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 5;
    RCC_OscInitStruct.PLL.PLLN       = 104;
    RCC_OscInitStruct.PLL.PLLFRACN   = 0;
    RCC_OscInitStruct.PLL.PLLP       = 1;   /* SYSCLK = 520 MHz */
    RCC_OscInitStruct.PLL.PLLR       = 2;
    RCC_OscInitStruct.PLL.PLLQ       = 4;
    RCC_OscInitStruct.PLL.PLLVCOSEL  = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLRGE     = RCC_PLL1VCIRANGE_2;
    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if (ret != HAL_OK) { Error_Handler(); }

    /* Select PLL as system clock, configure bus dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                   RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 |
                                   RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1);
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;   /* AHB = 260 MHz */
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
    if (ret != HAL_OK) { Error_Handler(); }

    /* I/O Compensation Cell */
    __HAL_RCC_CSI_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    HAL_EnableCompensationCell();
}

void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    HAL_MPU_Disable();

    /* Region 0: Background - all 4GB strongly ordered, no access */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = 0x00000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Region 1: OctoSPI RAM (ISSI IS66WVS4M8ALL) at 0x70000000, 16MB, Write-Through cacheable */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = 0x70000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_16MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Region 2: OctoSPI Flash at 0x90000000, 64MB */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = 0x90000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_64MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Region 3: D3 SRAM (BDMA audio buffer) at 0x38000000, 64KB
     * Must be non-cacheable: BDMA writes directly to D3 SRAM, bypassing D-Cache.
     * Without this, the CPU reads stale cached zeros from recordPDMBuf. */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = 0x38000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_64KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void CPU_CACHE_Enable(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();
}

/* Override the BSP __weak MX_SAI4_ClockConfig so that ALL PLL2 fields are
 * explicitly set — the default BSP implementation reads PLL2VCOSEL / PLL2RGE
 * from stale reset registers and never writes them back, which is fragile.
 *
 * Configuration (48 kHz group, covers 8/16/32/48/96 kHz):
 *   HSE = 25 MHz,  PLL2M = 25  →  VCO_in  = 1 MHz  (VCIRANGE_0: 1–2 MHz)
 *   PLL2N = 344                →  VCO_out = 344 MHz  (VCOMEDIUM: 150–420 MHz)
 *   PLL2P = 7                  →  SAI4_CK = 49.14 MHz
 *   MCKDIV ≈ 24  →  PDM_CLK   ≈ 2.05 MHz  (IMP34DT05 spec: 1.0–3.25 MHz ✓)
 *
 * 44.1 kHz group (11/22/44 kHz) uses PLL2N=271 / PLL2P=24 → 11.289 MHz SAI_CK.
 */
HAL_StatusTypeDef MX_SAI4_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate)
{
    UNUSED(hsai);

    RCC_PeriphCLKInitTypeDef rcc = {0};

    if ((SampleRate == AUDIO_FREQUENCY_11K) ||
        (SampleRate == AUDIO_FREQUENCY_22K) ||
        (SampleRate == AUDIO_FREQUENCY_44K))
    {
        rcc.PLL2.PLL2N = 271;
        rcc.PLL2.PLL2P = 24;
    }
    else
    {
        rcc.PLL2.PLL2N = 344;
        rcc.PLL2.PLL2P = 7;
    }

    rcc.PeriphClockSelection   = RCC_PERIPHCLK_SAI4A;
    rcc.Sai4AClockSelection    = RCC_SAI4ACLKSOURCE_PLL2;
    rcc.PLL2.PLL2M             = 25;
    rcc.PLL2.PLL2Q             = 1;
    rcc.PLL2.PLL2R             = 1;
    rcc.PLL2.PLL2FRACN         = 0;
    rcc.PLL2.PLL2RGE           = RCC_PLL2VCIRANGE_0;   /* VCO_in 1–2 MHz   */
    rcc.PLL2.PLL2VCOSEL        = RCC_PLL2VCOMEDIUM;    /* VCO_out 150–420 MHz */

    return HAL_RCCEx_PeriphCLKConfig(&rcc);
}