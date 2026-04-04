
#define MIDI_NOTE_OFF 128
#define MIDI_NOTE_ON 144
#define c_change 176
#define usart_buffer_size 3
#define user_data_start 0x080E0000
#define settings_data   0x080EF000  // the last 256 byte
#define SPIM_address  0x08400000   // default 16MB
#define cpu_clock    120000000
#define uart_receive 3     // uart buffer size ,needs at least 3 to work properly
#define cpu_clock_prescaler 1  // for pwm
#define TIM_counter_CNT 2048  //  41khz sampple rate ,320hz  at +1
#define TIM_CCR_default 1024    // duty cycle, about 50% of CNT
#define note_count 4
#define cycle_length  600  // one cycle sample rate
#define ADSR_CCR 1680000
#define sample_location 0x08020000       // stored samples start location on internal flash
#define SPIM_START_ADDR      0x08400000

#define flash_last_byte 0xFFFFFF // 16 777 215
#define flash_record_backup 0xFFEF00// page address should always start on 00  !!!!

#define poly 8   // polyphony- spell check doesn't know this word ,retarded
#define note_on 144
#define p_change 192
#define midi_channel 4  // current midi channel
#define delay_buffer_size 16384 //int16
#define wav_multi 256*cycle_length
#define delay_time_multiplier delay_buffer_size/32

#define SPI2_CS_HIGH   	GPIOB->scr = GPIO_PINS_5;
#define SPI2_CS_LOW    GPIOB->clr = GPIO_PINS_5;
#define SPI4_CS_HIGH   	GPIOB->scr = GPIO_PINS_6;
#define SPI4_CS_LOW    GPIOB->clr = GPIO_PINS_6;
#define ram_buffer_size 40   // in bytes , 16 samples +8 bytes error correct
#define audio_buffer_size 64
#define one_shot_size 80000 // size in word 16 bit


uint8_t countSetBits(uint8_t number) { // count bits in byte
	uint8_t count = 0;
    while (number) {
        count += number & 1; // Increment count if the last bit is 1
        number >>= 1;        // Right shift n by 1
    }
    return count;}




uint8_t usart3_rx_buffer[16]; // uart 3 rx buffer
uint8_t usart3_rx_counter;
uint32_t flash_memory_record[64]={};   // keep track of memory blocks start-end ,start-end, if empty then disable all flash writes ,
//stored in external flash , start and end per banks so 2 words per bank lets say 16 banks

uint8_t memory_record_error_flag=0;  // activate if anything wrong with memory record , then proceed to restore

uint32_t  sample_select[poly]={1200,1200};
uint32_t systick_hold;

int16_t delay_buffer[delay_buffer_size]; //200ms 10khz 16 bit.  using psram now
int16_t delay_buffer_2[260]; //200ms 10khz 16 bit.  using psram now


uint16_t delay_pointer[3];   // sets read and write for various fx
uint8_t delay_time=0;  //sets delaytime
int16_t delay_in_buf[256];
int16_t delay_out_buf[256];

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
uint16_t ADSR_counter_position[note_count]={0,255,511,767};   // sets position per note
uint16_t last_CCR;
int16_t triangle[128]; // holds 8 bit triangle
uint32_t wav_pointer[poly]={0,0,0,0,0,0,0,0}; // points to triangle wav
uint32_t CNT_list_selected[poly]={500,500,500,500,500,500,500,500}; // holds CNT_list selected value ,counter adder for freq
uint16_t ADSR_out_1;
uint16_t ADSR_out_2;
uint16_t ADSR_out[poly];
uint8_t zero_cross[poly];  // enabled when wav near zero
uint8_t zero_cross2;
uint8_t note_trigger;  // holds value for note out until cleared when zero cross is active

uint8_t midi_fifo_pointer_1; // for midi_fifo, for buffer
uint8_t midi_fifo_pointer_2; // for midi_fifo, for midi processing

