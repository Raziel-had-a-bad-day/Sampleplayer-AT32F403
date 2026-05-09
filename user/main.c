/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "at32f403a_407_board.h"
#include "at32f403a_407_clock.h"



#include"string.h"
#include "stdio.h"
#include <stdlib.h>
#include "string.h"
#include <ctype.h>
#include <stdbool.h>
#include "math.h"
#include "systick.h"
#include "out.bin.h"     // only enable for initial upload, binary ->.h file for samples

#include "variables.h"
#include "sampler_loader.h"
#include "ram.h"
#include "maincode.h"
#include "flash.h"
#include "audio.h"
#include "midi.h"



uint8_t data_count=0;
int32_t start_time=0;
int32_t stop_time=0;
int32_t elapsed_time=0;

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();


 // systick_interrupt_config(98);
  at32_board_init();
  gpio_config();
//  wk_nvic_config();
 // nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
 // systick_init();
  dac_config();
   usart_config();
  // spi_config();
   spi4_init();
   //wk_usart3_init();
   wk_uart4_init();

	SPI2_CS_HIGH;  // disable for ram for now
	sampler_loader_init();



   wk_dma1_channel2_init();// rx
/*   wk_dma_channel_config(DMA1_CHANNEL2,
                         (uint32_t)&SPI4->dt,
						 (uint32_t)ram_page_read_buf,
                        132);*/
   //dma_channel_enable(DMA1_CHANNEL2, TRUE);

   /* init dma1 channel3 */
   wk_dma1_channel3_init(); // TX

  /* wk_dma_channel_config(DMA1_CHANNEL3,
                         (uint32_t)&SPI4->dt,
                          (uint32_t)ram_page_write_buf,
                         132);
*/
  // SPI2_CS_LOW;
  // dma_channel_enable(DMA1_CHANNEL3, TRUE);

  //dma_config_tx_only();
  //dma_config();

 // spi_i2s_interrupt_enable(SPI2, SPI_I2S_TDBE_INT, TRUE);
	midi_note_pwm_calculator();  // issues with float
	//systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV); // this will kill speed
	waves();


///////////// load settings
	uint16_t i;
	int16_t temp[600];
	uint32_t read_adr= settings_data;

	for (i=0;i<512;i++){   // reading ok now
		//temp[i]=*(uint8_t*)(read_adr);
		all_settings[i]=*(uint8_t*)(read_adr);
		  read_adr += 1;
	}
	if (all_settings[0]!=255)	settings_storage();   // only reads once there is data

	//52 onwards
	memcpy(samples_store,samples_backup, sizeof(samples_backup));  // backup samples struct


	envelopes_preprocess(0);
	envelopes_preprocess(1);
	envelopes_preprocess(2);
	envelopes_preprocess(3);



	sample_select[0]=600;
	read_adr= user_data_start +sample_select[0];

	for (i=0;i<600;i++){   // reading ok now

	temp[i]=*(int16_t*)(read_adr);
	  read_adr += 2;
}



for (i=0;i<600;i++){   // reading ok now

	temp[i]=sine_lut[i]-32768; // convert to int

}
for (i=0;i<128;i++){   // reading ok now

	test_int[i]=i; // convert to int

}

memcpy(in_sample_holder,temp,1200); // for testing
memcpy(in_sample_holder_2,in_sample_holder,1200);

memset(flash_sample_buf,0,128); //testing

uint8_t psram_reset[2]={0x66,0x99};
delay_ms(15);

spi_i2s_data_transmit(SPI4,psram_reset[0] );   //  this should reset linear burst in theory
delay_ms(15);
SPI2_CS_LOW;
spi_i2s_data_transmit(SPI4,psram_reset[1] );

delay_ms(15);

for (i=0;i<256;i++){
		ram_page_write_buf[i]=i;
	}
