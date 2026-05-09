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
				memset(usart2_rx_buffer,0,uart_receive);  // clear for irq
				usart_interrupt_enable(USART2, USART_RDBF_INT, TRUE); // restart irq

				return 0 ;}   //0,,1,2,3,4

	while ((!midi_fifo_temp[3]) && (note_fifo_read<32)) {    // read until finds value then exit
		count_up=(note_fifo_read+midi_fifo_write)&31;  // starts from the last written pos +1 for 16 steps
		memcpy(midi_fifo_temp,midi_fifo_buf+count_up,4);memset(midi_fifo_buf+count_up,0,4);  // clear
		note_fifo_read+=4; // goes up by 4 bytes
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

	void control_change(uint8_t channel,uint8_t cc , uint8_t value){       // midi control change processing, needs a fifo buffer of som etype to avoid too much process
	//MIDI CC
	//	0 	Bank Select (MSB)  for patch banks , 7-volume, 5-portamento time
	// 73 -Attack ,   72 -Release ,71 -VCF REsonance , 74 -VCF cutoff freq,  91 -Reverb , 84-portamento, 94-detune, 95-phaser, 70-sound variation,
	//92 -tremolo, 75-79 generic fx settings , may use it for delay unit
	if (channel==4){

		if (value<4) value=0; //compansate for bad pots
		if (cc==72)  {ADSR_settings[1]=value&127;memset(envelopes_store,0,256);envelopes_preprocess(0);}// this will have to be fully recalculated ,will be slow
	// pot 1
	if(cc==5)  delay_time=value&127; // delay length
	//pot2
	if (cc==7) cc_7=value&127; // audio level
 if 	(cc==19) master_tune=value&127;  // tune

	if(cc==77)  cc_77=value&127;// pot 4 , stutter or second note pitch
	if(cc==78)  cc_78=value&127;// pot 4 , stutter or second note pitch

	lfo1_rate=(cc_77<<4)+1;
	float depth=cc_78;
	depth=pow(1.04,depth);  // smooth log 1.04 is about the best for 128,  1.022 for 255
	lfo1_depth=depth;
	if(lfo1_depth>127) lfo1_depth=127;
	//lfo1_depth=127-lfo1_depth;
	} // end of main channel
	if (channel!=4){ // not picking up
		if(cc==77){
		if (samples_store[value>>3].used)  current_playing_sample=value>>3;}


		if(cc==76)  cc_76=value&127;// feedback
		samples_store[current_playing_sample].speed=cc_76;
	} //end of samples/drums cc

	} //  end off cc process



void midi_incoming(void){     // midi processing when triggered,  not good with time code yet
uint8_t incoming_message[3];    //
// Define buffers as arrays (much easier to handle)

			// reasonable  , might try to catch during irq

		uint16_t start=4;
		if (midi_fifo(0,1))
		{memcpy(incoming_message,midi_fifo_temp,3);   // still has midi channel info
		memset(midi_fifo_temp,0,4);

		} else {midi_buf_flag=0;return;}    // quit if no data


		uint8_t message1[4]={0,0,0,0};
		uint8_t message2[4]={0,0,0,0};   // last bit sets finished reading
		uint8_t message3[4]={0,0,0,0};



		// Fill missing data from previous partial messages
		for(int i = 0; i < 3; i++) {
		    switch(midi_hold[i][3]) {
		        case 1: midi_hold[i][2] = incoming_message[0]; midi_hold[i][3] = 4; break;
		        case 2: memcpy(midi_hold[i], incoming_message, 3); midi_hold[i][3] = 4; break;
		    }
		}

		// === New MIDI Message Handling ===
		if (incoming_message[0] > 127) {
		    switch(incoming_message[0]) {
		        case note_on + midi_channel:
		        case note_on + 9: // extra drum channel
		            memcpy(midi_hold[NOTE], incoming_message, 3);
		            midi_hold[NOTE][3] = 4;
		            break;

		        case p_change + midi_channel:
		            memcpy(midi_hold[PC], incoming_message, 3);
		            midi_hold[PC][3] = 4;
		            break;

		        case c_change + midi_channel:
		        case c_change + 9:  // extra cc channel
		            memcpy(midi_hold[CC], incoming_message, 3);
		            midi_hold[CC][3] = 4;
		            break;
		    }
		}

		// Second byte is status byte
		if (incoming_message[1] > 127) {
		    switch(incoming_message[1]) {
		        case note_on + midi_channel:
		        case note_on + 9:
		            memcpy(midi_msg[NOTE], &incoming_message[1], 2);
		            midi_msg[NOTE][3] = 1;
		            break;

		        case p_change + midi_channel:
		            memcpy(midi_msg[PC], &incoming_message[1], 2);
		            midi_msg[PC][3] = 1;
		            break;

		        case c_change + midi_channel:
		        case c_change + 9:
		            memcpy(midi_msg[CC], &incoming_message[1], 2);
		            midi_msg[CC][3] = 1;
		            break;
		    }
		}

		// Third byte is status byte
		if (incoming_message[2] > 127) {
		    switch(incoming_message[1]) {
		        case note_on + midi_channel:
		        case note_on + 9:
		            midi_msg[NOTE][0] = incoming_message[1];
		            midi_msg[NOTE][3] = 2;
		            break;

		        case p_change + midi_channel:
		            midi_msg[PC][0] = incoming_message[1];
		            midi_msg[PC][3] = 2;
		            break;

		        case c_change + midi_channel:
		        case c_change + 9:
		            midi_msg[CC][0] = incoming_message[1];
		            midi_msg[CC][3] = 2;
		            break;
		    }
		}


		// === Process complete messages ===
		if (midi_hold[NOTE][3] == 4)          // Note On / Note message ready
		{
		    if (midi_hold[NOTE][1] < 80)      // velocity or note range filter
		    {
		        if (midi_hold[NOTE][0] == 148)   // Channel 5 (0x94)
		        {
		            note_fifo(midi_hold[NOTE][1], 0);
		            current_velocity = midi_hold[NOTE][2] & 127;
		            at32_led_toggle(LED2);
		            memset(midi_hold[NOTE], 0, 4);
		        }
		    }

		    if (midi_hold[NOTE][0] == 153)    // Channel 10 (0x99) - Drums
		    {
		        memcpy(drum_note_hold, midi_hold[NOTE], 3); // not doing anything
		        current_playing_sample=drum_note_hold[1]&15;  //select sample
		        if (samples_store[current_playing_sample].used) ADSR_counter_position[2] = 0; // only trigger if there is data

		        at32_led_toggle(LED2);
		        memset(midi_hold[NOTE], 0, 4);
		    }
		}

		// === Program Change ===
		if (midi_hold[PC][3] == 4)
		{
		    program_change(midi_hold[PC][1]);
		    memset(midi_hold[PC], 0, 4);
		}

		// === Control Change ===
		if (midi_hold[CC][3] == 4)
		{
		    control_change(midi_hold[CC][0]-176, midi_hold[CC][1], midi_hold[CC][2]);
		    memset(midi_hold[CC], 0, 4);
		}

		// === Handle partial messages (save for next round) ===
		if (midi_msg[NOTE][3] || midi_msg[PC][3] || midi_msg[CC][3])
		{
		    memcpy(midi_hold[NOTE], midi_msg[NOTE], 4);
		    memcpy(midi_hold[PC],  midi_msg[PC],  4);
		    memcpy(midi_hold[CC],  midi_msg[CC],  4);

		    // Clear the partial buffers
		    memset(midi_msg[NOTE], 0, 4);
		    memset(midi_msg[PC],  0, 4);
		    memset(midi_msg[CC],  0, 4);

		    start = 4;   // empty, return
		}




}

void note_process(void){ // deals with assigning notes for sounds , basic for now
	uint8_t i;
	uint8_t current_poly=2;// sets number of notes to process for now , samples done elsewhere
	int note_2=0;
	for (i=0;i<current_poly;i++){


		if ((sound_triggers[i]==0) && zero_cross[i]) sound_triggers[i]=note_trigger; // adds note_trigger to sound on zero cross
		if(zero_cross[i] && (sound_triggers[i]==128)) zero_cross[i]=0; // clears zero cross when it retriggers

		if(zero_cross[i]&& sound_triggers[i]){     // sends note to isr when enabled ,fairly reliable , zero cross works ok


				 					  // keyboard split
					CNT_list_selected[i]=CNT_list[sound_triggers[i]];  // first note


					if(i==1 )	{ note_2=(sound_triggers[i]-36)+cc_78;
					if (note_2<6) note_2=6;
					if (note_2>96) note_2=96;

						CNT_list_selected[i]=CNT_list[note_2];  }// modified second note
					CNT_list_selected[i]=(CNT_list_selected[i]*master_tune)>>6;  // tune

					ADSR_counter_position[i]=0+(i*256); // set to start, enables run

				//	if(i==2) ADSR_out[2]=127;  //drums
					stutter_flip=0;
					stutter_toggle=0;
					sound_triggers[i]=128; // wtf is this
					zero_cross[i]=0;
		 }// end of zero cross*/
	}



	  		if ((sound_triggers[0]>127) && (sound_triggers[1]>127)) {sound_triggers[0]=0;sound_triggers[1]=0;note_trigger=0; } // after playing both notes its off
	  		if ((sound_triggers[2]>127) ) {sound_triggers[2]=0;note_trigger=0; } //drums , this needs a different trigger

	}



