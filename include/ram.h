void ram_write(uint32_t address,uint16_t data){   // send 16bit only
		// 23 bits address max 4M word size

    	address=address<<1;   // only way to avoid corrupt last bit
    		uint8_t transmit[6]={0x02,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),
    				(uint8_t)(data>>8),(uint8_t)(data)};
    	   	uint8_t t_counter=0;



        	while(spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);

        	SPI2_CS_LOW;
        	while (t_counter<6){
        		while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);  // wait for flag
        		spi_i2s_data_transmit(SPI2, transmit[t_counter]);

        		t_counter++;

        	}

        	while(spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);

        	__NOP();
        	SPI2_CS_HIGH;


}

uint16_t ram_read(uint32_t address){   // receive 16bit word  ,, 1k page size
   // needs extra byte at the end to read correctly or it fails

		address=address<<1;
		uint8_t transmit[7]={0x03,(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)(address),127,124,120 };
    	uint16_t receive_buf=0;
    	uint8_t rec_2[7];
    	uint8_t t_counter=0;



    	__NOP();
    	SPI2_CS_LOW;
    	while (t_counter<6){
    		while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);  // wait for flag
    		spi_i2s_data_transmit(SPI2, transmit[t_counter]);
    		while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    		rec_2[t_counter] = spi_i2s_data_receive(SPI2);
    		t_counter++;

    	}

    	while(spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);
    	SPI2_CS_HIGH;
    	receive_buf=rec_2[4]<<8;
    	receive_buf+=rec_2[5];
    	return receive_buf;

}
void testing(void){



		spi_buf[spi_counter_2]=ram_read(spi_counter_2);

			//delay_ms(1);
		//}

			if (spi_counter_2>400) spi_counter_2=0; else spi_counter_2++;

}
