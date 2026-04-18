/** @addtogroup 403A_DAC_two_dac_trianglewave DAC_two_dac_trianglewave
  * @{
  */


#include "at32f403a_407_board.h"
#include "at32f403a_407_clock.h"
#define DMA1_CHANNEL1_BUFFER_SIZE  256
#define DMA1_CHANNEL1_MEMORY_BASE_ADDR   0
//#define DMA1_CHANNEL1_PERIPHERAL_BASE_ADDR  0

#define DMA1_CHANNEL2_BUFFER_SIZE   256
#define DMA1_CHANNEL2_MEMORY_BASE_ADDR   0


gpio_init_type  gpio_init_struct = {0};
//dma_init_type dma_init_struct = {0};
crm_clocks_freq_type crm_clocks_freq_struct = {0};

tmr_output_config_type tmr_output_struct;
uint8_t usart2_rx_buffer[usart_buffer_size];
volatile uint8_t usart2_rx_counter =0;
#define DMA_Buffer_size 260  // 256 + 4 command

//systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);

//SysTick->LOAD  = (uint32_t)(300);

void dac_config(void){
	 system_clock_config();
	 const uint16_t sine12bit[32] = {2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056,
	                                 4095, 4056, 3939, 3750, 3495, 3185, 2831, 2447,
	                                 2047, 1647, 1263, 909,  599,  344,  155,  38,
	                                 0,    38,   155,  344,  599,  909,  1263, 1647};

	 int16_t idx;

	 for(idx = 0; idx < 32; idx++)
	   {


		 ccr_buf[idx] = (sine12bit[idx] << 16) + (sine12bit[idx]);
	   }


	 for(idx = 0; idx < 128; idx++){
		sine_testing[idx]=(sine12bit[idx&31]-2047);
	 }

	 //  at32_board_init();void audio_dac_dma_init(void)

	     dma_init_type dma_init_struct = {0};
	     crm_clocks_freq_type crm_clocks_freq_struct;

	     /* Enable all required clocks */
	     crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
	     crm_periph_clock_enable(CRM_DAC_PERIPH_CLOCK, TRUE);
	     crm_periph_clock_enable(CRM_TMR7_PERIPH_CLOCK, TRUE);
	     crm_periph_clock_enable(CRM_TMR6_PERIPH_CLOCK, TRUE);
	     crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

	     /* PA4 = DAC1, PA5 = DAC2 as analog outputs */
	     gpio_init_type gpio_init_struct;
	     gpio_default_para_init(&gpio_init_struct);
	     gpio_init_struct.gpio_pins = GPIO_PINS_4 | GPIO_PINS_5;
	     gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
	     gpio_init(GPIOA, &gpio_init_struct);

	     /* Get system clock frequency */
	     crm_clocks_freq_get(&crm_clocks_freq_struct);

	     /* ==================== TMR7 - DAC Sample Rate Trigger (~40kHz) ==================== */
	     tmr_base_init(TMR7, 24, (crm_clocks_freq_struct.sclk_freq / 1000000 - 1));   // 25-1 → ~40kHz
	     tmr_cnt_dir_set(TMR7, TMR_COUNT_UP);
	     tmr_primary_mode_select(TMR7, TMR_PRIMARY_SEL_OVERFLOW);   // TRGO on update event

	     /* ==================== TMR6 - ADSR / Audio Timing (~100Hz) ==================== */
	     tmr_base_init(TMR6, 9999, (crm_clocks_freq_struct.sclk_freq / 1000000 - 1));  // ~100 Hz
	     tmr_cnt_dir_set(TMR6, TMR_COUNT_UP);
	     tmr_primary_mode_select(TMR6, TMR_PRIMARY_SEL_OVERFLOW);

	     /* DAC Configuration (Dual mode) */
	     dac_trigger_select(DAC1_SELECT, DAC_TMR7_TRGOUT_EVENT);
	     dac_trigger_select(DAC2_SELECT, DAC_TMR7_TRGOUT_EVENT);

	     dac_trigger_enable(DAC1_SELECT, TRUE);
	     dac_trigger_enable(DAC2_SELECT, TRUE);

	     dac_wave_generate(DAC1_SELECT, DAC_WAVE_GENERATE_NONE);
	     dac_wave_generate(DAC2_SELECT, DAC_WAVE_GENERATE_NONE);

	     dac_output_buffer_enable(DAC1_SELECT, FALSE);
	     dac_output_buffer_enable(DAC2_SELECT, FALSE);

	     dac_dma_enable(DAC1_SELECT, TRUE);
	     dac_dma_enable(DAC2_SELECT, TRUE);

	     dac_enable(DAC1_SELECT, TRUE);
	     dac_enable(DAC2_SELECT, TRUE);

	     /* ==================== DMA1_Channel1 for Dual DAC ==================== */
	     dma_reset(DMA1_CHANNEL1);
	     dma_default_para_init(&dma_init_struct);

	     dma_init_struct.buffer_size             = audio_buffer_size;
	     dma_init_struct.direction               = DMA_DIR_MEMORY_TO_PERIPHERAL;
	     dma_init_struct.memory_base_addr        = (uint32_t)ccr_buf;
	     dma_init_struct.memory_data_width       = DMA_MEMORY_DATA_WIDTH_WORD;      // Dual DAC needs 32-bit
	     dma_init_struct.memory_inc_enable       = TRUE;
	     dma_init_struct.peripheral_base_addr    = (uint32_t)0x40007420;            // DAC_DDTH12RD (Dual 12-bit right-aligned)
	     dma_init_struct.peripheral_data_width   = DMA_PERIPHERAL_DATA_WIDTH_WORD;
	     dma_init_struct.peripheral_inc_enable   = FALSE;
	     dma_init_struct.priority = DMA_PRIORITY_LOW;
	     dma_init_struct.loop_mode_enable        = TRUE;          // Very important for continuous playback


	     dma_interrupt_enable(DMA1_CHANNEL1, DMA_FDT_INT, TRUE);

	     nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
	     nvic_irq_enable(DMA1_Channel1_IRQn, 0, 0);

	     dma_init(DMA1_CHANNEL1, &dma_init_struct);

	     /* Flexible DMA mapping required on AT32F403A */
	     dma_flexible_config(DMA1, FLEX_CHANNEL1, DMA_FLEXIBLE_DAC1);

	     /* Start DMA and both timers */
	     dma_channel_enable(DMA1_CHANNEL1, TRUE);
	     tmr_counter_enable(TMR7, TRUE);     // DAC sample rate timer
	     tmr_counter_enable(TMR6, TRUE);     // ADSR / timing timer
	 }


