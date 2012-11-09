//                                DISCLAIMER
//
//               AT KEITHLEY METRABYTE WE TAKE PRIDE IN OUR SERVICING
//               OUR CUSTOMERS. WE ARE PLEASED TO OFFER YOU THIS SOFTWARE
//               AID PROGRAM FREE OF CHARGE.
//
//                  *******BEFORE INSTALLING THIS SOFTWARE*******
//                  *******PLEASE BE AWARE OF THE FOLLOWING******
//
//               THIS IS NOT A STANDARD KEITHLEY METRABYTE PRODUCT.
//
//               THIS SOFTWARE DOES NOT PROVIDE ANY WARRANTIES
//               AND IS NOT SUBJECT TO TECHNICAL SUPPORT.
//
//               KEITHLEY INSTRUMENTS INC., SHALL NOT BE LIABLE
//               FOR ANY SPECIAL, INCIDENTAL OR CONSEQUENTIAL
//               DAMAGES RELATED TO THE USE OF THIS SOFTWARE.
//
//               WE SINCERELY HOPE THAT THIS AID HELPS YOU GET THE
//               OPTIMAL USE OF YOUR KEITHLEY METRABYTE PRODUCT.
//
// DAS-1800ST Register Level Program Example

#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define base_adr        0x320
#define data_sel        base_adr + 2            // Data Select Register,R/W
#define dig_io          base_adr + 3            // Digital I/O Register,R/W
#define ctrl_a          base_adr + 4            // Control Register A,R/W
#define ctrl_b          base_adr + 5            // Control Register B,R/W
#define ctrl_c          base_adr + 6            // Control Register C,R/W
#define status          base_adr + 7            // Status Register,R/W
#define burst_length    base_adr + 8            // Burst Length Register,R/W
#define burst_rate      base_adr + 9            // Burst Mode Conv. Rate,R/W
#define qram_adr        base_adr + 10           // QRAM Address Start,R/W
#define counter_0       base_adr + 12           // Counter 0 Register,R/W
#define counter_1       base_adr + 13           // Counter 1 Register,R/W
#define counter_2       base_adr + 14           // Counter 2 Register,R/W
#define ctr_ctrl        base_adr + 15           // Counter Control Reg.,R/W

int  read_dig_in(void);
void scan_burst(void);
void scan_paced(void);
void set_chan_list(void);
void set_dig_out(int);
void set_pace_clk(void);
void software_measure(void);



unsigned int ad_data, ad_status, choice = 0;
unsigned int i;