uint8_t tmr_start=0;
uint8_t tmr_end=0;
//samples_store[0].size_bytes=321048;
//  maybe implement skip back function , record 30sec to mem and than skip back when needed

  while(1)
  {


/*
	  if (usart_flag_get(USART3, USART_RDBF_FLAG) != RESET)
	  {
	       ch = usart_data_receive(USART3);
	      if (ch) at32_led_toggle(LED2);

	      // toggle LED or printf via USART2
	  }
*/

	 // if((ccr_counter>=audio_buffer_size)) {  // process 16*2 samples, runs always
		  if((dac_ready)) {  // process 16*2 samples, runs always


 			// basic sound is 180us ,delay adds 50us
 		 		delay_calc();
 		 	  start_time=tmr_counter_value_get(TMR6);
 		 	 	 // about 1200us available before it goes bad
 		 	one_shot_playback_rate=(0x10000)-(cc_76<<7);  // set playback rate before
 		 for (i=0;i<audio_buffer_size;i++){  // 64 atm moment, 250us with linear +40us with hermite resample
 			// tmr_counter[i]=tmr_counter_value_get(TMR7);
 			 next_sample_tracker=i;  // just counts up inside the buf
 			 ccr_counter_2=i;
 			 next_sample();

 			} // process samples  300uS atm


 		 	 if ((!psram_busy)&&(!spi_process_counter)) spi_process_counter=1;  // starts spi processing, can block

 		 	 stop_time=tmr_counter_value_get(TMR6);
 		  	 	if(stop_time>start_time) elapsed_time=stop_time-start_time; else elapsed_time=0;

		 		uint8_t oneshot=(one_shot_position>>16)<<1;  // clear last bit as well , has to be even
 		  	 	if ((oneshot)>128) oneshot=128; // this thing gets screwed up a lot
 		  	 one_shot_pointer+=oneshot;
 		  	 	if (one_shot_pointer >(samples_store[current_playing_sample].size_bytes+samples_store[current_playing_sample].ram_addr))

 		  	 	{one_shot_pointer=samples_store[current_playing_sample].ram_addr;one_shot_position=0;}  //reset sample to start

 		  	 	//else one_shot_pointer+=126;  // wont work with other than 128 ?


		 		// reset one shot pointer
		 		one_shot_position&=0xFFFF; // needs to zero here
		 		//one_shot_position=0; //better reset

		 		memset(flash_sample_buf,0,255);  // clear
		 		ccr_counter=0;
		 		dac_ready=0;
	  }



 	if ((!spi_read_flag) && (!spi_write_flag) && (spi_process_counter)&&(!psram_busy)) spi_message_process();




 	if( uart_receive_timer[3] ) uart_receive_end(); // detect no transmission

 	if ((psram_sample_write==1) && (spi_process_counter==0)&& (!spi_write_flag))     { // copy serial data to psram
 		uart_receive_save();  }


 	  if( midi_in_clear) {  midi_fifo(usart2_rx_buffer,0); } // write to fifo ,returns 0 if done

 	  if (midi_buf_flag)     {midi_incoming();}  // not totally ok yet

 	  if (!note_trigger) note_trigger=note_fifo(0, 1);// enable note if any in fifo , note_fifo isnt really needed though, midi comes in slow , might just skip



 	  if (save_timer>60000) {settings_write_flag=1;
 	 memcpy(samples_backup,samples_store, sizeof(samples_store));
 	  settings_storage();flash_settings_write();save_timer=0;

 	  }  // saves every ten minutes

 	  if((!ADSR_timer_flag) && ((tmr_counter_value_get(TMR6)) >8200))  {  ADSR_TIM_writer();  // about 10ms
 	  ADSR_timer_flag=1;save_timer++;lfo1_out=lfo_out(); if( uart_receive_timer[3] ) {sample_write_end_timer++;}

 	  } //process ADSR
 	  if((ADSR_timer_flag) && ((tmr_counter_value_get(TMR6))<100)) ADSR_timer_flag=0;   //reset   , can miss starts might have sync with notes or trigger more often

 	  if (note_trigger)	 	  note_process();  // assign sounds  to polyphony for note trigger







  }  // end while
} // end of main

/**
  * @}
  */
void USART2_IRQHandler(void)  // midi in
{
  if(usart_interrupt_flag_get(USART2, USART_RDBF_FLAG) != RESET)
  {
    /* read one byte from the receive data register */

	  usart2_rx_buffer[usart2_rx_counter] = usart_data_receive(USART2);  // filter midi channel here first


	  (usart2_rx_buffer[0]==note_on+midi_channel || usart2_rx_buffer[0]==c_change+4 || usart2_rx_buffer[0]==note_on+9 ||
			  usart2_rx_buffer[0]==c_change+9) ?  usart2_rx_counter++:usart2_rx_counter;                       // not receiving cc 185


    if(usart2_rx_counter >(usart_buffer_size-1))
    {
      /* disable the usart2 receive interrupt */
      usart_interrupt_enable(USART2, USART_RDBF_INT, FALSE); // not ideal
      midi_in_clear=1;usart2_rx_counter=0;
  //	ADSR_counter_position[0]=0;
  	 			//		ADSR_out_1=64000;

    }
  }


	}