void usart_config(void){

	  gpio_init_type gpio_init_struct;
	  /* enable the usart2 and gpio clock */
	  crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);

	  /* enable the usart3 and gpio clock */
	//  crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);
	//  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

	  gpio_default_para_init(&gpio_init_struct);

	  /* configure the usart3 tx pin */
//	  gpio_init_struct.gpio_pins = GPIO_PINS_10;
//	  gpio_init(GPIOB, &gpio_init_struct);

	  /* configure the usart3 rx pin */
//	  gpio_init_struct.gpio_pins = GPIO_PINS_11;
	//  gpio_init(GPIOB, &gpio_init_struct);

	  /* configure the usart2 rx pin */
	  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
	  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	  gpio_init_struct.gpio_pins = GPIO_PINS_3;
	  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	  gpio_init(GPIOA, &gpio_init_struct);

	  /* config usart nvic interrupt */
	  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
	  nvic_irq_enable(USART2_IRQn, 0, 0);
//	  nvic_irq_enable(USART3_IRQn, 0, 0);

	  /* configure usart2 param */
	  usart_init(USART2, 31250, USART_DATA_8BITS, USART_STOP_1_BIT);
	  usart_receiver_enable(USART2, TRUE);

	  /* configure usart3 param */
//	  usart_init(USART3, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
//	  usart_transmitter_enable(USART3, TRUE);
//	  usart_receiver_enable(USART3, TRUE);


	  /* enable usart2 and usart3 interrupt */
	  usart_interrupt_enable(USART2, USART_RDBF_INT, TRUE);
	  usart_enable(USART2, TRUE);

//	  usart_interrupt_enable(USART3, USART_RDBF_INT, TRUE);
//	  usart_enable(USART3, TRUE);

//	  usart_interrupt_enable(USART3, USART_TDBE_INT, TRUE);

	//  usart_interrupt_enable(USART2, USART_TDBE_INT, TRUE);


}