volatile uint8_t ccr_counter; // keeps track
uint8_t ccr_counter_2;
uint32_t ccr_buf[audio_buffer_size];
uint32_t ccr2_out;
uint32_t ccr1_out;
int16_t ram_in[128]; // delay audio data to be sent to ram
int16_t ram_out[132];// delay audio date from ram
uint8_t next_sample_tracker; // tracks process count

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
uint8_t settings_write_flag=0;
uint8_t all_settings[256]={255,255,255,255};
uint32_t save_timer;
uint32_t temp_ccr;
int8_t note_fifo_write=0;
uint8_t note_fifo_buf[32];
int8_t midi_fifo_write=0;
uint8_t midi_fifo_buf[32];
uint8_t midi_buf_flag=0;  // high until all cleared
uint8_t midi_fifo_temp[4];
uint8_t current_velocity=0; // just holds last velocity
int32_t delay_filter;
uint16_t spi_buf[512];
uint16_t spi_counter_2=0;
int32_t temp_wave;
uint8_t sound_triggers[8]; // note_trigger for invidiual sounds
int16_t one_shot_wav[296]; // holds incoming data for one shot sample
uint32_t one_shot_pointer=0; // points to current byte location in wav

uint32_t one_shot_playback_rate=0xFFFF;
uint32_t one_shot_position;// use to calc pos , 1<<17 is one full step
uint8_t temp_store[256];  // delete this
uint8_t ram_page_read_buf[256];
uint8_t ram_test_buf[270];
uint8_t ram_page_write_buf[256];
int8_t test_byte[256];
volatile uint32_t spi_transmit_counter=0;
uint32_t spi_receive_counter=0;
uint8_t tmr_counter[32];
int16_t flash_sample_buf[128];
volatile uint8_t spi_read_flag=0;  // clear on irq
volatile uint8_t spi_write_flag=0;  // clear on irq
uint8_t spi_message_cue; // keeps track of spi messages
int16_t sine_testing[156];
uint8_t spi_adder=0;





