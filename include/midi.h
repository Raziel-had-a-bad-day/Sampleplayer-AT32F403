void note_process(void);
#include "math.h"
uint8_t note_fifo(uint8_t incoming,uint8_t read_enable){     // returns last incoming note ,simple fifo

	uint8_t temp=0;
	uint8_t note_fifo_read=0;
	uint8_t count_up=0;

	if (!read_enable) {      // simply count down to -1
		note_fifo_buf[note_fifo_write]=incoming;note_fifo_write=(note_fifo_write+1)&15;
				return 1 ;}   //0,,1,2,3,4

	while ((!temp) && (note_fifo_read<16)) {    // read until finds value then exit
		count_up=(note_fifo_read+note_fifo_write)&15;  // starts from the last written pos +1 for 16 steps
		temp=note_fifo_buf[count_up];note_fifo_buf[count_up]=0;  // clear
		note_fifo_read++;
	}
		return temp;
}



uint8_t midi_fifo(uint8_t* incoming,uint8_t read_enable){     // returns last midi message ,simple fifo


	uint8_t note_fifo_read=0;
	uint8_t count_up=0;

	if (!read_enable) {      // write then reset uart nvic
				memcpy(midi_fifo_buf+midi_fifo_write,incoming,3);
				midi_fifo_buf[midi_fifo_write+3]=1; // sets flag for later
				midi_fifo_write=(midi_fifo_write+4)&31;  // copy 3 bytes into every 4

				midi_buf_flag=1;
				midi_in_clear=0;
				memset(usart2_rx_buffer,0,uart_receive);
				usart_interrupt_enable(USART2, USART_RDBF_INT, TRUE);

				return 0 ;}   //0,,1,2,3,4

	while ((!midi_fifo_temp[3]) && (note_fifo_read<32)) {    // read until finds value then exit
		count_up=(note_fifo_read+midi_fifo_write)&31;  // starts from the last written pos +1 for 16 steps
		memcpy(midi_fifo_temp,midi_fifo_buf+count_up,4);memset(midi_fifo_buf+count_up,0,4);  // clear
		note_fifo_read+=4;
	}
		return midi_fifo_temp[3];
}


	void program_change(uint8_t pc_value){  // this is ok , getting garbage from seq

		if (pc_value>total_samples) pc_value=total_samples;
		if(pc_value) sample_select[0]=pc_value*1200;  // try to catch as lots of zero ??
		sample_select[1]=sample_select[0];

		uint16_t i=0;
		int16_t temp[600];
		uint32_t read_adr=  user_data_start+sample_select[0];


	for (i=0;i<600;i++){   // reading ok now

		temp[i]=*(int16_t*)(read_adr);
		  read_adr += 2;
	}
		memcpy(in_sample_holder,temp,1200);
		memcpy(in_sample_holder_2,temp,1200);
	}

	void control_change(uint8_t cc , uint8_t value){       // midi control change processing, needs a fifo buffer of som etype to avoid too much process
	//MIDI CC
	//	0 	Bank Select (MSB)  for patch banks , 7-volume, 5-portamento time
	// 73 -Attack ,   72 -Release ,71 -VCF REsonance , 74 -VCF cutoff freq,  91 -Reverb , 84-portamento, 94-detune, 95-phaser, 70-sound variation,
	//92 -tremolo, 75-79 generic fx settings , may use it for delay unit
	if (cc==72)  {ADSR_settings[1]=value&127;memset(envelopes_store,0,256);envelopes_preprocess(0);}// this will have to be fully recalculated ,will be slow
	// pot 1


	if(cc==5)  delay_time=value&127; // delay length
	//pot2


	if((cc==75) && (cc_75!=value&127))  {		cc_75=value&127;
	if (cc_75>total_samples) cc_75=total_samples;
	if(cc_75) {sample_select[0]=cc_75*1200;
	sample_select[1]=sample_select[0];
	uint16_t i;
	int16_t temp[600];
	uint32_t read_adr= settings_data;
	read_adr= user_data_start +sample_select[0];

	for (i=0;i<600;i++){   // reading ok now

		temp[i]=*(int16_t*)(read_adr);
		  read_adr += 2;
	}
	memcpy(in_sample_holder,temp,1200);
	memcpy(in_sample_holder_2,temp,1200);//HAL_Delay(20); // DO NOT REMOVIE , needs this or it goes bad
	//flash_read(sample_select[2],1200,test_sample ); // grab from ext flash ,needed initially
	//memcpy(in_sample_holder_2,out_bin+sample_select[2],1200);
	//memcpy(in_sample_holder_2,test_sample+4,1200); // a second sample for decay
	}}
	if(cc==76)  cc_76=value&127;// feedback
	if(cc==77)  cc_77=value&127;// pot 4 , stutter or second note pitch
	if(cc==78)  cc_78=value&127;// pot 4 , stutter or second note pitch
	lfo1_rate=(cc_77<<4)+1;
	float depth=cc_78;

	depth=pow(1.04,depth);  // smooth log 1.04 is about the best for 128,  1.022 for 255
	lfo1_depth=depth;


	if(lfo1_depth>127) lfo1_depth=127;
	//lfo1_depth=127-lfo1_depth;



	} //  cc= 72,5,76,77,78, these work for now ,       79,80   current available controllers



