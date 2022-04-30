/*
 * spi1_swo_trace.c
 *
 *  Created on: Apr 30, 2022
 *      Author: danielbraun
 */


#include "main.h"

#include "../oam/oam_flash.h"

extern SPI_HandleTypeDef hspi1;

static void MX_SPI1_Init(void)
{
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
}

// from mbed_itm_init

static void switch_to_swo_trace(void)
{
#if 0
	//itm_init();
	DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY |
			DBGMCU_CR_DBG_STOP |
			DBGMCU_CR_DBG_SLEEP |
			DBGMCU_CR_TRACE_IOEN;


	/* Enable write access to ITM registers. */
	//ITM->LAR  = ITM_ENABLE_WRITE;

	/* Trace Port Interface Selected Pin Protocol Register. */
	TPI->SPPR = (2 << TPI_SPPR_TXMODE_Pos);

	/* Trace Port Interface Formatter and Flush Control Register */
	TPI->FFCR = (1 << TPI_FFCR_TrigIn_Pos);

	/* Data Watchpoint and Trace Control Register */
	DWT->CTRL = (1 << DWT_CTRL_CYCTAP_Pos)       |
			(0xF << DWT_CTRL_POSTINIT_Pos)   |
			(0xF << DWT_CTRL_POSTPRESET_Pos) |
			(1 << DWT_CTRL_CYCCNTENA_Pos);

	/* Trace Privilege Register.
	 * Disable access to trace channel configuration from non-privileged mode.
	 */
	ITM->TPR  = 0x0;

	/* Trace Control Register */
	ITM->TCR  = (1 << ITM_TCR_TraceBusID_Pos) |
			(1 << ITM_TCR_DWTENA_Pos)     |
			(1 << ITM_TCR_SYNCENA_Pos)    |
			(1 << ITM_TCR_ITMENA_Pos);

	/* Trace Enable Register */
	ITM->TER = 0xFFFFFFFF;
#endif

	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  GPIO_InitStruct.Pin = GPIO_PIN_3; //|GPIO_PIN_4|GPIO_PIN_5;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF0_TRACE; //GPIO_AF5_SPI1;
	    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

static int flashcnt = 0;

void oam_flash_begin(void)
{
	if (flashcnt) Error_Handler();
	flashcnt++;
	// -----
	MX_SPI1_Init();
}
void oam_flash_end(void)
{
	if (flashcnt != 1) Error_Handler();
	flashcnt--;
	// -----
	HAL_SPI_DeInit(&hspi1);
	switch_to_swo_trace();
}


void oam_flash_unneeded(void)
{
	if (flashcnt != 0) Error_Handler();
	// -----
	HAL_SPI_DeInit(&hspi1);
	switch_to_swo_trace();
}
