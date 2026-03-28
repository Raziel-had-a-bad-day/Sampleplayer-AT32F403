/** @addtogroup 403A_DAC_two_dac_trianglewave DAC_two_dac_trianglewave
  * @{
  */


#include "at32f403a_407_board.h"
#include "at32f403a_407_clock.h"
gpio_init_type  gpio_init_struct = {0};
dma_init_type dma_init_struct = {0};
crm_clocks_freq_type crm_clocks_freq_struct = {0};

tmr_output_config_type tmr_output_struct;
uint8_t usart2_rx_buffer[usart_buffer_size];
volatile uint8_t usart2_rx_counter =0;


void dac_config(void){
	 system_clock_config();

	  at32_board_init();

	  /* turn led2/led3/led4 on */
	  at32_led_on(LED2);

	  crm_periph_clock_enable(CRM_DAC_PERIPH_CLOCK, TRUE);
	  crm_periph_clock_enable(CRM_TMR7_PERIPH_CLOCK, TRUE);
	  crm_periph_clock_enable(CRM_TMR6_PERIPH_CLOCK, TRUE);
	  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
	  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
	  /* once the dac is enabled, the corresponding gpio pin is automatically
	     connected to the dac converter. in order to avoid parasitic consumption,
	     the gpio pin should be configured in analog */
	  gpio_init_struct.gpio_pins = GPIO_PINS_4 | GPIO_PINS_5;
	  gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
	  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	  gpio_init(GPIOA, &gpio_init_struct);

	  /* get system clock */
	  crm_clocks_freq_get(&crm_clocks_freq_struct);

	  /* (systemclock/(systemclock/1000000))/100 = 10KHz */
	  tmr_base_init(TMR7, 25, (crm_clocks_freq_struct.sclk_freq/1000000 - 1));
	  tmr_cnt_dir_set(TMR7, TMR_COUNT_UP);

	  tmr_base_init(TMR6, 9999, (crm_clocks_freq_struct.sclk_freq/1000000 - 1));   // counter,divider = (cpu / 1m) around 100hz
	  tmr_cnt_dir_set(TMR6, TMR_COUNT_UP);   // ofr ADSR


	  /* primary tmr2 output selection */
	  tmr_primary_mode_select(TMR7, TMR_PRIMARY_SEL_OVERFLOW);
	  tmr_primary_mode_select(TMR6, TMR_PRIMARY_SEL_OVERFLOW);

	  /* dac1 and dac2 configuration */
	  dac_trigger_select(DAC1_SELECT, DAC_SOFTWARE_TRIGGER);
	  dac_trigger_select(DAC2_SELECT, DAC_SOFTWARE_TRIGGER);



	  dac_trigger_enable(DAC1_SELECT, TRUE);
	  dac_trigger_enable(DAC2_SELECT, TRUE);

	  dac_wave_generate(DAC1_SELECT, DAC_WAVE_GENERATE_NONE);
	  dac_wave_generate(DAC2_SELECT, DAC_WAVE_GENERATE_NONE);

	  dac_output_buffer_enable(DAC1_SELECT, FALSE);
	  dac_output_buffer_enable(DAC2_SELECT, FALSE);


	  /* enable dac1: once the dac1 is enabled, pa.04 is
	     automatically connected to the dac converter. */
	  dac_enable(DAC1_SELECT, TRUE);

	  /* enable dac2: once the dac2 is enabled, pa.05 is
	     automatically connected to the dac converter. */
	  dac_enable(DAC2_SELECT, TRUE);



	  /* enable tmr2 */
	  nvic_priority_group_config(NVIC_PRIORITY_GROUP_0);
	 	  nvic_irq_enable(TMR7_GLOBAL_IRQn, 0, 0);

	  tmr_counter_enable(TMR7, TRUE);
	  tmr_counter_enable(TMR6, TRUE);
	  tmr_interrupt_enable(TMR7,TMR_OVF_INT, TRUE);

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

/*void wk_spim_init(void)   // spim setup
{
   add user code begin spim_init 0

   add user code end spim_init 0

  gpio_init_type gpio_init_struct;
  gpio_default_para_init(&gpio_init_struct);

   add user code begin spim_init 1

   add user code end spim_init 1

   configure the CS pin
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_8;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

   configure the SCK pin
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_1;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOB, &gpio_init_struct);

   configure the IO0 pin
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_11;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

   configure the IO1 pin
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_12;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

   configure the IO2 pin
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_7;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOB, &gpio_init_struct);

   configure the IO3 pin
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_6;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOB, &gpio_init_struct);

  gpio_pin_remap_config(EXT_SPIM_GMUX_1000, TRUE);

   configure param
  flash_spim_model_select(FLASH_SPIM_MODEL1);

   unlock the spim flash program erase controller
  while(flash_flag_get(FLASH_SPIM_OBF_FLAG));
 flash_spim_unlock();
  //while(FLASH->ctrl3_bit.oplk);       // problems unlocking

   if the data written to spim flash need to be scrambled, please specify the scrambled range
  flash_spim_encryption_range_set(0x0);

   add user code begin spim_init 2

   add user code end spim_init 2
}*/

static void spi_config(void){

	  crm_periph_clock_enable(CRM_SPI2_PERIPH_CLOCK, TRUE);




	  gpio_init_type gpio_init_struct;
	   spi_init_type spi_init_struct;
	   spi_default_para_init(&spi_init_struct);
	   gpio_default_para_init(&gpio_init_struct);
	  // nvic_irq_enable(SPIx_IRQn, 0, 0);

	   /* add user code begin spi2_init 1 */

	   /* add user code end spi2_init 1 */

	   /* configure the SCK pin */
	   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	   gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	   gpio_init_struct.gpio_pins = GPIO_PINS_13;
	   gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
	   gpio_init(GPIOB, &gpio_init_struct);

	   /* configure the MISO pin */
	   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	   gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
	   gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	   gpio_init_struct.gpio_pins = GPIO_PINS_14;
	   gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	   gpio_init(GPIOB, &gpio_init_struct);

	   /* configure the MOSI pin */
	   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	   gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	   gpio_init_struct.gpio_pins = GPIO_PINS_15;
	   gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	   gpio_init(GPIOB, &gpio_init_struct);

	   /* configure the CS pin */
	/*   gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	   gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	   gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	   gpio_init_struct.gpio_pins = GPIO_PINS_7;
	   gpio_init_struct.gpio_pull =  GPIO_PULL_UP;
	   gpio_init(GPIOB, &gpio_init_struct);*/

	   /* configure param  , one skip maybe int related                  */
	   spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
	   spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
	   spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
	   spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
	   spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_4;  // 120 mhz divider ie 32 =3.6mhz
	   spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
	   spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE; // 1 seems better
	   spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
	   spi_init(SPI2, &spi_init_struct);

	   /* configure the cs pin output */
	 //  spi_hardware_cs_output_enable(SPI2, TRUE);

	   /* add user code begin spi2_init 2 */
	 //  spi_i2s_interrupt_enable(SPI2, SPI_I2S_RDBF_INT, TRUE);


	   /* add user code end spi2_init 2 */

	   spi_enable(SPI2, TRUE);
	   SPI2_CS_HIGH;
	   /* add user code begin spi2_init 3 */

	   /* add user code end spi2_init 3 */




}

void gpio_config(void)
{
  gpio_init_type gpio_init_struct;

  /* enable the gpioa clock */
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

  /* set default parameter */
  gpio_default_para_init(&gpio_init_struct);

  /* configure the gpio */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_7;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(GPIOB, &gpio_init_struct);
}

