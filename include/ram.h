void uart_receive_end(void);
void spi4_dma_rearm_tx(uint32_t mem_addr, uint16_t size);
void spi4_dma_rearm_rx(uint32_t mem_addr, uint16_t size);
void ram_page_read(uint32_t address,uint16_t size,uint8_t interface,int16_t* buf);
void ram_page_write(uint32_t address ,int16_t* data, uint16_t size,uint8_t interface);

void wk_dma_channel_config(dma_channel_type* dmax_channely, uint32_t peripheral_base_addr,
		uint32_t memory_base_addr, uint16_t buffer_size);
void read_busy(void);

void ram_write(uint32_t address,int8_t data){   // send 16bit only .. slow
		// 23 bits address max 4M word size

    	address=address<<1;   // only way to avoid corrupt last bit
    		uint8_t transmit[6]={0x02,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),
    				(uint8_t)(data>>8),(uint8_t)(data)};
    	   	uint8_t t_counter=0;
        //	while(spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);
        	SPI2_CS_LOW;
        	while (t_counter<6){
        		while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);  // wait for flag
        		spi_i2s_data_transmit(SPI2, transmit[t_counter]);
        		t_counter++;
        	}
        	while(spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);

        	SPI2_CS_HIGH;
}

int16_t ram_read(uint32_t address ){   // receive 16bit word  ,, 1k page size , this is slow
   // needs extra byte at the end to read correctly or it fails

		address=address<<1;
		uint8_t transmit[7]={0x03,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),0,0,0 };
    	int16_t receive_buf=0;
    	uint8_t rec_2[7];
    	uint8_t t_counter=0;



    	SPI2_CS_LOW;
    	while (t_counter<6){
    		while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);  // wait for flag
    		spi_i2s_data_transmit(SPI2, transmit[t_counter]);
    		if (t_counter>3) {

    			while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    		rec_2[t_counter] = spi_i2s_data_receive(SPI2);}
    		t_counter++;
    	}
    	while(spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);
    	SPI2_CS_HIGH;
    	receive_buf=rec_2[4]<<8;
    	receive_buf+=rec_2[5];
    	return receive_buf;

}
void testing(void){



		//spi_buf[spi_counter_2]=ram_read(spi_counter_2);

			//delay_ms(1);
		//}

			if (spi_counter_2>400) spi_counter_2=0; else spi_counter_2++;

}
void ram_page_read(uint32_t address,uint16_t size,uint8_t interface,int16_t* buf){   // receive 16bit word  ,add always +2 bytes to avoid losing data
   // needs extra byte at the end to read correctly or it fails
	//address has to even , always or lose data

		spi_read_pointer=buf;
		address=(address>>1)<<1;   // only way to avoid corrupt last bit
		//address=address&0xFFFFE0; //32 byte wrap junk

		while(spi_i2s_flag_get(SPI4, SPI_I2S_BF_FLAG) != RESET);
		SPI4_CS_HIGH;
		SPI2_CS_HIGH

		memset(ram_page_read_buf,0,512);

		uint8_t transmit[12]={0x03,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),3,4,5,6,7,8,9,10 };

		memcpy(ram_page_write_buf,transmit,4); // replace command bytes

		// leave extra for reading

		wk_dma_channel_config(DMA1_CHANNEL3,
				(uint32_t)&SPI4->dt,
				(uint32_t)ram_page_write_buf,
				size+4);

		wk_dma_channel_config(DMA1_CHANNEL2,
				(uint32_t)&SPI4->dt,
				(uint32_t)ram_page_read_buf,
				size+4);

		if(interface) {SPI2_CS_LOW;} else  {SPI4_CS_LOW;}  // select psram if 1
				//SPI2_CS_LOW;
		dma_channel_enable(DMA1_CHANNEL2, TRUE); // send

		dma_channel_enable(DMA1_CHANNEL3, TRUE);  //receive

		spi_read_flag=1;


	}

