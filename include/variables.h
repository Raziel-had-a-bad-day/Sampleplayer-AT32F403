
#define MIDI_NOTE_OFF 128
#define MIDI_NOTE_ON 144
#define c_change 176
#define usart_buffer_size 3

#define cpu_clock    84000000
#define uart_receive 3     // uart buffer size ,needs at least 3 to work properly
#define cpu_clock_prescaler 1  // for pwm
#define TIM_counter_CNT 2048  //  41khz sampple rate ,320hz  at +1
#define TIM_CCR_default 1024    // duty cycle, about 50% of CNT
#define note_count 4
#define cycle_length  600  // one cycle sample rate
#define ADSR_CCR 1680000
#define sample_location 0x08020000       // stored samples start location on internal flash

#define flash_last_byte 0xFFFFFF // 16 777 215
#define flash_record_backup 0xFFEF00// page address should always start on 00  !!!!

#define poly 8   // polyphony- spell check doesn't know this word ,retarded
#define note_on 144
#define p_change 192
#define midi_channel 4   // current midi channel
#define delay_buffer_size 8192



uint32_t flash_memory_record[64]={};   // keep track of memory blocks start-end ,start-end, if empty then disable all flash writes ,
//stored in external flash , start and end per banks so 2 words per bank lets say 16 banks

uint8_t memory_record_error_flag=0;  // activate if anything wrong with memory record , then proceed to restore

uint32_t  sample_select[poly];


int16_t delay_buffer[delay_buffer_size]; //200ms 10khz 16 bit
uint16_t delay_pointer[3];   // sets read and write for various fx
uint8_t delay_time=0;  //sets delaytime

// SWING - even numbered notes are shifted left or right
uint8_t serial_temp[uart_receive];
uint8_t serial_hold[uart_receive];
volatile uint8_t midi_in_clear=0; // high if ready
uint8_t led_blink;
uint8_t temp_incoming[32];
uint8_t temp_incoming_counter;
uint32_t CNT_list[128];  // holds CNT values for PWM 0-127 midi notes , 16 bit
uint16_t envelopes_store[256*note_count];  // holds envelopes data 1 byte = 100ms *4  256 max or 25s long
uint8_t ADSR_settings [4*note_count]={1,20,0,0,1,20,0,0,20,20,20,50,20,20,20,50,}; // keeps settings for 4 notes
uint8_t ADSR_sustain[note_count]={32,32,32,32}; // sustain levels
uint8_t ADSR_timer_flag; // sets up adsr process every 100ms
uint16_t ADSR_counter_position[note_count]={0,256,512,768};   // sets position per note
uint16_t last_CCR;
int16_t triangle[128]; // holds 8 bit triangle
uint32_t wav_pointer[poly]; // points to triangle wav
uint32_t CNT_list_selected[poly]; // holds CNT_list selected value ,counter adder for freq
uint16_t ADSR_out_1;
uint16_t ADSR_out_2;
uint8_t zero_cross;  // enabled when wav near zero
uint8_t note_trigger;  // holds value for note out until cleared when zero cross is active
uint8_t midi_fifo[64]; //holds incoming mid
uint8_t midi_fifo_pointer_1; // for midi_fifo, for buffer
uint8_t midi_fifo_pointer_2; // for midi_fifo, for midi processing
uint32_t ccr2_out;
uint8_t next_sample_ready=0;
uint8_t midi_note_hold[4]; // holds incoming midi note
uint8_t midi_pc_hold[4];  // holds program change data
uint8_t midi_cc_hold[4]; // holds incoming cc data
uint8_t test_hold[4];
uint8_t temp_data[260]={1,2,3,4,5,9,7,8,9};
int16_t in_sample_holder[600];
int16_t in_sample_holder_2[600];
uint8_t memory_write_error=0;
int8_t note_pitch_shift=0; // stores a number from key split upper range