void midi_incoming(void){     // midi processing when triggered,  not good with time code yet
uint8_t incoming_message[3];    //

			// reasonable  , might try to catch during irq

		uint16_t start=4;
		if (midi_fifo(0,1))
		{memcpy(incoming_message,midi_fifo_temp,3);
		memset(midi_fifo_temp,0,4);

		} else {midi_buf_flag=0;return;}    // quit if no data


		uint8_t message1[4]={0,0,0,0};
		uint8_t message2[4]={0,0,0,0};   // last bit sets finished reading
		uint8_t message3[4]={0,0,0,0};


		switch(midi_note_hold[3]) {   // fills missing midi note one

		case 1: midi_note_hold[2]=incoming_message[0];midi_note_hold[3]=4;break;
		case 2: midi_note_hold[1]=incoming_message[0];midi_note_hold[2]=incoming_message[1];midi_note_hold[3]=4;break;
		default:break;
		}

		switch(midi_pc_hold[3]) {   // fills missing midi pc data

		case 1: midi_pc_hold[2]=incoming_message[0];midi_pc_hold[3]=4;break;
		case 2: midi_pc_hold[1]=incoming_message[0];midi_pc_hold[2]=incoming_message[1];midi_pc_hold[3]=4;break;
		default:break;
		}

		switch(midi_cc_hold[3]) {   // fills missing midi cc data

		case 1: midi_cc_hold[2]=incoming_message[0];midi_cc_hold[3]=4;break;
		case 2: midi_cc_hold[1]=incoming_message[0];midi_cc_hold[2]=incoming_message[1];midi_cc_hold[3]=4;break;
		default:break;
		}

		if (incoming_message[0]>127){  // test first byte , this assumes that there are no extra leftovers from timecode etc, will deal that later
		switch(incoming_message[0]){
		case note_on+midi_channel:memcpy(midi_note_hold,incoming_message,3); midi_note_hold[3]=4;break;  // midi note full
		case p_change+midi_channel:memcpy(midi_pc_hold,incoming_message,3);midi_pc_hold[3]=4; break; // this one is finished
		case c_change+midi_channel:memcpy(midi_cc_hold,incoming_message,3);midi_cc_hold[3]=4; break;
		default:break;

		}

		}


		if (incoming_message[1]>127){	// test second byte
			switch(incoming_message[1]){
			case note_on+midi_channel:message1[0]=incoming_message[1];message1[1]=incoming_message[2];message1[3]=1;break;  // midi note partial
			case p_change+midi_channel:message2[0]=incoming_message[1];message2[1]=incoming_message[2];message2[3]=1; break; // cc partial ,needs 1 more value
			case c_change+midi_channel:message3[0]=incoming_message[1];message3[1]=incoming_message[2];message3[3]=1; break;
			default:break;

			}

			}


		if (incoming_message[2]>127){			// test third byte
				switch(incoming_message[1]){
				case note_on+midi_channel:message1[0]=incoming_message[1];message1[3]=2;break;  // midi note partial, needs 2 more values
				case p_change+midi_channel:message2[0]=incoming_message[1];message2[3]=2; break; // cc partial ,needs 2 more values
				case c_change+midi_channel:message3[0]=incoming_message[1];message3[3]=2; break;
				default:break;

				}

				}


		if (midi_note_hold[3]==4)   { //HAL_GPIO_TogglePin (GPIOC,LED_Pin);
			if (midi_note_hold[1]<80){   // too many high values still
				note_fifo(midi_note_hold[1],0);current_velocity=midi_note_hold[2]&127;
		//HAL_GPIO_TogglePin (GPIOC,LED_Pin);
		at32_led_toggle(LED2);
		memset(midi_note_hold,0,4);}}  // play note and clear


		if (midi_pc_hold[3]==4)   {

			memcpy(test_hold,serial_temp,3);
			program_change(midi_pc_hold[1]);
			memset(midi_pc_hold,0,4);} // pc_change and clear

		if (midi_cc_hold[3]==4) { control_change(midi_cc_hold[1],midi_cc_hold[2]);  memset(midi_cc_hold,0,4); }  // cc receive

		if ((message1[3]) || (message2[3]) || (message3[3])   )  {  // if a second message comes it might get overwritten


			memcpy(midi_note_hold,message1,4);  // store if not finished , will be cleared by the next uart receive
			memcpy(midi_pc_hold,message2,4);  // store if not finished , will be cleared by the next uart receive
			memcpy(midi_cc_hold,message3,4);
			start=4;   // empty ,return
		}




}

void note_process(void){ // deals with assigning notes for sounds , basic for now
	uint8_t i;
	uint8_t current_poly=2;// sets number of notes to process for now
	for (i=0;i<current_poly;i++){
	if ((sound_triggers[i]==0) && zero_cross[i]) sound_triggers[i]=note_trigger; // adds note_trigger to sound on zero cross
	 if(zero_cross[i] && (sound_triggers[i]==128)) zero_cross[i]=0; // clears zero cross when it retriggers

	  	  	  if(zero_cross[i]&& sound_triggers[i]){     // sends note to isr when enabled ,fairly reliable , zero cross works ok


				 					  // keyboard split
					CNT_list_selected[i]=CNT_list[sound_triggers[i]];  // first note
					if(i==1 )	CNT_list_selected[i]=CNT_list[(sound_triggers[i]+cc_77)&127];  // modified note
					ADSR_counter_position[i]=0;
					ADSR_out[i]=envelopes_store[i];   // this really should be elsewhere
					//ADSR_out_1=(ADSR_out_1*current_velocity)>>8;
					stutter_flip=0;
					stutter_toggle=0;
					sound_triggers[i]=128;
					zero_cross[i]=0;
		 }// end of zero cross*/
	}



	  		if ((sound_triggers[0]>127) && (sound_triggers[1]>127)) {sound_triggers[0]=0;sound_triggers[1]=0;note_trigger=0; } // after playing both notes its off







}