void gpio_config(void)
{
  gpio_init_type gpio_init_struct;

  /* enable the gpioa clock */
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

  /* set default parameter */
  gpio_default_para_init(&gpio_init_struct);

  /* configure the gpio */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;// CS spi3
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_5;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(GPIOB, &gpio_init_struct);

  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;//CS spi4
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_6;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(GPIOB, &gpio_init_struct);

}

void spi4_init(void)
{
  /* add user code begin spi4_init 0 */

  /* add user code end spi4_init 0 */
	  crm_periph_clock_enable(CRM_SPI4_PERIPH_CLOCK, TRUE);
	  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
	  crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
	  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
  gpio_init_type gpio_init_struct;
  spi_init_type spi_init_struct;

  gpio_default_para_init(&gpio_init_struct);
  spi_default_para_init(&spi_init_struct);

  /* add user code begin spi4_init 1 */

  /* add user code end spi4_init 1 */

  /* configure the SCK pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_7;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOB, &gpio_init_struct);

  /* configure the MISO pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_8;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOB, &gpio_init_struct);

  /* configure the MOSI pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_9;
  gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
  gpio_init(GPIOB, &gpio_init_struct);

  gpio_pin_remap_config(SPI4_GMUX_0010, TRUE);

  /* configure param */
  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_2;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(SPI4, &spi_init_struct);

  spi_i2s_dma_transmitter_enable(SPI4, TRUE);

  spi_i2s_dma_receiver_enable(SPI4, TRUE);

  /* add user code begin spi4_init 2 */

  /* add user code end spi4_init 2 */


  /* add user code begin spi4_init 2 */
  gpio_pin_remap_config(SPI4_GMUX_0010, TRUE);
  /* add user code end spi4_init 2 */

  spi_enable(SPI4, TRUE);
  SPI4_CS_HIGH;
  /* add user code begin spi4_init 3 */

  /* add user code end spi4_init 3 */
}


