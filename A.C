void CycleAmpLowFilter(void)
{
int	filter;
int	newfilter;
int	i;

	filter = adinfo.channel[0].filter & 0x001F;
	if(filter == 0x0000){	// 0.1
	newfilter = 0x0001;
	}
	if(filter == 0x0001){	// 1
	newfilter = 0x0002;
	}
	if(filter == 0x0002){	// 10
	newfilter = 0x0004;
	}
	if(filter == 0x0004){	  // 100
	newfilter = 0x0008;
	}
	if(filter == 0x0008){		// 300
	newfilter = 0x0010;
	}
	if(filter == 0x0010){		// 600
	newfilter = 0x0018;
	}
	if(filter == 0x0018){		// 900
	newfilter = 0x0000;
	}
	for(i=0;i<NAMP_CHANNELS;i++){
	    adinfo.channel[i].filter &= 0xFFE0;
   	    adinfo.channel[i].filter |= newfilter;
	    SetAmpFilter(i,adinfo.channel[i].filter);
	}
}

void CycleAmpHighFilter(void)
{
int	filter;
int	newfilter;
int	i;

filter = adinfo.channel[0].filter & 0x00E0;
	if(filter == 0x0000){	// 50
	newfilter = 0x0020;
	}
	if(filter == 0x0020){	// 100
	newfilter = 0x0040;
	}
	if(filter == 0x0040){	// 3K
	newfilter = 0x0080;
	}
	if(filter == 0x0080){	// 6K
	newfilter = 0x000C0;
	}
	if(filter == 0x00C0){	  // 9K
	newfilter = 0x0020;
	}
	for(i=0;i<NAMP_CHANNELS;i++){
	adinfo.channel[i].filter &= 0xFF1F;
	adinfo.channel[i].filter |= newfilter;
	SetAmpFilter(i,adinfo.channel[i].filter);
	}
}

void GetAmpFilters(char *l, char *h)
{
int   i, j;

//	 for( i=0; i<NAMP_CHANNELS; i++){
        i = 0;
		j = adinfo.channel[i].filter;
		if ( (j & (BIT0  | BIT1 | BIT2 | BIT3 | BIT4)) == 0 ){	  // 0.1 Hz
			strcpy(l, "0.1\0");
		}
		if (j & BIT0){					// 1 HZ
			strcpy(l, "1\0");
		}
		if (j & BIT1){					// 10 Hz
			strcpy(l, "10\0");
		}
		if (j & BIT2){					// 100 Hz
			strcpy(l, "100\0");
		}
		if (j &  BIT3){		// 300Hz
			strcpy(l, "300\0");
		}
		if (j & BIT4){		// 600 Hz
			strcpy(l, "600\0");
		}
		if ((j & (BIT3 | BIT4))== (BIT3 | BIT4)){	// 900 Hz
			strcpy(l, "900\0");
		}

		/* now do the high freqs */
		if ((j & (BIT5 | BIT6 | BIT7))== 0 ){
			strcpy(h, "50\0");
		}
		if (j & BIT5){
			strcpy(h, "100\0");
		}
		if (j & BIT6 ){
			strcpy(h, "3KHz\0");
		}
		if (j & BIT7 ){
			strcpy(h, "6KHz\0");
		}
		if ((j & (BIT6 | BIT7)) == (BIT6 | BIT7) ){
			strcpy(h, "9KHz\0");
		}
//	}		
}

