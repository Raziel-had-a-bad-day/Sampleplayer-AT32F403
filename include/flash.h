 void flash_to_ram_mirror (void);
 void flash_to_ram(void);
 void ram_to_flash_mirror (void);
 void flash_counter_write(uint32_t data);
 void controller_process(void);

void settings_storage(void){   // runs to store setting and read back

#define variable_count 9


	uint8_t *settings[variable_count]={
			(uint8_t*)ADSR_settings,
			//(uint8_t*)sample_select,
			(uint8_t*) cc_list_extra,
			&cc_75,
			&cc_76,
			&cc_77,
			&delay_time,
			(uint8_t*)current_write_pos,
			(uint8_t*) samples_backup,// might expand it a little
			(uint8_t*) one_shot_backup};// might expand it a little


			uint16_t settings_multi[variable_count]={16,32,1,1,1,1,4,sizeof(samples_backup),sizeof(one_shot_backup)};   // sets length,  sound_set*x ,512 atm
			uint8_t settings_temp[2048];
			uint16_t settings_total=0;  //adds up position , huge miss here retard alert
			uint16_t length=0; // max 64 atm
			uint16_t i=0;


			uint32_t read_adr= settings_data;

			for (i=0;i<variable_count;i++){

				length=(settings_multi[i]);
				if(settings_write_flag) {		memcpy(settings_temp,settings[i],length);	// copy to temp
				memcpy(all_settings+settings_total,settings_temp,length);

				}
				else {

					memcpy(settings_temp,all_settings+settings_total,length);  // copy value
					memcpy(settings[i],settings_temp,length);                }  //

		settings_total=settings_total+length;

			}

		//	for (i=0;i<1023;i++){if (all_settings[i]>127) all_settings[i]=0;}  // reset values just in case
			settings_write_flag=0;

}
void flash_settings_write(void){  // sector = 2k
	flash_unlock();
	uint32_t read_adr= settings_data;
	uint16_t i=0;

	 // backup samples struct
	flash_sector_erase(read_adr);
	flash_operation_wait_for(1000);

	for (i=0;i<2048;i++){   //
		flash_byte_program(read_adr, all_settings[i]);

		  read_adr += 1;
	}
	flash_operation_wait_for(1000);



}
void flash_counter_write(uint32_t data){  // write flash counter
	flash_unlock();
	uint32_t read_adr= settings_data+1024;  // just a preset location
	uint16_t i=0;
	for (i=0;i<4;i++){
		data=data>>i;
		flash_byte_program(read_adr+i, (uint8_t)(data&255));
	}

}

uint32_t flash_counter_read(){  // return current flash counter


	uint32_t read_adr= settings_data+8196;
	if (read_adr>16000000)  read_adr=0;  // reset but only if bad data
	return *(uint32_t*)(read_adr);

}





/*void read_busy(void){
	uint8_t temp[2]={0x05};
	uint8_t  temp2[2]={0,0};
	uint8_t volatile flag=1;  // this is needed as it can stuck here , something with gpio toggling

	 while (flag) {
		HAL_GPIO_WritePin(CS1_GPIO_Port, CS1_Pin, 0);  // when readin low till the end
		HAL_SPI_TransmitReceive (&hspi1,temp,temp2,2, 10); // request data , always leave extra room (clock) , works
		spi_i2s_data_transmit(SPI4, 0x05);

		HAL_GPIO_WritePin(CS1_GPIO_Port, CS1_Pin, 1);  // high end
		flag=temp2[1]&1;

	 }

}*/
void flash_read(uint32_t address,int16_t* mem){     // reads as int16
	//trying fast read, first received byte needs to be skipped though
	address=address<<1;
	uint8_t transmit[136]={0x03,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),121,122,123,124 };

	uint8_t rec_2[136];
	uint8_t t_counter=0;


	SPI4_CS_LOW;
	while (t_counter<(128+6)){
		while(spi_i2s_flag_get(SPI4, SPI_I2S_TDBE_FLAG) == RESET);  // wait for flag
		spi_i2s_data_transmit(SPI4, transmit[t_counter]);
		while(spi_i2s_flag_get(SPI4, SPI_I2S_RDBF_FLAG) == RESET);
		rec_2[t_counter] = spi_i2s_data_receive(SPI4);

		t_counter++;

	}
	while(spi_i2s_flag_get(SPI4, SPI_I2S_BF_FLAG) != RESET);
	SPI4_CS_HIGH;
	memcpy(mem,rec_2+5,128);  // modified for fast read




} // end of while loop

