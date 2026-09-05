
void delay_calc(void);
static inline int16_t resample_hermite(const int16_t* sample_data, uint32_t phase);
static inline int16_t resample_linear(const int16_t* sample_data,   // your sample in RAM/Flash
                                       uint32_t phase);               // 16.16 fixed point phase
static inline int16_t resample_hermite_float(const int16_t* sample_data, uint32_t phase);
int16_t resample_hermite_loop(const int16_t* sample_data,
                              uint32_t sample_length,      // length in samples
                              uint32_t* phase,             // pointer to phase accumulator
                              uint32_t increment)    ;
int16_t resample_linear_oneshot(const int16_t* sample_data,
                                uint32_t sample_length,
                                uint32_t* phase,
                                uint32_t increment);
int16_t resample_hermite_oneshot(const int16_t* sample_data,
                                 uint32_t sample_length,
                                 uint32_t* phase,          // 16.16 fixed point
                                 uint32_t increment);

void next_sample(void){  // this runs always , sound in generated when ADSR_out is on , wav_pointer shows sample pos in sample holder

	uint16_t delay_adder=32;
	if (delay_pointer[0]<256) delay_adder=0;
	uint32_t counter=wav_pointer[0]>>8;  // click on the first read
	uint32_t counter2=wav_pointer[1]>>8;

	//uint32_t one_shot_counter=one_shot_position &((audio_buffer_size*65536)-1);// phase

	uint8_t phase_lfo=(next_sample_tracker+(63-((lfo1_out)>>4)))&63;
	int32_t phase2=0;
	int32_t phase1=0;
	uint32_t one_shot_counter=one_play[0].position; // 63
	//uint32_t one_shot_counter=one_shot_position &((audio_buffer_size*65536)-1);// phase

	int32_t temp=0;
	uint8_t i;
	int32_t temp3=0;
	int32_t temp2=0;
	int32_t temp4=0;
	int32_t temp5=0;
	int32_t temp6=0;
	uint16_t phaser=lfo1_out;
	int32_t temp_sample=0;
	uint8_t divider=0;
	int16_t* pointer = in_sample_holder;
	int16_t* pointer2 = in_sample_holder_2;


	//uint32_t pointer3=SPIM_START_ADDR+one_shot_pointer;
	//int32_t pointer3=user_data_start+one_shot_pointer;


	int32_t feedback=cc_76;
	uint16_t temp_out;
	//uint32_t multi=8;
	//if(ADSR_counter_position[0]>cc_76)  pointer=in_sample_holder_2;
	//ADSR_out_1=64000;   // should play a note non stop
	/////////// sound 0 ///////////////
	if (counter>(cycle_length-1)) {counter=599;overload_flag++;} // just in case


	phaser=counter+phaser;phaser&=511;  // this could be using different lfo shapes
//	if(phaser>599) phaser=phaser-599;phaser&=511;
	counter=(counter*2);
	temp_sample=pointer[counter];   //casting the correct way
	temp3=((temp_sample*ADSR_out[0])>>15);   // modify signal with adsr signed * unsigned
	temp_sample=pointer[counter2*2];
	temp=((temp_sample*ADSR_out[1])>>19);   // quieter


	//temp_sample=(temp_sample+pointer[phaser<<1])/2;


	//temp=temp_sample; //testing only



	//////////////////    sound 1  //////////////


/*	counter=wav_pointer[1]>>8;
	if (counter>(cycle_length-1)) counter=599; // just in case
	counter=(counter*2);
	temp_sample=pointer2[counter];

	temp=((temp_sample*ADSR_out[1])>>17);*/
	//////////////////   one shot wave playback   //////////

	//temp2=resample_hermite_oneshot(flash_sample_buf,audio_buffer_size,&one_play_counter,one_play_playback_rate);
	//temp2=resample_hermite_loop(flash_sample_buf,audio_buffer_size,&one_play_counter,(1<<16));


	for (int var = 0; var < poly_limit; ++var) {
	if ((current_playing_sample[var]==1) ) {temp2+=(sample_grab(var)>>current_ducking_level[var]);			;divider++;} // might have to expand
	}  // might use ducking for all audio level control


	//temp2=temp2*(4-divider);
	//temp2=temp2*2;
	//temp2=resample_hermite(flash_sample_buf,one_shot_counter);// 305/257us
	//temp2=resample_hermite_float(flash_sample_buf,one_shot_counter);// 328/257us
	//temp2=((flash_sample_buf[next_sample_tracker]*ADSR_out[2])>>7); //246/198  us
	//temp2=flash_sample_buf[next_sample_tracker];
	//temp2=flash_sample_buf[one_shot_counter>>16];
    filter_accus[0]=((temp2*freq_point[0])+(filter_accus[0]*freq_point[1]))>>15;
    filter_accus[1]=(((filter_accus[0]*freq_point[0])+(filter_accus[1]*freq_point[1]))>>15); //1
    filter_accus[2]=(( filter_accus[1]*freq_point[0])+(filter_accus[2]*freq_point[1]))>>15;
    filter_accus[3]=(((filter_accus[2]*freq_point[0])+(filter_accus[3]*freq_point[1]))>>15); //1
    temp2=filter_accus[3];
	//temp2=(temp2*ADSR_out[2])>>15;  // might control initial level from adsr_out to control clipping
	////////   mixer  ////

	//temp=temp2; //testing
	//temp=((temp3+temp)); // only with fx
	//temp=(temp*current_velocity)>>7;
	temp=temp+(temp2); // no right shift yet
//	if (temp>(1<<22))     {multi-=4; }

	//temp*=output_gain;  // separate control for each , this on eis about 0.7 with 3 notes

	//if (temp>(1<<15)) output_gain*=0.9;  //  needs to be near mixer

	//if (temp2>(1<<10)) side_gain*=0.9999;  //sidechain , this can be elsewhere
	//if (temp2>(1<<2))    sidechain_accu=((sidechain_accu*255)+temp2)>>8;  // dc accu ,slow rise

	//if(side_gain<0.5) side_gain=0.5;
	//side_gain=1-side_gain;

	//temp*=side_gain;

	if (stutter_flip) temp=0;


		temp3=temp;
feedback=50; // testing
//delay_time=0;  //testing
#define SHIFT 7     // ×128 / ÷128
	if (delay_time>2) {     // bit heavy ,
		//also needs an incoming limiter
		//temp=(temp*(128-(feedback/4)))+(delayed*feedback);  // reduces signal of feedback

		//temp=temp*(128-(feedback/4))+(delayed*feedback);  // reduces signal of feedback
		//if ((temp>32767) || (temp<-32767))  {output_gain*=0.9;}


		//int32_t delayed = (int16_t) ram_read(delay);  // major slow down needs to be different
		int32_t delayed = ram_out[next_sample_tracker]*4;  // for reading  , up to 128 samples
		int32_t delayed_2 = ram_out[(next_sample_tracker+delay_adder)]*4;
		//int32_t delayed_2 =delayed;
		int32_t fb_contrib = delayed * (int32_t)feedback;
		int32_t accumulator = (int32_t)temp* (128-(feedback/4));  // incoming
		accumulator += fb_contrib;
		temp4=accumulator>>SHIFT;
		//temp3*=output_gain2; //
		//if (temp3>(1<<15)) output_gain2*=0.9;
	//	if (temp3>(1<<14)) output_gain2*=0.9; // delay input limiter

		delay_filter=(delay_filter+temp4)/8; // smoothing

		ram_in[next_sample_tracker]=delay_filter; // write back stops here, maybe lower signal and then gain
		//ram_write(delay_2,(int16_t) delay_filter); // write back stops here
		int32_t dry  = (int32_t)temp   * 50;   // these can be elminated
		int32_t wet  =delayed      * 50;
		int32_t mix  = dry + wet;
		int32_t wet_2  =delayed_2      * 50;
		int32_t mix_2  = dry + wet_2;
		temp3=mix>>6;  // L
		temp=mix_2>>6; // R



	}  // Delay read


	// stereo flanger


	if (temp>(1<<15)) {audio_gain_cut[0]++;audio_gain_cut[1]++;}
	if (temp3>(1<<15)) {audio_gain_cut[0]++;audio_gain_cut[1]++;}


	if (phaser_enable){  // turn off on 0
	phase_delay [next_sample_tracker]=temp2;
	phase1=(phase_delay[phase_lfo]+temp2)/2;
	phase2=(phase_delay[(32+phase_lfo)&63]+temp2)/2;
	if (phase2>(1<<15)||phase1>(1<<15) ) audio_gain_cut[2]++;


	temp+=phase1;// mix back

	temp3+=phase2;
	}

	temp=(temp)>>(4+phaser_enable); // basic note velocity , not exact based on last value sent
	//temp=temp>>5;

		temp3=temp3>>(4+phaser_enable);
		//temp3=sine_testing[next_sample_tracker];temp=temp3;   // grab sample from flash
	temp+=2047;
	temp3+=2047;

	ccr2_out=(ccr2_out+temp)>>1; // smoother
	ccr1_out=(ccr1_out+temp3)>>1;
	ccr2_out=temp;
	ccr1_out=temp3;

	//ccr_buf[ccr_counter_2]=((uint32_t)ccr2_out << 16) | (uint32_t)ccr1_out;
	audio_out_buf[ccr_counter_2]=((uint32_t)ccr2_out << 16) | (uint32_t)ccr1_out;  // write to temp buffer , might run a limiter after
	//ccr_buf[ccr_counter_2+1]=ccr1_out;


	//ccr1_out=temp3+2048;// for testing
	for (i=0;i<3;i++){ // advance data pointer, for freq generation , all notes

		wav_pointer[i]=wav_pointer[i]+CNT_list_selected[i];
		if (wav_pointer[i]>wav_multi) {wav_pointer[i]=wav_pointer[i]-wav_multi;
		zero_cross[i]=1;
		}

	}


//	if ( (wav_pointer[0]<1000)&& note_trigger ) {zero_cross[0]=1;} // this works good
//	if ( (wav_pointer[1]<1000)&& note_trigger ) {zero_cross[1]=1;} // this works good

	//ADSR_counter_position[0]=0;
	//ADSR_out_1=envelopes_store[0];
	next_sample_ready=2;

	for (i=0;i<poly;i++){
	one_play[i].position+=one_play[i].playback_rate;  // use this now calculate playback position
	if(one_play[i].position>(((64*MAX_Rate)-1)<<16)) one_play[i].position=(((64*MAX_Rate)-1)<<16); // limit to download buffer size
			}


		}