uint8_t test_sample[1205];
volatile uint8_t spi_dma_ready=0;
uint8_t total_samples=80; // max samples in current bank atm no more than 105 per bank (128k)
uint8_t cc_75=0;
uint8_t cc_76=0;
uint8_t cc_77=0;  // incoming cc
uint16_t release_start[note_count]={5,261,517,773}; // points to release start pos in envelopes_store
uint8_t stutter_rate=0;
uint8_t stutter_toggle=0;
uint8_t stutter_flip=0;
float output_gain=1;   // settles around 0.7
float output_gain2=1;   // 1 is ok  0.85 with full feedback
int16_t multi=128;





void midi_note_pwm_calculator(void){    // calculates counter values for pwm

	float freq_list[128]; // store frequencies
	int i;

	float cpu_out=(cpu_clock/TIM_counter_CNT)/cycle_length;  // sets the f when +1 step
	for(i=0;i<128;i++){

		  freq_list[i]=440.0 * pow(2.0, (i - 69) / 12.0); //Calculate frequencies

	}
	for(i=0;i<128;i++){    // 1 cycle is 128 steps so

		   CNT_list[127-i]=round((cpu_out/freq_list[i])*4800); //Calculate step speed for different notes ,needs big numbers for correct speeds

	}

}
void envelopes_preprocess(uint8_t note){     // calcualtes adsr values for notes ,seems ok , 4 x envelopes
	uint8_t i=note;
	uint8_t n;

	uint16_t envelopes_pointer=0;  // for envelope file
	uint8_t adsr_pointer=0;
	int level=0;

	uint8_t while_pointer;
	uint16_t max_level=64000;   // sets max adsr value
	uint16_t level_target=max_level;

//	uint16_t size=(note+1)*256;
   // memset (envelopes_store,0,size);
//	for(i=0;i<note_count;i++){
		n=0;

		adsr_pointer=ADSR_settings[i*4];  //attack
		level_target=max_level;
		while_pointer=adsr_pointer;
		while (n<(while_pointer)){

			envelopes_pointer=(i*256)+n;  // count up
			level=(level+(level_target/(adsr_pointer)));
			if(level>max_level) level=max_level;   // limit

			envelopes_store[envelopes_pointer]=level;  // should finish around 255
			n++; }


		//release_start[note]=n;   //  for changing sample , can be anywhere or a set point for all sounds

		adsr_pointer=ADSR_settings[(i*4)+1]; // decay
		level_target=max_level-ADSR_sustain[i];
		while_pointer=adsr_pointer+n;
		while (n<(while_pointer)){

			envelopes_pointer=(i*256)+n;
			level=(level-(level_target/(adsr_pointer)));
			if(level>max_level) level=max_level;
			if(level<0) level=0;
			envelopes_store[envelopes_pointer]=level;
			n++; }



		level_target=ADSR_sustain[i];
		adsr_pointer=ADSR_settings[(i*4)+2]; // sustain length
		while_pointer=adsr_pointer+n;
		while (n<(while_pointer)){

			envelopes_pointer=(i*256)+n;
			level=level_target;


			envelopes_store[envelopes_pointer]=level;
			n++; }


		level_target=ADSR_sustain[i];
		adsr_pointer=ADSR_settings[(i*4)+3]; // release length
		while_pointer=adsr_pointer+n;

		while (n<(while_pointer)){

			envelopes_pointer=(i*256)+n;
			level=(level-(level_target/(adsr_pointer)));
			if(level<0) level=0;
			envelopes_store[envelopes_pointer]=level;
			n++; }

				//	}  // end of i counter


} // end of envelopes process


void waves(void) {
    int value = 0;
    int direction = 1; // 1 for increasing, 0 for decreasing
    uint8_t counter=0;
    while (counter<128) {

        if (direction) {
            value+=32;
            if (value >= 1023) {value=1023;direction = 0;} // Change direction
        } else {
            value-=32;
            if (value <= -1023) value=-1023; // Change direction
        }

    triangle[counter]=value;
    counter ++;

    }




}