void ram_page_write(uint32_t address ,int16_t* data, uint16_t size,uint8_t interface){   // Size in bytes, interface 2=erase function on flash
	//address has to even , always or lose data


	memset(ram_page_write_buf,0,512);
	memcpy(ram_page_write_buf+4,data,size);  // copy to send buffer



		while(spi_i2s_flag_get(SPI4, SPI_I2S_BF_FLAG) != RESET);
		SPI2_CS_HIGH;
		SPI4_CS_HIGH;

		address=(address>>1)<<1;   // only way to avoid corrupt last bit
		//address=address&0xFFFFE0; //32 byte wrap junk
		uint8_t transmit[12]={0x02,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),3,4,5,6,7,8,9,10 };
		//if(interface==2) {transmit[0]=0xD8,interface=0;}  // block erase function
			memcpy(ram_page_write_buf,transmit,4); // replace command bytes
		   wk_dma_channel_config(DMA1_CHANNEL3,
		                         (uint32_t)&SPI4->dt,
								 (uint32_t)ram_page_write_buf,
		                         size+4);

			if(interface) {SPI2_CS_LOW;} else  {SPI4_CS_LOW;}  // select psram if 1
			dma_channel_enable(DMA1_CHANNEL3, TRUE);

			spi_write_flag=1;
}
void spi4_polling_rx(const uint8_t *data, uint16_t length)		//rx
{
    if (length == 0) return;

    SPI4_CS_LOW;                     // your CS pin for the new device

    for (uint16_t i = 0; i < length; i++)
    {
        while (spi_i2s_flag_get(SPI4, SPI_I2S_TDBE_FLAG) == RESET);
        spi_i2s_data_transmit(SPI4, data[i]);
		while(spi_i2s_flag_get(SPI4, SPI_I2S_RDBF_FLAG) == RESET);
		test_byte[i] = spi_i2s_data_receive(SPI4);

    }

    while (spi_i2s_flag_get(SPI4, SPI_I2S_BF_FLAG) != RESET);   // wait shift complete

    SPI4_CS_HIGH;
}


void spi4_polling_tx(const uint8_t *data, uint16_t length)		//spi4_polling_tx(ram_page_write_buf, your_length);
{
    if (length == 0) return;

    SPI4_CS_LOW;                     // your CS pin for the new device

    for (uint16_t i = 0; i < length; i++)
    {
        while (spi_i2s_flag_get(SPI4, SPI_I2S_TDBE_FLAG) == RESET);
        spi_i2s_data_transmit(SPI4, data[i]);
    }

    while (spi_i2s_flag_get(SPI4, SPI_I2S_BF_FLAG) != RESET);   // wait shift complete

    SPI4_CS_HIGH;
}

