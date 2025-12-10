


	void program_change(uint8_t pc_value){  // this is ok , getting garbage from seq

		if (pc_value>total_samples) pc_value=total_samples;
		if(pc_value) sample_select[0]=pc_value*1200;  // try to catch as lots of zero ??
		sample_select[1]=sample_select[0];

		//HAL_Delay(20); // DO NOT REMOVIE , needs this or it goes bad
		memcpy(in_sample_holder,out_bin+sample_select[0],1200);
	//	flash_read(sample_select[1],1200,test_sample ); // grab from ext flash ,needed initially
	//	memcpy(in_sample_holder,test_sample+4,1200);


	}

	void control_change(uint8_t cc , uint8_t value){       // midi control change processing, needs a fifo buffer of som etype to avoid too much process
	//MIDI CC
	//	0 	Bank Select (MSB)  for patch banks , 7-volume, 5-portamento time
	// 73 -Attack ,   72 -Release ,71 -VCF REsonance , 74 -VCF cutoff freq,  91 -Reverb , 84-portamento, 94-detune, 95-phaser, 70-sound variation,
	//92 -tremolo, 75-79 generic fx settings , may use it for delay unit
	if (cc==72)  {ADSR_settings[1]=value&127;memset(envelopes_store,0,256);envelopes_preprocess(0);}// this will have to be fully recalculated ,will be slow
	if(cc==91)  delay_time=value&127;
	if(cc==75)  {		cc_75=value&127;
	if (cc_75>total_samples) cc_75=total_samples;
	if(cc_75) {sample_select[2]=cc_75*1200;
	//HAL_Delay(20); // DO NOT REMOVIE , needs this or it goes bad
	//flash_read(sample_select[2],1200,test_sample ); // grab from ext flash ,needed initially
	memcpy(in_sample_holder_2,out_bin+sample_select[2],1200);
	//memcpy(in_sample_holder_2,test_sample+4,1200); // a second sample for decay
	}}
	if(cc==76)  cc_76=value&127;
	if(cc==77)  cc_77=value&127;

	} // 5, 75,76,77



void midi_incoming(void){     // midi processing when triggered,  not good with time code yet
uint8_t incoming_message[3];    //

			// reasonable

//uint32_t countup=tmr_counter_value_get(TMR6);
		uint16_t start=4;

		memcpy(incoming_message,usart2_rx_buffer,3);


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

		note_trigger=midi_note_hold[1]&127;   // needs zones
		//HAL_GPIO_TogglePin (GPIOC,LED_Pin);
		at32_led_toggle(LED2);
		memset(midi_note_hold,0,4);}  // play note and clear


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

		start=4;

					if (start==4)  // always runs

					{memset(usart2_rx_buffer,0,uart_receive);
					usart_interrupt_enable(USART2, USART_RDBF_INT, TRUE);

					midi_in_clear=0;
					return;

					}// if no info reload ;


}






