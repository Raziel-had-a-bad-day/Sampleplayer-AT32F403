void next_sample(void){


	delay_pointer[0]++;
	delay_pointer[0] &=(delay_buffer_size-1);
	//delay_pointer[0] &=8191;
	uint32_t counter=wav_pointer[0]>>8;  // /256
	int32_t temp=0;
	uint8_t i;
	int32_t temp3=0;
	int32_t temp2=0;
	int32_t temp_sample=0;
	uint16_t delay=delay_pointer[0];
	uint16_t delay_pointer=delay+(delay_time*64);  // set max delay time

	uint16_t delay_2=delay_pointer&(delay_buffer_size-1);
	uint16_t delay_3=(delay_pointer+64)&(delay_buffer_size-1);
	//float feedback=cc_76*0.007;
	int16_t* pointer = in_sample_holder;
	int16_t* pointer2 = in_sample_holder_2;
	int32_t feedback=cc_76;
	uint16_t temp_out;
	//uint32_t multi=8;
	//if(ADSR_counter_position[0]>cc_76)  pointer=in_sample_holder_2;
	//ADSR_out_1=64000;   // should play a note non stop
	/////////// sound 0 ///////////////
	if (counter>(cycle_length-1)) counter=counter&127; // just in case

	counter=(counter*2);
	temp_sample=pointer[counter];   //casting the correct way
	temp2=((temp_sample*ADSR_out_1)>>17);   // modify signal with adsr signed * unsigned

	//////////////////    sound 1  //////////////
	counter=wav_pointer[1]>>8;
	if (counter>(cycle_length-1)) counter=counter&127; // just in case
	counter=(counter*2);
	temp_sample=pointer2[counter];
	temp=((temp_sample*ADSR_out_1)>>17);
	//////////////////   sound 2  //////////
	counter=wav_pointer[2]>>8;
	if (counter>(cycle_length-1)) counter=counter&127; // just in case

	counter=(counter*2);
	temp_sample=pointer[counter];
	temp3=((temp_sample*ADSR_out_2)>>17);

	////////   mixer  ////


	temp=((temp2+temp+temp3  ));

//	if (temp>(1<<22))     {multi-=4; }

	temp*=output_gain;  // separate control for each , this on eis about 0.7 with 3 notes

	if (temp>(1<<15)) output_gain*=0.9;

	if (stutter_flip) temp=0;

	temp3=temp;
#define SHIFT 7     // ×128 / ÷128
	if (delay_time) {

		//temp=(temp*(128-(feedback/4)))+(delayed*feedback);  // reduces signal of feedback

		//temp=temp*(128-(feedback/4))+(delayed*feedback);  // reduces signal of feedback
		//if ((temp>32767) || (temp<-32767))  {output_gain*=0.9;}

		int32_t delayed = delay_buffer[delay];
		int32_t delayed_2 = delay_buffer[delay_3];
		int32_t fb_contrib = delayed * (int32_t)feedback;
		int32_t accumulator = (int32_t)temp* (128-(feedback/4));
		accumulator += fb_contrib;
		temp3=accumulator>>SHIFT;
		temp3*=output_gain2;
		if (temp3>(1<<15)) output_gain2*=0.8;
		if (temp3>(1<<14)) output_gain2*=0.9; // delay input limiter

		delay_filter=(delay_filter+temp3)/2;

		delay_buffer[delay_2]=delay_filter; // write back stops here

		int32_t dry  = (int32_t)temp   * 43;
		int32_t wet  =delayed      * 85;
		int32_t mix  = dry + wet;
		int32_t wet_2  =delayed_2      * 85;
		int32_t mix_2  = dry + wet_2;
		temp3=mix>>7;
		temp=mix>>7;

	}  // Delay read






/*
if (delay_time) {
    int32_t delayed = delay_buffer[delay];           // old value

    // Write new value into delay line
    int32_t fb_contrib = delayed * (int32_t)feedback;           // 0..127 * sample
    int32_t accumulator = (int32_t)temp << SHIFT;               // input ×128
    accumulator += fb_contrib;

    int16_t new_sample = (int16_t)(accumulator >> SHIFT);
    // optional: soft saturate if you want extra safety
    // if (new_sample > 32767) new_sample = 32767;
    //if (new_sample < -32768) new_sample = -32768;
	temp=new_sample;
    temp*=output_gain2;
	if (temp>(1<<15)) output_gain2*=0.7;
	if (temp>(1<<14)) output_gain2*=0.9;

	delay_buffer[delay_2] = temp;

    // Output = dry + wet × delayed
    // Using ~2/3 wet to match your 85/128 feeling
    int32_t dry  = (int32_t)temp   * 43;
    int32_t wet  = delayed         * 85;
    int32_t mix  = dry + wet;



    temp = (int16_t)(mix >> SHIFT);
}
*/









	//temp_out=(temp>>5)+2048;
	temp=(temp*current_velocity)>>12; // basic note velocity , not exact based on last value sent
	//temp=temp>>5;
	temp3=(temp3*current_velocity)>>12;
	//ccr1_out=temp3+2048;// to unsigned
	ccr2_out=temp+2048;// to unsigned
	ccr1_out=temp3+2048;// to unsigned

	for (i=0;i<poly;i++){ // advance data pointer, for freq generation , all notes

		if (wav_pointer[i]>wav_multi) {wav_pointer[i]=wav_pointer[i]-wav_multi;zero_cross[i]=1; }  else wav_pointer[i]=wav_pointer[i]+CNT_list_selected[i];


	}
	//ADSR_counter_position[0]=0;
	//ADSR_out_1=envelopes_store[0];
	next_sample_ready=2;
		}




void ADSR_TIM_writer(void){   // single note for now  20ms ,16 bit ,could be smoother , also causes gaps

	//cc_77=0;
	if (stutter_toggle>=stutter_rate) {stutter_toggle=0;stutter_flip=!stutter_flip;} else stutter_toggle++;

	if (cc_77) stutter_rate=cc_77^1; else {stutter_rate=0;stutter_toggle=0;stutter_flip=0;}
	//uint32_t countup=tmr_counter_value_get(TMR6);
	uint16_t counter=ADSR_counter_position[0];

	ADSR_out_1=envelopes_store[counter];   // data out for pwm
	if (output_gain<0.7) output_gain*=1.000001;  // regain
	if (output_gain2<1) output_gain2*=1.000001;  // regain
	//if(multi<128) multi++;

	if ((counter) && (!ADSR_out_1))  counter=255;  // force 0 if no signal
	if (counter>254) counter=255; else counter++; // stop at the end
	ADSR_counter_position[0]=counter;
	/////////////////////
	counter=ADSR_counter_position[1];
	ADSR_out_2=envelopes_store[counter];   // data out for pwm
	if ((counter) && (!ADSR_out_2))  counter=511;  // force 0 if no signal
	if (counter>510) counter=511; else counter++; // stop at the end
	ADSR_counter_position[1]=counter;






}