void ADSR_TIM_writer(void){   // single note for now  10ms ,16 bit ,could be smoother , 400 samples ish , might change to line up with sample process

	//cc_77=0;  stutter section /////////////
	//if (stutter_toggle>=stutter_rate) {stutter_toggle=0;stutter_flip=!stutter_flip;} else stutter_toggle++;

	//if (cc_77) stutter_rate=cc_77^1; else {stutter_rate=0;stutter_toggle=0;stutter_flip=0;}  // stutter disabled for now


	//uint32_t countup=tmr_counter_value_get(TMR6);
	uint16_t counter=ADSR_counter_position[0];
	uint32_t length=(samples_store[current_playing_sample[0]].size_bytes>>9);
	uint32_t temp;
	if (length<129) length=129;

	temp=(envelopes_store[counter]*audio_gain[1])>>8;   // data out for pwm
	ADSR_out[0]=temp;
//	if (output_gain<0.7) output_gain*=1.000001;  // regain
//	if (output_gain2<1) output_gain2*=1.000001;  // regain
//	side_gain=sidechain_accu*0.0001;   // 2000-4000
//	if (side_gain>1) side_gain=1;
//	side_gain=1-side_gain;
//	if(side_gain<1) side_gain*=1.01;  // regain
	//if(multi<128) multi++;

	if ((counter) && (!ADSR_out[0]))  counter=255;  // force 0 if no signal
	if (counter>254) counter=255; else counter++; // stop at the end
	ADSR_counter_position[0]=counter;
	/////////////////////  second envelope
	counter=ADSR_counter_position[1];
	temp=(envelopes_store[counter]*audio_gain[1])>>8;   // data out around 16 bit
	ADSR_out[1]=temp;
	if ((counter>255) && (!ADSR_out[1]))  counter=511;  // force 0 if no signal
	if (counter>510) counter=511; else counter++; // stop at the end
	ADSR_counter_position[1]=counter;

	////////////////          drums or samples length should be set by current sample size

	counter=ADSR_counter_position[2]; // this really should end at length of samples
	if (counter>length) counter=length;
	ADSR_out[2]=255*audio_gain[2];   // samples
	if (counter>(length-127)) ADSR_out[2]= length-counter; // fade out
	if (counter>length) {counter=length;ADSR_out[2]=0;} else counter++; // stop at the end
	ADSR_counter_position[2]=counter;  //
	////////////////




}