/////////////////////////////////////// flash save function

typedef struct {
    uint32_t sample_id;
    uint32_t flash_addr;   // address on W25Q
    uint32_t data_size;
    uint8_t  status;       // 1=valid, 2=deleted
    uint8_t  reserved[7];
} SampleEntry;



SampleEntry sample_table[MAX_SAMPLES];

bool add_to_table(uint32_t id, uint32_t addr, uint32_t size)
{
    if (table_count >= MAX_SAMPLES) {
        return false;        // table full
    }

    sample_table[table_count].sample_id  = id;
    sample_table[table_count].flash_addr = addr;
    sample_table[table_count].data_size  = size;
    sample_table[table_count].status     = 1;

    table_count++;
    return true;
}


// Start a new sample
uint32_t start_new_sample(uint32_t sample_id)
{
    uint32_t addr = current_write_pos;

    SampleHeader hdr = {
        .magic = 0x53414D50,
        .id = sample_id,
        .size = 0,           // will update at the end
        .status = 1
    };

   // w25q_write(addr, (uint8_t*)&hdr, sizeof(SampleHeader));

    current_write_pos = addr + sizeof(SampleHeader);
    return addr;             // return start address
}

// Stream one page / chunk
void append_chunk(const uint8_t* data, uint32_t len)
{
  //  w25q_write(current_write_pos, data, len);
    current_write_pos += len;
}

