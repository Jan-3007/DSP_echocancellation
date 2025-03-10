// FM4_slider_interface.c
/*
	Code module to support the FM4 S6E2CC GUI slider interface app
	
	Six slider variables are stored in the global float array P_vals[6]. The initial
	values given to this array in the main module are the values used when a
	real-time DSP application first launches on the FM4 S6E2CC board.

  The main component in making this interface work is a UART running through the
	virtual comm port provided along side the CMSIS-DAP debug interface on the board.

  Copyright (C) 2016 Mark Wickert
	
	
	The Spansion (now part of Cypress) software is acknowledged below.
*/

/*******************************************************************************
* Copyright (C) 2013 Spansion LLC. All Rights Reserved.
*
* This software is owned and published by:
* Spansion LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with Spansion
* components. This software is licensed by Spansion to be adapted only
* for use in systems utilizing Spansion components. Spansion shall not be
* responsible for misuse or illegal use of this software for devices not
* supported herein.  Spansion is providing this software "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the software.
*
* SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
* WARRANTY OF NONINFRINGEMENT.
* SPANSION SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
* SAVINGS OR PROFITS,
* EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
* FROM, THE SOFTWARE.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Disclaimer and Copyright notice must be
* included with each copy of this software, whether used in part or whole,
* at all times.
*/

/* Change history:
 *
 * ter, jan 02, 2019: 	code modified to support ',' as decimal separator
 * ter, jan 30, 2019: 	code modified to support latest version of PDL (rev 2.1.0).
 * 						init_slider_interface: 	Updated init for stcMfsUartCfg structure
 * 												UART init updated following according examples
 * 						update_slider_parameters: 	Receive buffer no longer needed,
 * 													variable receiveData used instead
 * 													German / US locale support now directly in code
 * 						write_uart0: Completely new
 * 						initUart0: All UART related stuff from init_slider_interface  moved to new routine Uart0Init
 * ter, feb 06, 2019: 	function 'update_LI_HP' added
 *
 *
 */

#include <FM4_slider_interface.h>


/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/

void init_slider_interface(struct FM4_slider_struct *FM4_slider,
                              float32_t P_val0, float32_t P_val1, float32_t P_val2,
                              float32_t P_val3, float32_t P_val4, float32_t P_val5)
{
	//Fill structure with variables for parameter slider communication
	// Slider values
	FM4_slider->P_vals[0] = P_val0;
	FM4_slider->P_vals[1] = P_val1;
	FM4_slider->P_vals[2] = P_val2;
	FM4_slider->P_vals[3] = P_val3;
	FM4_slider->P_vals[4] = P_val4;
	FM4_slider->P_vals[5] = P_val5;
	FM4_slider->H_found = 0;

	// clear buffer and counters
	FM4_slider->idx_P_rec = 0;
	FM4_slider->P_idx = 0;

}
	
/* update_slider_parameters: If data is received from UART,
 * get character from internal buffer and decode char string into slider float32_t
 * held in P_vals[] array. Receive data from UART asynchronously (Non-blocking)
 *
 */
void update_slider_parameters(struct FM4_slider_struct *FM4_slider)
{
	uint8_t receiveData;

	while(Mfs_Uart_GetStatus(&UART0, UartRxFull) == TRUE)
		{
			receiveData = Mfs_Uart_ReceiveData(&UART0);

			if (receiveData >= 0x2C)
			{
					Mfs_Uart_SendData(&UART0, receiveData);
					//Wait for header char 'H'
					if (FM4_slider->H_found == 0)
					{
						if (receiveData == 0x48) // 'H' <=> 0x48
						{
							FM4_slider->H_found = 1;
						}
					}
					else
					{
						if ((receiveData >= 0x30 && receiveData <= 0x39) ||  		// 0 - 9 ||
							(receiveData == 0x2E) || (receiveData == 0x2C) ||		// . , ||
							(receiveData == 0x2D))									// -
						{
							if (receiveData == 0x2C) FM4_slider->P_rcvd[FM4_slider->idx_P_rec] = 0x2E; // support for german locale
							else FM4_slider->P_rcvd[FM4_slider->idx_P_rec] = receiveData;
							FM4_slider->idx_P_rec++;
						}
						else	if (receiveData == 0x3A)  // ':' <=> 0x3A
								{
									FM4_slider->P_idx = (uint8_t) atoi(FM4_slider->P_rcvd);
									FM4_slider->idx_P_rec = 0;
									memset(FM4_slider->P_rcvd, 0, sizeof(FM4_slider->P_rcvd)); //clear received char array
								}
						else 	if (receiveData == 0x54) // 'T' <=> 0x54
								{
									FM4_slider->P_vals[FM4_slider->P_idx] = (float32_t) atof(FM4_slider->P_rcvd);
									memset(FM4_slider->P_rcvd, 0, sizeof(FM4_slider->P_rcvd)); // //clear received char array
									FM4_slider->idx_P_rec = 0;
									FM4_slider->H_found = 0;
								}
						else
						{
							//memset(P_rcvd, 0, sizeof(P_rcvd));
							FM4_slider->H_found = 0;
						}
					}
				}
			}
}

/* update_LI_HP: Check if slider 4 / 5 values have changed
 * and update CODEC line in and head phone levels accordingly
 * Input: Pointer to slider structure
 */
void setLIandHPlevels (struct FM4_slider_struct *FM4_slider)
{
	 static float32_t oldPvals5 = 0.0; // local storage for slider params
	 static float32_t oldPvals4 = 0.0;

		// check if new values are available
		if (oldPvals5 != FM4_slider->P_vals[5])
		{																						// Left / Right Headphone Out
			Wm8731_SendCmd(WM8731_REG_LHPHONE_OUT, (uint16_t)FM4_slider->P_vals[5] | 0x0100);	// with	LRINBOTH bit set
			oldPvals5 = FM4_slider->P_vals[5];
		}

		if (oldPvals4 != FM4_slider->P_vals[4])
		{																						// Left / Right Line In
			Wm8731_SendCmd( WM8731_REG_LLINE_IN, (uint16_t)FM4_slider->P_vals[4] | 0x0100);		// with LRHPBOTH
			oldPvals4 = FM4_slider->P_vals[4];
		}
}

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