void wk_nvic_config(void)
{
	  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

	  NVIC_SetPriority(MemoryManagement_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	  NVIC_SetPriority(BusFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	  NVIC_SetPriority(UsageFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	  NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	  NVIC_SetPriority(DebugMonitor_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	  NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
//	  nvic_irq_enable(SPI2_I2S2EXT_IRQn, 0, 0);
//	  nvic_irq_enable(TMR7_GLOBAL_IRQn, 0, 0);
}

	void wk_dma1_channel2_init(void)
	{
	/* add user code begin dma1_channel2 0 */
		crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
	/* add user code end dma1_channel2 0 */

	dma_init_type dma_init_struct;

	dma_reset(DMA1_CHANNEL2);
	dma_default_para_init(&dma_init_struct);
	dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
	dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
	dma_init_struct.memory_inc_enable = TRUE;
	dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
	dma_init_struct.peripheral_inc_enable = FALSE;
	dma_init_struct.priority = DMA_PRIORITY_LOW;
	dma_init_struct.loop_mode_enable = FALSE;
	dma_init(DMA1_CHANNEL2, &dma_init_struct);




	 nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
	  nvic_irq_enable(DMA1_Channel2_IRQn, 4, 0);  ///  THIS IS IMPORTANT


	/* flexible function enable */
	dma_flexible_config(DMA1, FLEX_CHANNEL2, DMA_FLEXIBLE_SPI4_RX);

	/* enable dma1 channel2 full data transfer interrupt */
	dma_interrupt_enable(DMA1_CHANNEL2, DMA_FDT_INT, TRUE);

	/* enable dma1 channel2 half data transfer interrupt */
//	dma_interrupt_enable(DMA1_CHANNEL2, DMA_HDT_INT, TRUE);

	/* add user code begin dma1_channel2 1 */

	/* add user code end dma1_channel2 1 */
	}

	/**
	* @brief  init dma1 channel3 for "spi4_tx"
	* @param  none
	* @retval none
	*/
	void wk_dma1_channel3_init(void)
	{
	/* add user code begin dma1_channel3 0 */
		crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
	/* add user code end dma1_channel3 0 */

	dma_init_type dma_init_struct;

	dma_reset(DMA1_CHANNEL3);
	dma_default_para_init(&dma_init_struct);
	dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
	dma_init_struct.memory_inc_enable = TRUE;
	dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
	dma_init_struct.peripheral_inc_enable = FALSE;
	dma_init_struct.priority = DMA_PRIORITY_LOW;
	dma_init_struct.loop_mode_enable = FALSE;
	dma_init(DMA1_CHANNEL3, &dma_init_struct);


	 nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
		  nvic_irq_enable(DMA1_Channel3_IRQn, 1, 0);  ///  THIS IS IMPORTANT

	/* flexible function enable */
	dma_flexible_config(DMA1, FLEX_CHANNEL3, DMA_FLEXIBLE_SPI4_TX);

	/* enable dma1 channel3 full data transfer interrupt */
	dma_interrupt_enable(DMA1_CHANNEL3, DMA_FDT_INT, TRUE);

	/* add user code begin dma1_channel3 1 */

	/* add user code end dma1_channel3 1 */
	}

	void wk_usart3_init(void)
	{
	  /* add user code begin usart3_init 0 */
		  crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);
		  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
		  crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
	  /* add user code end usart3_init 0 */

	  gpio_init_type gpio_init_struct;
	  gpio_default_para_init(&gpio_init_struct);

	  /* add user code begin usart3_init 1 */

	  /* add user code end usart3_init 1 */

	  /* configure the TX pin */
	  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	  gpio_init_struct.gpio_pins = GPIO_PINS_10;
	  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	  gpio_init(GPIOB, &gpio_init_struct);

	  /* configure the RX pin */
	  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
	  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	  gpio_init_struct.gpio_pins = GPIO_PINS_11;
	  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	  gpio_init(GPIOB, &gpio_init_struct);

	  /* configure param */
	  usart_init(USART3, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
	  usart_transmitter_enable(USART3, TRUE);
	  usart_receiver_enable(USART3, TRUE);
	  usart_parity_selection_config(USART3, USART_PARITY_NONE);

	  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
	  nvic_irq_enable(USART3_IRQn, 0, 0);
	  usart_interrupt_enable(USART3, USART_RDBF_INT, TRUE);

	  usart_hardware_flow_control_set(USART3, USART_HARDWARE_FLOW_NONE);

	  /* enable receive data buffer full interrupt */
	//

	  /* add user code begin usart3_init 2 */

	  /* add user code end usart3_init 2 */

	  usart_enable(USART3, TRUE);

	  /* add user code begin usart3_init 3 */

	  /* add user code end usart3_init 3 */
	}
	void wk_uart4_init(void)
	{
	  /* add user code begin uart4_init 0 */
		  crm_periph_clock_enable(CRM_UART4_PERIPH_CLOCK, TRUE);
		  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
	  /* add user code end uart4_init 0 */

	  gpio_init_type gpio_init_struct;
	  gpio_default_para_init(&gpio_init_struct);

	  /* add user code begin uart4_init 1 */

	  /* add user code end uart4_init 1 */

	  /* configure the TX pin */
	  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	  gpio_init_struct.gpio_pins = GPIO_PINS_0;
	  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	  gpio_init(GPIOA, &gpio_init_struct);

	  /* configure the RX pin */
	  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
	  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	  gpio_init_struct.gpio_pins = GPIO_PINS_1;
	  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	  gpio_init(GPIOA, &gpio_init_struct);

	  gpio_pin_remap_config(UART4_GMUX_0010, TRUE);

	  /* configure param */
	  usart_init(UART4, 9600, USART_DATA_8BITS, USART_STOP_1_BIT);
	  usart_transmitter_enable(UART4, TRUE);
	  usart_receiver_enable(UART4, TRUE);
	  usart_parity_selection_config(UART4, USART_PARITY_NONE);

	  usart_hardware_flow_control_set(UART4, USART_HARDWARE_FLOW_NONE);


	  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
	  nvic_irq_enable(UART4_IRQn, 0, 0);

	  /* enable receive data buffer full interrupt */
	  usart_interrupt_enable(UART4, USART_RDBF_INT, TRUE);

	  /* add user code begin uart4_init 2 */

	  /* add user code end uart4_init 2 */

	  usart_enable(UART4, TRUE);

	  /* add user code begin uart4_init 3 */

	  /* add user code end uart4_init 3 */
	}
