void settings_storage(void){   // runs to store setting and read back

#define variable_count 6


	uint8_t *settings[variable_count]={
			(uint8_t*)ADSR_settings,
			(uint8_t*)sample_select,
			&cc_75,
			&cc_76,
			&cc_77,
			&delay_time};
			uint8_t settings_multi[variable_count]={16,32,1,1,1,1};   // sets length,  sound_set*x ,512 atm
			uint8_t settings_temp[96];
			uint16_t settings_total=0;  //adds up position , huge miss here retard alert
			uint8_t length=0; // max 64 atm
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
	flash_sector_erase(read_adr);
	flash_operation_wait_for(1000);

	for (i=0;i<256;i++){   //
		flash_byte_program(read_adr, all_settings[i]);

		  read_adr += 1;
	}
	flash_operation_wait_for(1000);



}