void UART4_IRQHandler(void)  //file transfer
{
  if(usart_interrupt_flag_get(UART4, USART_RDBF_FLAG) != RESET)
  {
	  at32_led_toggle(LED2);
	//  usart_interrupt_enable(USART2, USART_RDBF_INT, FALSE);
	         //   uart_receive_char(usart_data_receive(UART4));
	  usart4_rx_buffer[usart4_rx_counter] = usart_data_receive(UART4);  // filter midi channel here first


		    if(usart4_rx_counter >126 ) // buffer full
		    {
		    	memcpy(usart4_int_buffer,usart4_rx_buffer,128);
		    	usart4_rx_counter=0;
		    	psram_sample_write=1;
		    	psram_busy=1;
		    	spi_process_counter=0;
		    	// disable other comms
		     // usart_interrupt_enable(UART4, USART_RDBF_INT, FALSE); //



		    } else  usart4_rx_counter++;

	           // usart_interrupt_enable(USART2, USART_RDBF_INT, FALSE);
  }


	}




void TMR7_GLOBAL_IRQHandler(void)  // not needed
{
  if(tmr_interrupt_flag_get(TMR7, TMR_OVF_FLAG) != RESET)
  {
		//uint16_t	temp=ccr_buf[ccr_counter];
		//uint16_t	temp2=ccr_buf[ccr_counter+1];
	//	if(next_sample_ready==2) temp=ccr2_out; temp2=ccr1_out; // only update if ready
	//	temp=ccr2_out;

		//	dac_dual_data_set(DAC_DUAL_12BIT_RIGHT, temp, temp2);
		//	 dac_dual_software_trigger_generate();
			// ccr_counter++;  // doesnt really do much
		//if (ccr_reset) {ccr_counter=0;ccr_reset=0;}

			// ccr_counter&=127;

    tmr_flag_clear(TMR7, TMR_OVF_FLAG);
  }
}

void DMA1_Channel1_IRQHandler(void) // TX DAC
{
  if(dma_interrupt_flag_get(DMA1_FDT1_FLAG) != RESET)
  {
 //ccr_counter=audio_buffer_size; // ready for next set
	  dac_ready=1;
 //memset(ccr_buf,0,(audio_buffer_size));
 memcpy(ccr_buf,audio_out_buf,(audio_buffer_size*4));  // copy last processed audio
 dma_flag_clear(DMA1_FDT1_FLAG);
  }
}




void DMA1_Channel2_IRQHandler(void)			// RX SPI4
{
  /* add user code begin DMA1_Channel2_IRQ 0 */

  /* add user code end DMA1_Channel2_IRQ 0 */

  if(dma_interrupt_flag_get(DMA1_FDT2_FLAG) != RESET)
  {

    dma_flag_clear(DMA1_FDT2_FLAG);
    dma_channel_enable(DMA1_CHANNEL2, FALSE);
    SPI4_CS_HIGH;
    SPI2_CS_HIGH;
    memcpy(spi_read_pointer,ram_page_read_buf+5,256);
    spi_read_flag=0;


    /* add user code end DMA1_FDT2_FLAG */
  }

  if(dma_interrupt_flag_get(DMA1_HDT2_FLAG) != RESET) //half transfer  , triggers full transfer flag for soem reason
  {

    /* add user code begin DMA1_HDT2_FLAG */
    /* handle half data transfer and clear flag */
   dma_flag_clear(DMA1_HDT2_FLAG);
           /* add user code end DMA1_HDT2_FLAG */
  }

  /* add user code begin DMA1_Channel2_IRQ 1 */

  /* add user code end DMA1_Channel2_IRQ 1 */
}


void DMA1_Channel3_IRQHandler(void)   // TX SPI4
{
  /* add user code begin DMA1_Channel3_IRQ 0 */

  /* add user code end DMA1_Channel3_IRQ 0 */

  if(dma_interrupt_flag_get(DMA1_FDT3_FLAG) != RESET)
  {
    /* add user code begin DMA1_FDT3_FLAG */
    /* handle full data transfer and clear flag */
    dma_flag_clear(DMA1_FDT3_FLAG);
    dma_channel_enable(DMA1_CHANNEL3, FALSE);
    SPI4_CS_HIGH;
    SPI2_CS_HIGH;
    spi_write_flag=0;

    /* add user code end DMA1_FDT3_FLAG */
  }

  /* add user code begin DMA1_Channel3_IRQ 1 */

  /* add user code end DMA1_Channel3_IRQ 1 */
}



/**
  * @}
  */
