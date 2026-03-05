/*
 * clock_config.c – System clock for STM32H735G-DK
 *   HSE = 25 MHz → PLL1 → 520 MHz SYSCLK
 *   HCLK = 260 MHz (AHB div 2), APBx = 130 MHz (APB div 2)
 *   Voltage scaling: SCALE0, Flash latency: 3 WS
 */
#include "clock_config.h"

void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  HAL_StatusTypeDef  ret;

  /* Voltage Scaling 0: required for 520 MHz operation */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* HSE on, PLL1 from HSE: 25/5*104 = 520 MHz */
  RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState        = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState        = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState        = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM        = 5;
  RCC_OscInitStruct.PLL.PLLN        = 104;
  RCC_OscInitStruct.PLL.PLLP        = 1;   /* 520 MHz */
  RCC_OscInitStruct.PLL.PLLQ        = 4;
  RCC_OscInitStruct.PLL.PLLR        = 2;
  RCC_OscInitStruct.PLL.PLLFRACN    = 0;
  RCC_OscInitStruct.PLL.PLLVCOSEL   = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE      = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if (ret != HAL_OK) Error_Handler();

  /* Select PLL1 as SYSCLK, AHB /2 = 260 MHz, APBx /2 = 130 MHz */
  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                     RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 |
                                     RCC_CLOCKTYPE_PCLK2   | RCC_CLOCKTYPE_D3PCLK1;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
  if (ret != HAL_OK) Error_Handler();

  /* I/O compensation cell */
  __HAL_RCC_CSI_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  HAL_EnableCompensationCell();
}

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  HAL_MPU_Disable();

  /* Region 0: full 4 GB – strongly ordered, no access */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x00;
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

  /* Region 1: OctoSPI RAM / LCD framebuffer (0x70000000, 32 MB)
   * Non-cacheable + non-bufferable so LTDC DMA sees fresh data */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x70000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 2: OctoSPI NOR flash (0x90000000, 128 MB) – cacheable, executable */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x90000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_128MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 3: D3 SRAM (0x38000000, 16 KB) – non-cacheable for BDMA audio PDM buffer */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x38000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_16KB;
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

HAL_StatusTypeDef MX_SAI4_Block_A_Init(SAI_HandleTypeDef* hsai, MX_SAI_Config_t *MXConfig)
{
  HAL_StatusTypeDef ret = HAL_OK;

  __HAL_SAI_DISABLE(hsai);

  hsai->Init.AudioFrequency      = MXConfig->AudioFrequency;
  hsai->Init.MonoStereoMode      = MXConfig->MonoStereoMode;
  hsai->Init.AudioMode           = MXConfig->AudioMode;
  hsai->Init.NoDivider           = SAI_MASTERDIVIDER_DISABLE;
  hsai->Init.Protocol            = SAI_FREE_PROTOCOL;
  hsai->Init.DataSize            = MXConfig->DataSize;
  hsai->Init.FirstBit            = SAI_FIRSTBIT_LSB;
  hsai->Init.ClockStrobing       = MXConfig->ClockStrobing;
  hsai->Init.Synchro             = MXConfig->Synchro;
  hsai->Init.OutputDrive         = MXConfig->OutputDrive;
  hsai->Init.FIFOThreshold       = SAI_FIFOTHRESHOLD_1QF;
  hsai->Init.SynchroExt          = MXConfig->SynchroExt;
  hsai->Init.CompandingMode      = SAI_NOCOMPANDING;
  hsai->Init.TriState            = SAI_OUTPUT_RELEASED;
  hsai->Init.Mckdiv              = 0;
  hsai->Init.PdmInit.Activation  = ENABLE;
  hsai->Init.PdmInit.MicPairsNbr = 1;   /* H735G-DK has 1 PDM mic pair */
  hsai->Init.PdmInit.ClockEnable = SAI_PDM_CLOCK1_ENABLE;

  hsai->FrameInit.FrameLength       = MXConfig->FrameLength;
  hsai->FrameInit.ActiveFrameLength = MXConfig->ActiveFrameLength;
  hsai->FrameInit.FSDefinition      = SAI_FS_STARTFRAME;
  hsai->FrameInit.FSPolarity        = SAI_FS_ACTIVE_HIGH;
  hsai->FrameInit.FSOffset          = SAI_FS_FIRSTBIT;

  hsai->SlotInit.FirstBitOffset = 0;
  hsai->SlotInit.SlotSize       = SAI_SLOTSIZE_DATASIZE;
  hsai->SlotInit.SlotNumber     = 1;
  hsai->SlotInit.SlotActive     = MXConfig->SlotActive;

  if (HAL_SAI_Init(hsai) != HAL_OK) ret = HAL_ERROR;

  __HAL_SAI_ENABLE(hsai);
  return ret;
}
