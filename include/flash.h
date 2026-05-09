



void settings_storage(void){   // runs to store setting and read back

#define variable_count 7


	uint8_t *settings[variable_count]={
			(uint8_t*)ADSR_settings,
			(uint8_t*)sample_select,
			&cc_75,
			&cc_76,
			&cc_77,
			&delay_time,
			(uint8_t*) samples_backup};// might expand it a little



			uint16_t settings_multi[variable_count]={16,32,1,1,1,1,sizeof(samples_backup)};   // sets length,  sound_set*x ,512 atm
			uint8_t settings_temp[512];
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

	for (i=0;i<512;i++){   //
		flash_byte_program(read_adr, all_settings[i]);

		  read_adr += 1;
	}
	flash_operation_wait_for(1000);



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