void spi2_polling_tx(const uint8_t *data, uint16_t length)		//spi4_polling_tx(ram_page_write_buf, your_length);
{
    if (length == 0) return;

    SPI2_CS_LOW;                     // your CS pin for the new device

    for (uint16_t i = 0; i < length; i++)
    {
        while (spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
        spi_i2s_data_transmit(SPI2, data[i]);
    }

    while (spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);   // wait shift complete

    SPI2_CS_HIGH;
}

void spi2_polling_rx(const uint8_t *data, uint16_t length)		//rx
{
    if (length == 0) return;

    SPI2_CS_LOW;                     // your CS pin for the new device

    for (uint16_t i = 0; i < length; i++)
    {
        while (spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
        spi_i2s_data_transmit(SPI2, data[i]);
		while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
		test_byte[i] = spi_i2s_data_receive(SPI2);

    }

    while (spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);   // wait shift complete

    SPI2_CS_HIGH;
}



void spi4_dma_rearm_tx(uint32_t mem_addr, uint16_t size)
{
    dma_channel_type *ch = DMA1_CHANNEL3;

    ch->ctrl &= ~0x00000001U;           // clear EN bit
    while (ch->ctrl & 0x00000001U);     // wait until disabled

    // Clear flags for Channel 3 (bits 12-15 in DMA1->clr)
    DMA1->clr = (1U << 12) | (1U << 13) | (1U << 14) | (1U << 15);

    wk_dma_channel_config(ch, (uint32_t)&SPI4->dt, mem_addr, size);

    // Control register for TX
    ch->ctrl = (1U << 4)  |     // DIR = 1   (Memory to Peripheral)
               (1U << 7)  |     // MINC = 1  (Memory increment)
               (0U << 6)  |     // PINC = 0
               (0U << 10) |     // MWIDTH 8-bit  (change to (1U<<10) for 16-bit)
               (0U << 8)  |     // PWIDTH 8-bit
               (1U << 1)  |     // TCIE = 1  (Transfer Complete interrupt)
               (1U << 13);      // Priority High  (bits 12-13 = 10b)

    ch->ctrl |= 0x00000001U;    // EN = 1
}

// RX: DMA1 Channel 2  (SPI4 -> Memory)
void spi4_dma_rearm_rx(uint32_t mem_addr, uint16_t size)
{
    dma_channel_type *ch = DMA1_CHANNEL2;

    ch->ctrl &= ~0x00000001U;
    while (ch->ctrl & 0x00000001U);

    // Clear flags for Channel 2 (bits 8-11)
    DMA1->clr = (1U << 8) | (1U << 9) | (1U << 10) | (1U << 11);

    wk_dma_channel_config(ch, (uint32_t)&SPI4->dt, mem_addr, size);

    // Control register for RX
    ch->ctrl = (0U << 4)  |     // DIR = 0   (Peripheral to Memory)
               (1U << 7)  |     // MINC = 1
               (0U << 6)  |
               (0U << 10) |     // MWIDTH 8-bit
               (0U << 8)  |     // PWIDTH 8-bit
               (1U << 1)  |     // TCIE = 1
               (1U << 13);      // Priority High

    ch->ctrl |= 0x00000001U;
}
void wk_dma_channel_config(dma_channel_type* dmax_channely, uint32_t peripheral_base_addr,
		uint32_t memory_base_addr, uint16_t buffer_size)
	{
	/* add user code begin dma_channel_config 0 */

	/* add user code end dma_channel_config 0 */

	dmax_channely->dtcnt = buffer_size;
	dmax_channely->paddr = peripheral_base_addr;
	dmax_channely->maddr = memory_base_addr;

	/* add user code begin dma_channel_config 1 */

	/* add user code end dma_channel_config 1 */
	}
	// Reliable re-arm for SPI4 DMA (Normal mode, variable size)
	// Call this every time you want to start a new transfer (TX, RX, or both)
	// TX: DMA1 Channel 3  (Memory -> SPI4)
void spi_message_process (void){
	uint16_t download_size=download_buffer; // sets max downloaded data per fetch mostly to get extra data if needed , minimum 128+2 for now
	switch(spi_process_counter){  // cue spi messages here
	case 0: if (psram_busy) spi_process_counter=15; else spi_process_counter=1;break;
	case 1:if (current_playing_sample[0]) {ram_page_read((one_shot[0].pointer+psram_sample_start),download_size,1,one_shot[0].buf);}
	break;				////////read from psram

	case 2:  ram_page_read((delay_pointer[0]*2) , download_size, 1,ram_out);break; // leave extra when reading, delay read

	case 3 : ram_page_write((delay_pointer[1]*2), ram_in,(audio_buffer_size*2),1);break;//delay_write
	case 4 : ram_page_write((psram_sample_start-357), test_int,256,1 );break;//test write ,might just run it always for now
	case 5:  memset(test_int_buf,0,254);ram_page_read((psram_sample_start-357) , 254, 1,test_int_buf);break; // test read back
	case 6 :if (current_playing_sample[1]) {ram_page_read((one_shot[1].pointer+psram_sample_start),download_size,1,one_shot[1].buf);}
	break;				////////read from psram

	case 7 :if (current_playing_sample[2]){ram_page_read((one_shot[2].pointer+psram_sample_start),download_size,1,one_shot[2].buf);}
	break;					////////read from psram
	case 8 :if (current_playing_sample[3]){ram_page_read((one_shot[3].pointer+psram_sample_start),download_size,1,one_shot[3].buf);}
	break;	// only read if enabled

 	default:break;



 	  }


 	 if (spi_process_counter>=8) spi_process_counter=0; else spi_process_counter++;

	}

void uart_receive_end(void){
		//if ((((uart_receive_timer[4]+2000) )<tmr_counter_value_get(TMR6))     )
			if ((sample_write_end_timer>100)     )
		{     // finish sample save, if this fails though its a problem , needs a better timer
			uart_receive_timer[3]=0;
			if (usart4_rx_counter&1) usart4_rx_counter+=1; //has to be even
			memcpy(usart4_int_buffer,usart4_rx_buffer,usart4_rx_counter);  // need to save the end part

			ram_page_write((usart4_total_counter+psram_sample_start+128),usart4_int_buffer,128,1);
			usart4_total_counter+=usart4_rx_counter;

			samples_store[current_sample_save].size_bytes=one_shot_var+usart4_rx_counter; // add end bit
			samples_store[current_sample_save].ram_addr=(usart4_total_counter-one_shot_var) & 0xFFFC00;  // not super accurate so clear one k
			samples_store[current_sample_save].used=1;
			samples_store[current_sample_save].speed=64;
			usart4_rx_counter=0;  //clear just in case random data
			usart4_total_counter+=4096; // add 4k
			usart4_total_counter&=0xFFFFF000;  // set to next 4k block start
			flash_counter_write(usart4_total_counter);// this migh tbe obsolet

			one_shot_var=0;
			psram_busy=0;
			current_sample_save++;  // this should just count up no extra info for now
			current_sample_save&=15;
			ram_to_flash_mirror (); // save all ram , but it needs to start from the beginning


			save_timer=66000; // settings backup
		} // this triggers after some time uart4 finished receiving


}
void uart_receive_save(void){ // only after 128 bytes
		// wav ignore 44 bytes initially
		if (usart4_total_counter> 16777086) return;    // quit when full, this is saved in flash until nearly full

		if (!uart_receive_timer[3]) { // runs after first time writing sample to ram
			int empty=-1;


			// this now has to write to the last address only , keep writing until nearly full then do a compact operation

			for (int i = 0; i < total_sample_count; i++){  // look for empty slot
				if (samples_store[i].used>1)samples_store[i].used=0;  // in case bad data
				if (!samples_store[i].used) {empty=i; break;}
			}
			//empty=0; // force only first slot for now , delete later
			current_sample_save=empty; // saves
			samples_store[empty].ram_addr=usart4_total_counter;
			samples_store[empty].used=1;
			flash_backup_start=usart4_total_counter; //sets starting address for flash backup
		} // this ok but can fail

		if ((one_shot_var&4095)==0)printf(" %d kb  \n",(usart4_total_counter>>10)); // sends back some data , works
		spi_write_flag=1; //needs to be set here

		ram_page_write((usart4_total_counter+psram_sample_start),usart4_int_buffer,128,1);

		psram_sample_write=0;  // waits for next message
		ADSR_counter_position[2]=767; // mute
		uart_receive_timer[3]=1;
		sample_write_end_timer=0;


		one_shot_var+=128; // in bytes
		usart4_total_counter+=128; // tracks all the samples


}
void uart4_command_process(void){

		// needs to reset if no command

	char text[10];
	memcpy(text,command_buffer,10);


	if (strncmp(text, "delete", 6) == 0) { //clear all
 //erase all when full then start from zero
		usart4_rx_reset=1;
		printf("deleting sample list \n");

	for (int i = 0; i < total_sample_count; i++){ samples_store[i].used=0;
	samples_store[i].ram_addr=0;samples_store[i].size_bytes=0; }  // clear all samples
	usart4_total_counter=0; //reset couner
	flash_counter_write(usart4_total_counter);

	usart4_rx_counter=0;  //clear just in case random data

	psram_busy=0;
	psram_sample_write=0;
	one_shot_var=0;
	save_timer=66000;  //delete records
	return;  // quit on clear
	}

	if (strncmp(text, "save", 4) == 0) { //copy selected sample from ram to flash  ie copy 1 16
		usart4_rx_counter=0;
		save_timer=66000;  //delete records
		return;
	}
	if (strncmp(text, "clear", 5) == 0) { //clear selected sample slots in ram 0-15 , this one now just to flag
		int id=-1;


		if (sscanf(text + 5, "%d", &id) == 1 && id >= 0 && id < total_sample_count) {
			printf("clearing selected sample %d  \n",id);


			samples_store[id].used=0;
			samples_store[id].ram_addr=0;samples_store[id].size_bytes=0;

		}usart4_rx_reset=1;
	}
	if (strncmp(text, "load", 4) == 0) { //load to selected slot if available 0-15

	}
	if (strncmp(text, "loadall", 7) == 0) { //load any number of samples to any free slot, default function

	}

	if (strncmp(text, "erase", 5) == 0) { //erase selected slot , this now will just flag data to be erased

	}
	if (strncmp(text, "settings", 7) == 0) { //erase selected slot in flash 0-15
		save_timer=66000;usart4_rx_reset=1;
	}



}
void flash_to_ram(void){  // copies all stored samples to ram , mirroring flash to ram ,starts above delay space, should be starting empty then fill up
	 int empty=-1;
	 int transfer_counter=0;
	 uint32_t sample_start=0;
	 uint32_t sample_size=0;

		for (int i = 0; i < total_sample_count; i++){  // look for empty slot
					if (samples_store[i].used>1)samples_store[i].used=0;  // in case bad data
					if (!samples_store[i].used) {empty=i; break;} // quit if no more data

					if (samples_store[i].used){  // if there is a sample

						sample_size=samples_store[i].size_bytes;
						sample_start=samples_store[i].ram_addr;

						for (int k = 0; k < sample_size; k+=128)

						ram_page_read((sample_start+psram_sample_start),128,0,usart4_int_buffer);
						ram_page_write((sample_start+psram_sample_start),usart4_int_buffer,128,1);

					} // end of transfer

		} //  end of loop


	 }// end of flash to ram


void ram_to_flash(void){  // copies samples to flash from ram, for now all of it when started
	 int empty=-1;
	 int transfer_counter=0;
	 uint32_t sample_start=0;
	 uint32_t sample_size=0;

		for (int i = 0; i < total_sample_count; i++){  // look for empty slot
					if (samples_store[i].used>1)samples_store[i].used=0;  // in case bad data
					if (!samples_store[i].used) {empty=i; break;} // quit if no more data

					if (samples_store[i].used){  // if there is a sample

						sample_size=samples_store[i].size_bytes;
						sample_start=samples_store[i].ram_addr;

						for (int k = 0; k < sample_size; k+=128)

						ram_page_read((sample_start+psram_sample_start),128,0,usart4_int_buffer);
						ram_page_write((sample_start+psram_sample_start),usart4_int_buffer,128,1);

					} // end of transfer

		} //  end of loop


	 }// end of flash to ram

void flash_to_ram_mirror (void){ // just copy the entire flash to ram ,16 mbyte
	 uint8_t test=0;
	 printf("Copying flash to ram. Let's go. \n");
	 for (uint32_t  i = psram_sample_start; i < 8388608; i+=256){
		 ram_page_read(i,256+2,0,usart4_int_buffer);//read flash,dma
		 while (spi_read_flag);
		 ram_page_write(i,usart4_int_buffer,256,1);// write to psram , 127 is bad
		 while (spi_write_flag);
	 }

}

void ram_to_flash_mirror (void){ // just copy the entire flash to ram ,500kb ,blocking , about 15s for 500kb
		// need to break it up eventually , ok for now
	printf("Writing to flash , this is slow , please wait \n");
	 uint32_t address=0;
	 uint8_t test=0;
	 // missing data in chunks but  random
	 	 uint32_t aligned=(flash_backup_start+psram_sample_start)&0xFFFF0000; // start copying on a 64kb block , not a problem

	for (uint32_t  i = aligned ; i < (655360+aligned); i+=256){ // 600kB for now, should be enough

		 ram_page_read(i,256+2,1,usart4_int_buffer);//read ram,dma  data is good

		 while (spi_read_flag);
		 // block erase  64kb = 2000ms max  ;
		  address=i;
		 uint8_t transmit[260]={0x03,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),3,4,5,6,7,8,9,10 };
		 memcpy(transmit+4,usart4_int_buffer,256);

		 if ((i&65535)==0){ // 64k block erase,  working ok , total about 2-3s
			 read_busy();
			 transmit[0]=0x06;
			 spi4_polling_tx(transmit,1);// write enable
			 read_busy();
			 transmit[0]=0xD8;
			 spi4_polling_tx(transmit,4);// erase
			 delay_ms(200);

			 read_busy();  // typ 150  max 2000ms


		 }//end of block erase

		 read_busy();
		 transmit[0]=0x06;
		 spi4_polling_tx(transmit,1);// write enable
		 read_busy();
		 transmit[0]=0x02;
		 spi4_polling_tx(transmit,260);// page write
		 read_busy(); // crude but for now only ,page program is 3ms max
		 delay_ms(1);


	 }
	printf("Finished writing to flash, Good bye. \n");
}
void read_busy(void){ // needs to have write enable on to work or hangs , do not use without it
	uint8_t temp[4]={0x05,0,0,0}; // check for busy

	uint8_t volatile flag=1;  // this is needed as it can stuck here , something with gpio toggling
	test_byte[0]=1; // 2 is write enable , 3 is write enable+busy
	test_byte[1]=1;
	test_byte[2]=1;
	test_byte[3]=1;
	//	HAL_Delay(20);
	 SPI2_CS_HIGH;
	while (flag) {

		//delay_ms(1);
		spi4_polling_rx(temp,4);  // it can be read continuosly so it is repeating while enabled
		//HAL_SPI_TransmitReceive (&hspi1,temp,temp2,2, 10); // request data , always leave extra room (clock) , works
		//HAL_GPIO_WritePin(CS1_GPIO_Port, CS1_Pin, 1);  // high end
		if ((test_byte[2]&3)==0) flag=0; // reads 2 always
		if ((test_byte[2]&3)==2) flag=0; // reads 2 always

	 }

}