void audio_gain_global(void){  // needs to be around adsr

	for (uint8_t i=0;i<3;i++){
		if ((audio_gain_cut[i]>60)&& audio_gain[i]) {audio_gain[i]--;} // go down to 1 for now , adjust number of clicks allowed

		audio_gain_cut[i]=0;

	}




}

	void delay_calc(void){

		delay_pointer[0]+=audio_buffer_size;  //reading , counts up samples
		delay_pointer[0] &=(delay_buffer_size-1);

		uint32_t delay=delay_pointer[0]; // reading
		uint32_t delay_pointer1=delay+(delay_time*delay_time_multiplier);  // adds length between read and write for write back

		uint32_t delay_2=delay_pointer1&(delay_buffer_size-1); // loops pointer number , for writing back
		//uint32_t delay_3=(delay_pointer1+delay_time_multiplier)&(delay_buffer_size-1); // extra position for writing back
		//uint16_t delay_3=delay_pointer1&(delay_buffer_size-1);
		delay_pointer[1]=delay_2; // write back

	}

	// 4-point Hermite interpolation (recommended)
	// 4-point Hermite interpolation for int16_t samples
	// phase is 16.16 fixed-point (integer part = sample index, frac = 0..65535)
	static inline int16_t resample_hermite(const int16_t* sample_data, uint32_t phase)
	{
	    uint32_t idx  = phase >> 16;      // integer sample index
	    uint32_t frac = phase & 0xFFFF;   // fractional part (0..65535)

	    // Read 4 surrounding samples with safe clamping at boundaries
	    int32_t x0 = sample_data[idx - 1];
	    int32_t x1 = sample_data[idx];
	    int32_t x2 = sample_data[idx + 1];
	    int32_t x3 = sample_data[idx + 2];

	    // Hermite coefficients
	    int32_t c0 = x1;
	    int32_t c1 = (x2 - x0) >> 1;                                 // tangent at x1
	    int32_t c2 = x0 - ((5 * x1) >> 1) + (x2 << 1) - (x3 >> 1);   // curvature
	    int32_t c3 = ((x3 - x0) >> 1) + (((3 * (x1 - x2)) >> 1));    // sharper curvature

	    // t = frac / 65536.0   → fixed point
	    int32_t t  = frac;
	    int32_t t2 = (t * t) >> 16;
	    int32_t t3 = (t2 * t) >> 16;

	    // Hermite polynomial: c0 + c1*t + c2*t² + c3*t³
	    int32_t result = c0
	                   + ((c1 * t)  >> 16)
	                   + ((c2 * t2) >> 16)
	                   + ((c3 * t3) >> 16);

	    // Clamp to int16_t range
	    if (result > 32767)  return 32767;
	    if (result < -32768) return -32768;

	    return (int16_t)result;
	}
	// Returns 12-bit sample (0..4095)
	static inline int16_t resample_linear(const int16_t* sample_data,   // your sample in RAM/Flash
	                                       uint32_t phase)               // 16.16 fixed point phase
	{
	    uint32_t idx = phase >> 16;                    // integer part
	    uint32_t frac = phase & 0xFFFF;                // fractional part (0..65535)

	    int32_t a = sample_data[idx];
	    int32_t b = sample_data[(idx + 1)];

	    // Linear:   a + (b - a) * frac / 65536
	    return (int16_t)(a + (((b - a) * frac) >> 16));
	}
	static inline int16_t resample_hermite_float(const int16_t* sample_data, uint32_t phase)
	{
	    float idx_f = (float)(phase >> 16) + (float)(phase & 0xFFFF) / 65536.0f;
	    uint32_t idx = (uint32_t)idx_f;
	    float frac = idx_f - (float)idx;

	    float x0 = sample_data[idx - 1];
	    float x1 = sample_data[idx];
	    float x2 = sample_data[idx + 1];
	    float x3 = sample_data[idx + 2];

	    float c0 = x1;
	    float c1 = 0.5f * (x2 - x0);
	    float c2 = x0 - 2.5f*x1 + 2.0f*x2 - 0.5f*x3;
	    float c3 = 0.5f*(x3 - x0) + 1.5f*(x1 - x2);

	    float result = c0 + c1*frac + c2*frac*frac + c3*frac*frac*frac;

	    // Clamp
	    if (result >  32767.0f) return 32767;
	    if (result < -32768.0f) return -32768;

	    return (int16_t)result;
	}

	// Hermite interpolation for one-shot samples
	// Returns 0 when sample has finished playing
	int16_t resample_hermite_oneshot(const int16_t* sample_data,
	                                 uint32_t sample_length,
	                                 uint32_t* phase,          // 16.16 fixed point
	                                 uint32_t increment)
	{
	    uint32_t idx = *phase >> 16;

	    // Sample has ended → return silence and stop advancing
	    if (idx >= sample_length - 2) {       // -2 because we need 2 samples after idx
	        *phase = 0;                       // optional: reset phase
	        return 0;
	    }

	    uint32_t frac = *phase & 0xFFFF;

	    // Read 4 surrounding samples (safe near the end)
	    int32_t x0 = sample_data[idx - 1];
	    int32_t x1 = sample_data[idx];
	    int32_t x2 = sample_data[idx + 1];
	    int32_t x3 = sample_data[idx + 2];

	    // Hermite coefficients
	    int32_t c0 = x1;
	    int32_t c1 = (x2 - x0) >> 1;
	    int32_t c2 = x0 - ((5 * x1) >> 1) + (x2 << 1) - (x3 >> 1);
	    int32_t c3 = ((x3 - x0) >> 1) + ((3 * (x1 - x2)) >> 1);

	    int32_t t  = frac;
	    int32_t t2 = (t * t) >> 16;
	    int32_t t3 = (t2 * t) >> 16;

	    int32_t result = c0
	                   + ((c1 * t)  >> 16)
	                   + ((c2 * t2) >> 16)
	                   + ((c3 * t3) >> 16);

	    // Update phase
	    *phase += increment;

	    // Clamp
	    if (result > 32767)  return 32767;
	    if (result < -32768) return -32768;

	    return (int16_t)result;
	}
	int16_t resample_linear_oneshot(const int16_t* sample_data,
	                                uint32_t sample_length,
	                                uint32_t* phase,      // this is simple an accu ?
	                                uint32_t increment)  // increase phase value ?
	{
	    uint32_t idx = *phase >> 16;

	    if (idx >= sample_length - 1) {  // exit if longer than buf
	        *phase = 0;
	        return 0;
	    }

	    uint32_t frac = *phase & 0xFFFF;

	    int32_t a = sample_data[idx];
	    int32_t b = sample_data[idx + 1];

	    int32_t result = a + (((b - a) * frac) >> 16);

	    *phase += increment;

	    if (result > 32767)  return 32767;
	    if (result < -32768) return -32768;

	    return (int16_t)result;
	}

	// Returns interpolated int16_t sample with forward looping
	// phase is 16.16 fixed point
	int16_t resample_hermite_loop(const int16_t* sample_data,
	                              uint32_t sample_length,      // length in samples
	                              uint32_t* phase,             // pointer to phase accumulator
	                              uint32_t increment)          // pitch/speed
	{
	    uint32_t idx = *phase >> 16;
	    uint32_t frac = *phase & 0xFFFF;

	    // Handle looping
	    if (idx >= sample_length) {
	        idx %= sample_length;                    // forward loop
	        *phase = (idx << 16) | frac;             // keep fractional part
	    }

	    // Read 4 points with proper wrapping
	    uint32_t i0 = (idx == 0) ? sample_length - 1 : idx - 1;
	    uint32_t i1 = idx;
	    uint32_t i2 = (idx + 1) % sample_length;
	    uint32_t i3 = (idx + 2) % sample_length;

	    int32_t x0 = sample_data[i0];
	    int32_t x1 = sample_data[i1];
	    int32_t x2 = sample_data[i2];
	    int32_t x3 = sample_data[i3];

	    // Hermite coefficients
	    int32_t c0 = x1;
	    int32_t c1 = (x2 - x0) >> 1;
	    int32_t c2 = x0 - ((5 * x1) >> 1) + (x2 << 1) - (x3 >> 1);
	    int32_t c3 = ((x3 - x0) >> 1) + ((3 * (x1 - x2)) >> 1);

	    int32_t t  = frac;
	    int32_t t2 = (t * t) >> 16;
	    int32_t t3 = (t2 * t) >> 16;

	    int32_t result = c0
	                   + ((c1 * t)  >> 16)
	                   + ((c2 * t2) >> 16)
	                   + ((c3 * t3) >> 16);

	    // Update phase
	    *phase += increment;

	    // Clamp output
	    if (result > 32767)  return 32767;
	    if (result < -32768) return -32768;

	    return (int16_t)result;
	}


	void sampler_looping_functions(uint32_t sample_start,uint32_t sample_size,uint16_t phase, uint8_t fx){
		// various playback fx ,returns sample memory address, things like reverse ,ping pong , various mid sample looping
		// it'l change direction at certain points
		// phase = 0-FFFF : position


	}