// Finish the sample
void finish_sample(uint32_t start_addr, uint32_t total_data_size)
{
    SampleHeader hdr;
    // Read current header first (to preserve magic & id)
  //  w25q_read(start_addr, (uint8_t*)&hdr, sizeof(SampleHeader));

    hdr.size = total_data_size;

   // w25q_write(start_addr, (uint8_t*)&hdr, sizeof(SampleHeader));

    add_to_table(hdr.id, start_addr, total_data_size);

    next_sample_id++;
}
// Call this when free space is low
void compact_flash(void)
{
    uint32_t read_pos = 0;           // not really used, we use the table
    uint32_t write_pos_new = 0;
    uint8_t  buffer[256];            // small buffer - adjust to your comfort

    // Process all entries in the lookup table
    for(uint32_t i = 0; i < table_count; i++)
    {
        if(sample_table[i].status != 1)
            continue;                    // skip deleted

        uint32_t old_addr = sample_table[i].flash_addr;
        uint32_t data_size = sample_table[i].data_size;
        uint32_t total_size = sizeof(SampleHeader) + data_size;

        // If it's already in the right place, just move write pointer
        if(old_addr == write_pos_new)
        {
            write_pos_new += total_size;
            continue;
        }

        // === Copy Header ===
        SampleHeader hdr;
       // w25q_read(old_addr, (uint8_t*)&hdr, sizeof(SampleHeader));

      //  w25q_write(write_pos_new, (uint8_t*)&hdr, sizeof(SampleHeader));

        // === Copy Data in small chunks ===
        uint32_t src = old_addr + sizeof(SampleHeader);
        uint32_t dst = write_pos_new + sizeof(SampleHeader);
        uint32_t remaining = data_size;

        while(remaining > 0)
        {
            uint32_t chunk = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining;

           // w25q_read(src, buffer, chunk);
           // w25q_write(dst, buffer, chunk);

            src += chunk;
            dst += chunk;
            remaining -= chunk;
        }

        // Update address in lookup table
        sample_table[i].flash_addr = write_pos_new;

        write_pos_new += total_size;
    }

    // Erase the old unused area (important!)
    if(write_pos_new < current_write_pos)
    {
       // w25q_erase_range(write_pos_new, current_write_pos - write_pos_new);
    }

    // Update global write position
    current_write_pos = write_pos_new;

    // Optional: Save table and positions to internal flash here
   // save_table_and_positions();
}
void controller_process(void){ // process incoming controller info ,16 bit address(low then high) , 8 bit data, 8 bit checksum
	uint8_t n;
	for (n = 0; n < 4; ++n) { // look for checksum,works, 3 digits , sample(1-4) , part(0-7),parameter (0-3)

		uint8_t checksum = usart3_rx_temp[n] ^ usart3_rx_temp[(n+1)&3]^ usart3_rx_temp[(n+2)&3]; //
		uint8_t play_set=1;
		if (usart3_rx_temp[(n+3)&3]==checksum){ // getting mixed even when checksum ok
			controller_address=(usart3_rx_temp[n]) | (usart3_rx_temp[(n+1)&3]<<8);
			controller_value=(usart3_rx_temp[(n+2)&3])&127;
			at32_led_toggle(LED2);


			// FX starts at 5000 ie 5110  , phaser=5010 delay=511

			if (controller_address>5000) controller_address-=4896;
			if ((controller_address>484)||(controller_address<110)) {  // if checksum is ok but data is bad
			usart3_rx_temp[4]=0;
			usart3_rx_temp[0]=0;usart3_rx_temp[1]=0;
			usart3_rx_counter=(usart3_rx_counter+1)&3;
			usart3_rx_counter=0;

			return; } //


			if (controller_value>127){ controller_address=0;} // if bad data, clear

			printf(" %d   ",controller_address);printf(" %d  \n",controller_value);
			uint8_t midi_generated[3]={153,1,127};  //trigger sampple playback
			uint8_t sample_select=((controller_address/100)-1)&15; // 0-3 for now
			uint8_t part_select=(((controller_address%100)/10)-1)&7; // part edited
			uint8_t feat_select=((controller_address%100)%10); // select between start, end, pitch,gap

			current_playing_part[sample_select]=part_select;

			switch (feat_select) { // modify values for sample , this works ok
				case 0:one_shot[sample_select].start[part_select]=sample_address_calculate(sample_select,controller_value); break; // start change enter
				case 1:one_shot[sample_select].length[part_select]=controller_value;
				one_shot[sample_select].end[part_select]=sample_address_calculate(sample_select,controller_value);break; // end  enter
				//case 2:
				//one_shot[sample_select].speed[part_select]=(0x10000*MAX_Rate)-((controller_value&127)<<10);break;// copy ratebreak;//pitch or playback speed
				case 2:
				one_shot[sample_select].speed[part_select]=(CNT_list[controller_value]<<1);break;// copy ratebreak;//pitch or playback speed

				case 3:one_shot[sample_select].gap[part_select]=controller_value;break; // following gap length
				case 4:lfo1_rate=controller_value;play_set=0;break;
				case 5:delay_time=controller_value;play_set=0;break;

				default:break;
			} // end of feat select

    		//one_play[sample_select].pointer=one_shot[sample_select].start[part_select];
    		//one_play[sample_select].position=0;
    		//one_play[sample_select].playback_rate=one_shot[sample_select].speed[part_select];



			if (play_set){
			if (!one_shot[sample_select].start[part_select]) one_shot[sample_select].start[part_select]=samples_store[sample_select].ram_addr; // in case start is 0

			if (part_select==1) sample_select+=5;
			if (!midi_in_clear){  // trigger midi note
				usart2_rx_buffer[0]=153;
				usart2_rx_buffer[1]=sample_select;
				usart2_rx_buffer[2]=127;
				midi_in_clear=1; }  // this is temp
			}
			break;
		}// end of checksum
	}// end of loop
	usart3_rx_temp[4]=0;  //clear receive flag
	}