const uint16_t sine_lut[600]={

		32768, 33111, 33454, 33797, 34140, 34482, 34825, 35167,
		35509, 35851, 36193, 36534, 36874, 37215, 37554, 37893,
		38232, 38570, 38908, 39244, 39580, 39916, 40250, 40584,
		40916, 41248, 41579, 41909, 42238, 42566, 42893, 43219,
		43544, 43867, 44189, 44510, 44830, 45148, 45465, 45781,
		46095, 46408, 46719, 47029, 47337, 47644, 47949, 48252,
		48553, 48853, 49151, 49448, 49742, 50035, 50325, 50614,
		50901, 51186, 51468, 51749, 52028, 52304, 52579, 52851,
		53121, 53389, 53654, 53918, 54178, 54437, 54693, 54947,
		55198, 55447, 55694, 55938, 56179, 56418, 56654, 56888,
		57118, 57347, 57572, 57795, 58015, 58233, 58447, 58659,
		58868, 59074, 59277, 59477, 59675, 59869, 60060, 60249,
		60434, 60616, 60796, 60972, 61145, 61315, 61482, 61646,
		61806, 61964, 62118, 62269, 62416, 62561, 62702, 62840,
		62975, 63106, 63234, 63359, 63480, 63598, 63712, 63824,
		63931, 64036, 64136, 64234, 64328, 64418, 64506, 64589,
		64669, 64746, 64819, 64889, 64955, 65017, 65076, 65132,
		65183, 65232, 65277, 65318, 65355, 65390, 65420, 65447,
		65470, 65490, 65506, 65519, 65528, 65533, 65535, 65533,
		65528, 65519, 65506, 65490, 65470, 65447, 65420, 65390,
		65355, 65318, 65277, 65232, 65183, 65132, 65076, 65017,
		64955, 64889, 64819, 64746, 64669, 64589, 64506, 64418,
		64328, 64234, 64136, 64036, 63931, 63824, 63712, 63598,
		63480, 63359, 63234, 63106, 62975, 62840, 62702, 62561,
		62416, 62269, 62118, 61964, 61806, 61646, 61482, 61315,
		61145, 60972, 60796, 60616, 60434, 60249, 60060, 59869,
		59675, 59477, 59277, 59074, 58868, 58659, 58447, 58233,
		58015, 57795, 57572, 57347, 57118, 56888, 56654, 56418,
		56179, 55938, 55694, 55447, 55198, 54947, 54693, 54437,
		54178, 53918, 53654, 53389, 53121, 52851, 52579, 52304,
		52028, 51749, 51468, 51186, 50901, 50614, 50325, 50035,
		49742, 49448, 49151, 48853, 48553, 48252, 47949, 47644,
		47337, 47029, 46719, 46408, 46095, 45781, 45465, 45148,
		44830, 44510, 44189, 43867, 43544, 43219, 42893, 42566,
		42238, 41909, 41579, 41248, 40916, 40584, 40250, 39916,
		39580, 39244, 38908, 38570, 38232, 37893, 37554, 37215,
		36874, 36534, 36193, 35851, 35509, 35167, 34825, 34482,
		34140, 33797, 33454, 33111, 32768, 32424, 32081, 31738,
		31395, 31053, 30710, 30368, 30026, 29684, 29342, 29001,
		28661, 28320, 27981, 27642, 27303, 26965, 26627, 26291,
		25955, 25619, 25285, 24951, 24619, 24287, 23956, 23626,
		23297, 22969, 22642, 22316, 21991, 21668, 21346, 21025,
		20705, 20387, 20070, 19754, 19440, 19127, 18816, 18506,
		18198, 17891, 17586, 17283, 16982, 16682, 16384, 16087,
		15793, 15500, 15210, 14921, 14634, 14349, 14067, 13786,
		13507, 13231, 12956, 12684, 12414, 12146, 11881, 11617,
		11357, 11098, 10842, 10588, 10337, 10088, 9841, 9597,
		9356, 9117, 8881, 8647, 8417, 8188, 7963, 7740,
		7520, 7302, 7088, 6876, 6667, 6461, 6258, 6058,
		5860, 5666, 5475, 5286, 5101, 4919, 4739, 4563,
		4390, 4220, 4053, 3889, 3729, 3571, 3417, 3266,
		3119, 2974, 2833, 2695, 2560, 2429, 2301, 2176,
		2055, 1937, 1823, 1711, 1604, 1499, 1399, 1301,
		1207, 1117, 1029, 946, 866, 789, 716, 646,
		580, 518, 459, 403, 352, 303, 258, 217,
		180, 145, 115, 88, 65, 45, 29, 16,
		7, 2, 0, 2, 7, 16, 29, 45,
		65, 88, 115, 145, 180, 217, 258, 303,
		352, 403, 459, 518, 580, 646, 716, 789,
		866, 946, 1029, 1117, 1207, 1301, 1399, 1499,
		1604, 1711, 1823, 1937, 2055, 2176, 2301, 2429,
		2560, 2695, 2833, 2974, 3119, 3266, 3417, 3571,
		3729, 3889, 4053, 4220, 4390, 4563, 4739, 4919,
		5101, 5286, 5475, 5666, 5860, 6058, 6258, 6461,
		6667, 6876, 7088, 7302, 7520, 7740, 7963, 8188,
		8417, 8647, 8881, 9117, 9356, 9597, 9841, 10088,
		10337, 10588, 10842, 11098, 11357, 11617, 11881, 12146,
		12414, 12684, 12956, 13231, 13507, 13786, 14067, 14349,
		14634, 14921, 15210, 15500, 15793, 16087, 16384, 16682,
		16982, 17283, 17586, 17891, 18198, 18506, 18816, 19127,
		19440, 19754, 20070, 20387, 20705, 21025, 21346, 21668,
		21991, 22316, 22642, 22969, 23297, 23626, 23956, 24287,
		24619, 24951, 25285, 25619, 25955, 26291, 26627, 26965,
		27303, 27642, 27981, 28320, 28661, 29001, 29342, 29684,
		30026, 30368, 30710, 31053, 31395, 31738, 32081, 32424


};

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