uint16_t lfo_out(){   // creates and lfo output/  one step

	uint32_t output=0;
	uint32_t test=0;
	lfo1_counter=(lfo1_counter+lfo1_rate)&32767;   // 15 bit
	test= lfo1_counter>>8;
	if (test>127) test=127;
	if (lfo1_rate>2) phaser_enable=2; else phaser_enable=0;
	output=sine_wave[test]; // 0-1023
	//lfo1_2_out=output;
	//output=((output*lfo1_depth)>>8)+(lfo1_depth*2);
	output=((output*lfo1_depth)>>7);
	if (output>1023) output=1023;

	return output;

}
void ducking_control(void){  // creates current_ducking_level for audio
	uint8_t duck=0;
	for (int var = 0; var < 8; ++var) {

		if (current_ducking_mask[var]) {  // only test ducking slaves
			duck=0;
			for (int i = 0; i< 8; ++i) {
				if ((var!=i) && (current_playing_sample[i]==1) && (!current_ducking_mask[i])) duck=1;  // do not activate by other active ducking
			}
			if (duck) current_ducking_level[var]=current_ducking_mask[var]&3; else current_ducking_level[var]=0;
		}

	}


}



int32_t soft_clip(int32_t x) {
    if (x > 32767) return 32767 - ((x - 32767) >> 2);
    if (x < -32768) return -32768 - ((x + 32768) >> 2);
    return x;
}

