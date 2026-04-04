
void spi4_dma_rearm_tx(uint32_t mem_addr, uint16_t size);
void spi4_dma_rearm_rx(uint32_t mem_addr, uint16_t size);


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
void ram_page_read(uint32_t address,int16_t size,uint8_t interface){   // receive 16bit word  ,, 1k page size in ram
   // needs extra byte at the end to read correctly or it fails
	//while(spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);
	//SPI2_CS_HIGH;

		address=address<<1;
		while(spi_i2s_flag_get(SPI4, SPI_I2S_BF_FLAG) != RESET);
		SPI4_CS_HIGH;
		SPI2_CS_HIGH

		memset(ram_page_read_buf,0,256);

		uint8_t transmit[12]={0x03,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),3,4,5,6,7,8,9,10 };

		memcpy(ram_page_write_buf,transmit,4); // replace command bytes

		// leave extra for reading

		wk_dma_channel_config(DMA1_CHANNEL3,
				(uint32_t)&SPI4->dt,
				ram_page_write_buf,
				size+4);

		wk_dma_channel_config(DMA1_CHANNEL2,
				(uint32_t)&SPI4->dt,
				ram_page_read_buf,
				size+4);

		if(interface) {SPI2_CS_LOW;} else  {SPI4_CS_LOW;}  // select psram if 1
				//SPI2_CS_LOW;
		dma_channel_enable(DMA1_CHANNEL2, TRUE);

		dma_channel_enable(DMA1_CHANNEL3, TRUE);

		spi_read_flag=1;


	}

void ram_page_write(uint32_t address ,int16_t* data){   // psram tend to flip to 32byte wrap around mode , so now limit it 32 byte writes
		// 23 bits address max 4M word size


	memset(ram_page_write_buf,0,256);
	memcpy(ram_page_write_buf+4,data,128);  // copy to send buffer



		while(spi_i2s_flag_get(SPI4, SPI_I2S_BF_FLAG) != RESET);
		SPI2_CS_HIGH;
		SPI4_CS_HIGH;

		address=address<<1;   // only way to avoid corrupt last bit
		uint8_t transmit[12]={0x02,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),3,4,5,6,7,8,9,10 };

			memcpy(ram_page_write_buf,transmit,4); // replace command bytes
		   wk_dma_channel_config(DMA1_CHANNEL3,
		                         (uint32_t)&SPI4->dt,
								 ram_page_write_buf,
		                         133);
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
void wk_dma_channel_config(dma_channel_type* dmax_channely, uint32_t peripheral_base_addr, uint32_t memory_base_addr, uint16_t buffer_size)
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




