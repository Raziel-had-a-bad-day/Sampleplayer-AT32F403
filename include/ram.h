
void spi4_dma_rearm_tx(uint32_t mem_addr, uint16_t size);
void spi4_dma_rearm_rx(uint32_t mem_addr, uint16_t size);
void ram_page_write(uint32_t address ,int16_t* data,uint16_t size);
void wk_dma_channel_config(dma_channel_type* dmax_channely, uint32_t peripheral_base_addr,
		uint32_t memory_base_addr, uint16_t buffer_size);

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
void ram_page_read(uint32_t address,uint16_t size,uint8_t interface,int16_t* buf){   // receive 16bit word  ,, 1k page size in ram
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

void ram_page_write(uint32_t address ,int16_t* data, uint16_t size){   // Size in bytes
	//address has to even , always or lose data


	memset(ram_page_write_buf,0,512);
	memcpy(ram_page_write_buf+4,data,size);  // copy to send buffer



		while(spi_i2s_flag_get(SPI4, SPI_I2S_BF_FLAG) != RESET);
		SPI2_CS_HIGH;
		SPI4_CS_HIGH;

		address=(address>>1)<<1;   // only way to avoid corrupt last bit
		//address=address&0xFFFFE0; //32 byte wrap junk
		uint8_t transmit[12]={0x02,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),3,4,5,6,7,8,9,10 };

			memcpy(ram_page_write_buf,transmit,4); // replace command bytes
		   wk_dma_channel_config(DMA1_CHANNEL3,
		                         (uint32_t)&SPI4->dt,
								 (uint32_t)ram_page_write_buf,
		                         size+4);
			SPI2_CS_LOW;

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

 	 switch(spi_process_counter){  // cue spi messages here
 	  case 0: if (psram_busy) spi_process_counter=15; else spi_process_counter=1;break;
 	  case 1:ram_page_read((one_shot_pointer+psram_sample_start),((audio_buffer_size*2)+2),1,flash_sample_buf);break;				////////read from psram


 	  case 2:  ram_page_read((delay_pointer[0]*2) , ((audio_buffer_size*2)+2), 1,ram_out);break; // leave extra when reading

 	  case 3 : ram_page_write((delay_pointer[1]*2), ram_in,(audio_buffer_size*2));break;//dealy_write
 	  case 4 : ram_page_write((psram_sample_start-357), test_int,256 );break;//might just run it always for now
 	 case 5:  memset(test_int_buf,0,254);ram_page_read((psram_sample_start-357) , 254, 1,test_int_buf);break;


 	 default:break;



 	  }


 	 if (spi_process_counter>=5) spi_process_counter=0; else spi_process_counter++;

	}

void uart_receive_end(void){
		//if ((((uart_receive_timer[4]+2000) )<tmr_counter_value_get(TMR6))     )
			if ((sample_write_end_timer>100)     )
		{     // finish sample save, if this fails though its a problem , needs a better timer
			uart_receive_timer[3]=0;
			if (usart4_rx_counter&1) usart4_rx_counter+=1; //has to be even
			memcpy(usart4_int_buffer,usart4_rx_buffer,usart4_rx_counter);  // need to save the end part

			ram_page_write((usart4_total_counter+psram_sample_start+128),usart4_int_buffer,128);
			usart4_total_counter+=usart4_rx_counter;

			samples_store[current_sample_save].size_bytes=one_shot_var+usart4_rx_counter; // add end bit
			samples_store[current_sample_save].ram_addr=usart4_total_counter-one_shot_var;
			samples_store[current_sample_save].used=1;

			one_shot_var=0;
			psram_busy=0;
			current_sample_save++;
			current_sample_save&=15;

		} // this triggers after some time uart4 finished receiving


}
void uart_receive_save(void){
		// wav ignore 44 bytes initially
		if (usart4_total_counter> 16777086) return;    // quit when full

		if (!uart_receive_timer[3]) { // runs after first time writing sample to ram
/*			int empty=-1;

			for (int i = 0; i < total_sample_count; i++){  // look for empty slot
				if (samples_store[i].used>1)samples_store[i].used=0;  // in case bad data
				if (!samples_store[i].used) {empty=i; break;}
			}

			if (empty==-1) { //quit when full
			usart_data_transmit(UART4,(char)('X'));
			psram_sample_write=0; // will send non stop
			return;
			}
			current_sample_save=empty; // saves
			samples_store[empty].ram_addr=usart4_total_counter;
			samples_store[empty].used=1;*/
		} // this ok but can fail

		if ((one_shot_var&4095)==0)usart_data_transmit(UART4,(usart4_total_counter>>12)); // sends back some data
		spi_write_flag=1; //needs to be set here

		ram_page_write((usart4_total_counter+psram_sample_start),usart4_int_buffer,128);
		psram_sample_write=0;  // waits for next message

		uart_receive_timer[3]=1;
		sample_write_end_timer=0;


		one_shot_var+=128; // in bytes
		usart4_total_counter+=128; // tracks all the samples


}
