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
#include "out.bin.h"

#include "variables.h"

#include "maincode.h"

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


  dac_config();
  usart_config();

	midi_note_pwm_calculator();  // issues with float
	envelopes_preprocess(0);
	envelopes_preprocess(1);
	envelopes_preprocess(2);
	envelopes_preprocess(3);
	waves();

	uint16_t i;
	int16_t temp;
	uint32_t read_adr= 0x8020000;
	//memcpy(temp,sine12bit,32);
	//flash_read(0x8020000,in_sample_holder,600);

/*for (i=0;i<600;i++){

	//temp=(sine12bit[i&31]<<4)-33767;

	in_sample_holder[i]=*(int16_t*)(read_adr);
	  read_adr += 2;
}*/

memcpy(in_sample_holder,out_bin,1200);


memcpy(in_sample_holder_2,in_sample_holder,1200);
 //    while(usart_flag_get(USART2, USART_RDBF_FLAG) == RESET);       // while receiving






  while(1)
  {




	//  if ( (tmr_counter_value_get(TMR6)&127)==0)  next_sample();
	 // if ( spi_dma_ready==1)  {memcpy(in_sample_holder,test_sample+4,1200); spi_dma_ready=0;}
	 	  if (next_sample_ready==1) next_sample(); // grab sample
	 	  if( midi_in_clear) midi_incoming(); // process midi incoming
	 	 if((!ADSR_timer_flag) && ((tmr_counter_value_get(TMR6)) >8200))  {  ADSR_TIM_writer(); ADSR_timer_flag=1;} //process ADSR
	 	  if((ADSR_timer_flag) && ((tmr_counter_value_get(TMR6))<100)) ADSR_timer_flag=0;   //reset

	 	  if(zero_cross){     // sends note to isr when enabled ,fairly reliable
	 			if(note_trigger ){    // this needs to wait for zero cross

	 				if (note_trigger>72) note_pitch_shift=-12+(note_trigger-72);  // stary from -12
	 				if ((note_trigger>47) && (note_trigger<73))							{   // keyboard split
	 					CNT_list_selected[0]=CNT_list[note_trigger+note_pitch_shift];  // add value for ccr
	 					CNT_list_selected[1]=CNT_list[note_trigger+5+note_pitch_shift];  // add value for ccr ,freq
	 					ADSR_counter_position[0]=0;
	 					ADSR_out_1=envelopes_store[0];
	 					stutter_flip=0;
	 					stutter_toggle=0;
	 				}

	 					if (note_trigger<48){
	 					CNT_list_selected[2]=CNT_list[note_trigger];  // add value for ccr ,freq  , 36
	 					ADSR_counter_position[1]=256; 					// trigger note
	 					ADSR_out_2=envelopes_store[256];
	 				}

	 					note_trigger=0;
	 					zero_cross=0;
	 				}
	 	  } // end of zero cross*/
  }
}

/**
  * @}
  */
void USART2_IRQHandler(void)  //works
{
  if(usart_interrupt_flag_get(USART2, USART_RDBF_FLAG) != RESET)
  {
    /* read one byte from the receive data register */
    usart2_rx_buffer[usart2_rx_counter++] = usart_data_receive(USART2);

    if(usart2_rx_counter == usart_buffer_size)
    {
      /* disable the usart2 receive interrupt */
      usart_interrupt_enable(USART2, USART_RDBF_INT, FALSE);
      midi_in_clear=1;usart2_rx_counter=0;
    }
  }


}
void TMR7_GLOBAL_IRQHandler(void)  // works
{
  if(tmr_interrupt_flag_get(TMR7, TMR_OVF_FLAG) != RESET)
  {
		uint32_t	temp=2048;;
		if(next_sample_ready==2) temp=ccr2_out;  // only update if ready
		ccr2_out=temp;
			next_sample_ready=1;
			dac_dual_data_set(DAC_DUAL_12BIT_RIGHT, ccr2_out, ccr2_out);
			 dac_dual_software_trigger_generate();

    tmr_flag_clear(TMR7, TMR_OVF_FLAG);
  }
}
/**
  * @}
  */
