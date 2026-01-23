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
#include "math.h"

#include "out.bin.h"     // only enable for initial upload

#include "variables.h"

#include "maincode.h"
#include "flash.h"
#include "audio.h"
#include "midi.h"

uint8_t data_count=0;


/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();

  at32_board_init();

  wk_spim_init();
  dac_config();
  usart_config();

	midi_note_pwm_calculator();  // issues with float

	waves();

	uint16_t i;
	int16_t temp[600];
	uint32_t read_adr= settings_data;

	for (i=0;i<256;i++){   // reading ok now
		//temp[i]=*(uint8_t*)(read_adr);
		all_settings[i]=*(uint8_t*)(read_adr);
		  read_adr += 1;
	}
	if (all_settings[0]!=255)	settings_storage();   // only reads once there is data
	envelopes_preprocess(0);
	envelopes_preprocess(1);
	envelopes_preprocess(2);
	envelopes_preprocess(3);

read_adr= user_data_start +sample_select[0];

for (i=0;i<600;i++){   // reading ok now

	temp[i]=*(int16_t*)(read_adr);
	  read_adr += 2;
}

memcpy(in_sample_holder,temp,1200);

memcpy(in_sample_holder_2,in_sample_holder,1200);
 //    while(usart_flag_get(USART2, USART_RDBF_FLAG) == RESET);       // while receiving


//if (all_settings[0]!=255)	settings_storage(); // check and load settings





  while(1)
  {

	  if( midi_in_clear) {  midi_fifo(usart2_rx_buffer,0); } // write to fifo ,returns 0 if done


	  if (midi_buf_flag)     midi_incoming();  // not totally ok yet



	  	  if (!note_trigger) note_trigger=note_fifo(0, 1);   // enable note if incoming
	  	  if (save_timer>60000) {settings_write_flag=1;settings_storage();flash_settings_write();save_timer=0; }  // saves every ten minutes
	 	  if (next_sample_ready==1) next_sample(); // grab sample
	 	//  if( midi_in_clear) midi_incoming(); // process midi incoming
	 	 if((!ADSR_timer_flag) && ((tmr_counter_value_get(TMR6)) >8200))  {  ADSR_TIM_writer(); ADSR_timer_flag=1;save_timer++;} //process ADSR
	 	  if((ADSR_timer_flag) && ((tmr_counter_value_get(TMR6))<100)) ADSR_timer_flag=0;   //reset   , can miss starts might have sync with notes or trigger more often

	 	  if(zero_cross[0]&& note_trigger){     // sends note to isr when enabled ,fairly reliable , zero cross works ok

	 		// if ((note_trigger>47) ){  {  // this needs to wait for zero cross
	 		  if ((note_trigger>47) ){    // this needs to wait for zero cross

	 				 					  // keyboard split
	 					CNT_list_selected[0]=CNT_list[note_trigger];  // add value for ccr
	 					CNT_list_selected[1]=CNT_list[note_trigger+5];  // add value for ccr ,freq
	 					ADSR_counter_position[0]=0;
	 					ADSR_out_1=envelopes_store[0];   // this reall should be elsewhere
	 					//ADSR_out_1=(ADSR_out_1*current_velocity)>>8;
	 					stutter_flip=0;
	 					stutter_toggle=0;
	 					note_trigger=0;
	 						 					zero_cross[0]=0;
	 		 }}// end of zero cross*/

		 	  if(zero_cross[2] && note_trigger){     // sends note to isr when enabled ,fairly reliable
		 		// if(note_trigger){     // sends note to isr when enabled ,fairly reliable
	 					if (note_trigger<48){
	 					CNT_list_selected[2]=CNT_list[note_trigger];  // add value for ccr ,freq  , 36
	 					ADSR_counter_position[1]=255; 					// trigger note
	 					ADSR_out_2=envelopes_store[255];
	 					//ADSR_out_2=(ADSR_out_2*current_velocity)>>8;

	 					note_trigger=0;
	 					zero_cross[2]=0;
	 					}}// end of zero cross*/

  }  // end while
} // end of main

/**
  * @}
  */
void USART2_IRQHandler(void)  //works
{
  if(usart_interrupt_flag_get(USART2, USART_RDBF_FLAG) != RESET)
  {
    /* read one byte from the receive data register */
	//  usart2_rx_buffer[usart2_rx_counter++] = usart_data_receive(USART2);


	  usart2_rx_buffer[usart2_rx_counter] = usart_data_receive(USART2);  // filter midi channel here first
	  if (usart2_rx_buffer[0]==note_on+midi_channel)  usart2_rx_counter++;   // try to get note on message start , no cc atm ,stops some random  notes
	  if (usart2_rx_buffer[0]==c_change+4)
		  usart2_rx_counter++;
	//  usart_interrupt_enable(USART2, USART_RDBF_INT, FALSE);
	// usart2_rx_buffer[usart2_rx_counter++] = usart_data_receive(USART2);

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
void TMR7_GLOBAL_IRQHandler(void)  // works
{
  if(tmr_interrupt_flag_get(TMR7, TMR_OVF_FLAG) != RESET)
  {
		uint16_t	temp=temp_ccr;;
		uint16_t	temp2=temp_ccr;;
		if(next_sample_ready==2) temp=ccr2_out; temp2=ccr1_out; // only update if ready
	//	temp=ccr2_out;
			next_sample_ready=1;
			dac_dual_data_set(DAC_DUAL_12BIT_RIGHT, temp, temp2);
			 dac_dual_software_trigger_generate();
			 temp_ccr=ccr2_out;

    tmr_flag_clear(TMR7, TMR_OVF_FLAG);
  }
}
/**
  * @}
  */