void main(void)
{
	int dig_value;
	float vout;
 clrscr();
 do
 {
	gotoxy(1,1);
	printf("\n Enter 1 for Software Measure");
	printf("\n Enter 2 for Scan Burst Mode");
	printf("\n Enter 3 for Scan Paced Mode");
	printf("\n Enter 4 for Set Digital Out");
	printf("\n Enter 5 for Read Digital In");
	printf("\n\n");
	scanf("%d",&choice);
	switch(choice)
	{
		case 1:
			software_measure();
			break;
		case 2:
			scan_burst();
			break;
		case 3:
			scan_paced();
			break;
		case 4:
			printf("\n Enter Digital Output ");
			scanf("%d",&dig_value);
			set_dig_out(dig_value);
			break;
		case 5:
			i = read_dig_in();
			printf("\n Dig In = %d",i);
			break;
	  }

	delay(250);
 }
 while (choice <6);
 printf("\n\n Done");
}
int read_dig_in()
{
	return( (int)(inp(dig_io) & 0x0f));
}
void scan_burst()
{

	set_pace_clk();
	set_chan_list();
						// Set up the A/D
	outp(ctrl_c, 0x45);                     // Bipolar, SE, Burst, Int Clk
	outp(data_sel, 0x00);                   // data from A/D
	outp(burst_length, 4);                  // Burse length 4 chan
	outp(burst_rate, 9);                    // 100Khz
	outp(ctrl_a, 0x00);                     // soft gate,disable ctr,rst fifo
	outp(ctrl_a, 0x05);                     // soft gate,start ctr,enab fifo
	outp(status, 0x80);                     // Enable A/D conversions
	while (((inp(status) & 0x20) ==  0) && (!kbhit() ));
	outp(status, 0x00);                     // Disable A/D conversions
	outp(ctrl_a, 0x01);                     // soft gate,disable ctr,enab fifo
	outp(data_sel, 0x00);                   // select data from A/D
	printf("\n");
	for (i=0; i<4; i++)
	{
		ad_data = inpw(base_adr)& 0xfff;// Read A/D Twos Complement
		ad_data ^= 0x800;               // XOR to get Comp Bin Code
		printf("%d   %x h\t",i, ad_data);
	}
}
void scan_paced()
{
	set_pace_clk();
	set_chan_list();
						// Set up the A/D
	outp(ctrl_c, 0x41);                     // Bipolar, SE, Burst, Int Clk
	outp(data_sel, 0x00);                   // data from A/D
	outp(ctrl_a, 0x00);                     // soft gate,disable ctr,rst fifo
	outp(ctrl_a, 0x05);                     // soft gate,start ctr,enab fifo
	outp(status, 0x80);                     // Enable A/D conversions
	while ( ((inp(status) & 0x20) == 0) && (!kbhit() ));
	outp(status, 0x00);                     // Disable A/D conversions
	outp(ctrl_a, 0x01);                     // soft gate,disable ctr,enab fifo
	outp(data_sel, 0x00);                   // select data from A/D
	printf("\n");
	for (i=0; i<4; i++)
	{
		ad_data = inpw(base_adr)& 0xfff;// Read A/D Twos Complement
		ad_data ^= 0x800;               // XOR to get Comp Bin Code
		printf("%d %x h \t",i,ad_data);
	}
}

void set_chan_list()
{                                               
	outp(data_sel, 0x01);                   // select QRAM
	outp(qram_adr, 0x03);                   // # of  mux channels = 4
	outpw(base_adr, 0x00);                  // chan 0, gain 1
	outpw(base_adr, 0x101);                 // chan 1, gain 2 or 10
	outpw(base_adr, 0x02);                  // chan 2, gain 1
	outpw(base_adr, 0x03);                  // chan 3, gain 1
	outp(qram_adr, 0x03);                   // reset to start addr of qram
}                                               

void set_dig_out(int value)
{
	value &= 0xff;
	outpw(dig_io, value);
}

void set_pace_clk()
{
						// sample rate = 1KHz
	outp(ctr_ctrl, 0xb4);                   // Ctr 2, mode 2, lsb-msb
	outp(counter_2, 0x0a);                  // Lsb of 10 dec.
	outp(counter_2, 0x00);                  // Msb of 10 dec.
	outp(ctr_ctrl, 0x74);                   // Ctr 1, mode 2, lsb-msb
	outp(counter_1, 0xf4);                  // Lsb of 500 dec.
	outp(counter_1, 0x01);                  // Msb of 500 dec.
}
void software_measure()
{
						// Set up the QRAM
	outp(data_sel, 0x01);                   // select QRAM
	outp(qram_adr, 0x00);                   // starting addr for mux chan
	outp(base_adr, 0x00);                   // chan 0, gain 1
	outp(qram_adr, 0x00);                   // reset to start addr of qram
						// Set up the A/D
	outp(ctrl_c, 0x40);                     // Bipolar, SE
	outp(ctrl_a, 0x00);                     // reset fifo
	outp(data_sel, 0x00);                   // data from A/D
	outp(ctrl_a, 0x01);                     // enable fifo
	outp(status, 0x80);                     // Enable A/D conversions
	outp(base_adr, 0x00);                   // start A/D
	while ( (inp(status) & 0x40) == 0x00);  // wait for Fifo NOT Empty
	ad_data = inpw(base_adr)& 0xfff;        // Read A/D Twos Complement
	outp(ctrl_a, 0x00);                     // disable fifo
	ad_data ^= 0x800;                       // XOR to get Comp Bin Code
	printf("\n A/D Data %x h",ad_data);
}