int32_t soft_clip_cubic(int32_t x) {
    // Input should be roughly in -1.5 .. 1.5 range (scaled)
    int32_t x3 = (x * x >> 15) * x >> 15;
    return x - (x3 >> 2);          // tune the >> 2
}

int32_t apply_gain_soft(int32_t sample, int32_t gain)
{
    // Apply gain
    int32_t x = (sample * gain) >> 15;

    // Cheap & good sounding soft clip (cubic approximation)
    // Works great in int32
    if (x > 32767)  x = 32767;
    if (x < -32768) x = -32768;

    // Optional nicer soft clip (still cheap):
    // x = x - (x*x*x >> 30);   // very mild saturation
    // or the classic:
    // int32_t x2 = (x * x) >> 15;
    // x = x + ((x * x2) >> 16);   // adjust coefficients to taste

    return x;
}

// Call this every sample (or every 4–8 samples for cheaper version)
int32_t compute_gain(int32_t *channels, int num_ch, int32_t current_gain)
{
    // 1. Find peak or RMS-ish across all channels
    int32_t peak = 0;
    for (int i = 0; i < num_ch; i++) {
        int32_t a = channels[i] >= 0 ? channels[i] : -channels[i];
        if (a > peak) peak = a;
    }

    // Optional: simple envelope follower (attack/release)
    static int32_t env = 0;
    if (peak > env)
        env += (peak - env) >> 3;          // fast attack
    else
        env += (peak - env) >> 6;          // slower release

    // 2. Soft knee gain computer (everything in Q15 / Q16)
    // Threshold example: 0.7 of full scale (you can change)
    const int32_t thresh = 23000;          // ~0.7 * 32767
    const int32_t knee   = 4000;           // soft knee width

    int32_t gain = 32767;                  // unity

    if (env > thresh - knee) {
        // Soft region
        int32_t over = env - (thresh - knee);
        // Simple quadratic soft knee (very cheap)
        int32_t reduction = (over * over) >> 12;   // tune the shift
        gain = 32767 - reduction;
        if (gain < 4096) gain = 4096;      // never go completely silent
    }

    // 3. Smooth the gain itself (important!)
    current_gain += (gain - current_gain) >> 4;   // smooth factor

    return current_gain;
}
