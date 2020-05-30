/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1995, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright  law.  They  may not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 *************************************************************************/

/*---------------------------------------------------------------------*
        Copyright (C) 1997,1998 Nintendo. (Originated by SGI)
        
        $RCSfile: game.c,v $
        $Revision: 1.9 $
        $Date: 1999/04/16 07:11:47 $
 *---------------------------------------------------------------------*/

/*
 * File:  game.c
 *
 *
 */

#include <ultra64.h>
#include <PR/ramrom.h>		/* needed for argument passing into the app */
#include <assert.h>

#include "memory.h"
#include "game.h"
#include "static.h"
#include "controller.h"
#include "font_ext.h"
#include "src/gameboy.h"
#include "src/graphics.h"
#include "src/test/cpu_test.h"

/*
 * Task header.
 */
OSTask          taskHeader =
{
	M_GFXTASK,		/* type of task */
	OS_TASK_DP_WAIT,	/* flags - wait for DP to be available */
	NULL,			/* ucode boot (fill in later) */
	0,			/* ucode boot size (fill in later) */
	NULL,			/* ucode (fill in later) */
	SP_UCODE_SIZE,		/* ucode size */
	NULL,			/* ucode data (fill in later) (to init DMEM) */
	SP_UCODE_DATA_SIZE,	/* ucode data size */
	&dram_stack[0],		/* stack used by ucode */
	SP_DRAM_STACK_SIZE8,	/* size of stack */
	&rdp_output[0],		/* fifo output buffer start */
	&rdp_output[0]+RDP_OUTPUT_LEN,	/* fifo output buffer end */
	NULL,			/* display list pointer (fill in later) */
	0,			/* display list size (ignored) */
	NULL,			/* yield buffer (used if yield will occur) */
	0			/* yield buffer length */
};

/*
 * global variables
 */
Gfx		*glistp;	/* RSP display list pointer */
Dynamic		dynamic;	/* dynamic data */
int		draw_buffer=0;	/* frame buffer being updated (0 or 1) */
int		fontcol[4];	/* color for shadowed fonts */


extern char     _gbromSegmentRomStart[];
extern char     _gbromSegmentRomEnd[];

#define RUN_TESTS 0
				
/*
 * macros 
 */
#define FONTCOL(r,g,b,a)        {fontcol[0]=r;  \
                                fontcol[1]=g;   \
                                fontcol[2]=b;   \
                                fontcol[3]=a;}
#define FONTCOLM(c)             FONTCOL(c)
#define SHOWFONT(glp,str,x,y)   {                                             \
                font_set_color( 0,0,0,255);                                   \
                font_set_pos( (x)+(1), (y)+(0) );                             \
                font_show_string( glp, str );                                 \
                font_set_pos( (x)+(0), (y)+(1) );                             \
                font_show_string( glp, str );                                 \
                font_set_color( fontcol[0],fontcol[1],fontcol[2],fontcol[3]); \
                font_set_pos( x, y );                                         \
                font_show_string( glp, str );}

/*
 * This is the main routine of the app.
 */
void
game(void)
{
    OSTask      *theadp;
    Dynamic     *dynamicp;
    OSContPad	**pad;
    char 	*cstring;
    char	str[200];
	int x, y;
	int loop, offset, color;
	int lastButton;
	int cyclesToRun;
	int cycleStep;
	int line = 0;
	OSTime lastTime;
	OSTime lastDrawTime;
	OSTime frameTime;
	void* debugWrite;

	x = 18;
	y = 18;
	loop = 0;
	offset = 0;
	color = 0;
	lastButton = 0;
	line = -1;

	frameTime = 0;
	lastDrawTime = 0;
	lastTime = 0;
	cyclesToRun = 0;
	gGameboy.cpu.cyclesRun = 0;
	cycleStep = 0;// 1024 * 1024 / 30;

	sprintf(str, "Didn't run tests %X", &gGameboy);
#if RUN_TESTS
	runTests(str);
#endif

	initGameboy(&gGameboy, &gGBRom);

	// WRITE_REGISTER_DIRECT(&gGameboy.memory, REG_LCDC, LCDC_BG_TILE_DATA);

	gGameboy.memory.misc.colorPalletes[0] = 0x0842;
	gGameboy.memory.misc.colorPalletes[1] = 0x1084;
	gGameboy.memory.misc.colorPalletes[2] = 0x2108;
	gGameboy.memory.misc.colorPalletes[3] = 0x4210;

	gGameboy.memory.misc.monochromePallete[0] = 
		gGameboy.memory.misc.monochromePalleteSource[0];
	gGameboy.memory.misc.monochromePallete[1] = 
		gGameboy.memory.misc.monochromePalleteSource[1];
	gGameboy.memory.misc.monochromePallete[2] = 
		gGameboy.memory.misc.monochromePalleteSource[2];
	gGameboy.memory.misc.monochromePallete[3] = 
		gGameboy.memory.misc.monochromePalleteSource[3];

	gGameboy.memory.vram.tiles[1].rows[0] = 0xFFFF;
	gGameboy.memory.vram.tiles[1].rows[1] = 0x8181;
	gGameboy.memory.vram.tiles[1].rows[2] = 0x8181;
	gGameboy.memory.vram.tiles[1].rows[3] = 0x8181;
	gGameboy.memory.vram.tiles[1].rows[4] = 0x8181;
	gGameboy.memory.vram.tiles[1].rows[5] = 0x8181;
	gGameboy.memory.vram.tiles[1].rows[6] = 0x8181;
	gGameboy.memory.vram.tiles[1].rows[7] = 0xFFFF;
	
	gGameboy.memory.vram.tiles[2].rows[0] = 0x0F33;
	gGameboy.memory.vram.tiles[2].rows[1] = 0x0F33;
	gGameboy.memory.vram.tiles[2].rows[2] = 0x0F33;
	gGameboy.memory.vram.tiles[2].rows[3] = 0x0F33;
	gGameboy.memory.vram.tiles[2].rows[4] = 0x0F33;
	gGameboy.memory.vram.tiles[2].rows[5] = 0x0F33;
	gGameboy.memory.vram.tiles[2].rows[6] = 0x0F33;
	gGameboy.memory.vram.tiles[2].rows[7] = 0x0F33;
	
	gGameboy.memory.vram.tiles[3].rows[0] = 0x330F;
	gGameboy.memory.vram.tiles[3].rows[1] = 0x330F;
	gGameboy.memory.vram.tiles[3].rows[2] = 0x330F;
	gGameboy.memory.vram.tiles[3].rows[3] = 0x330F;
	gGameboy.memory.vram.tiles[3].rows[4] = 0x330F;
	gGameboy.memory.vram.tiles[3].rows[5] = 0x330F;
	gGameboy.memory.vram.tiles[3].rows[6] = 0x330F;
	gGameboy.memory.vram.tiles[3].rows[7] = 0x330F;

	gGameboy.memory.vram.tiles[32].rows[0] = 0x5555;
	gGameboy.memory.vram.tiles[32].rows[1] = 0xAAAA;
	gGameboy.memory.vram.tiles[32].rows[2] = 0x5555;
	gGameboy.memory.vram.tiles[32].rows[3] = 0xAAAA;
	gGameboy.memory.vram.tiles[32].rows[4] = 0x5555;
	gGameboy.memory.vram.tiles[32].rows[5] = 0xAAAA;
	gGameboy.memory.vram.tiles[32].rows[6] = 0x5555;
	gGameboy.memory.vram.tiles[32].rows[7] = 0xAAAA;

	for (loop = 0; loop < 1024; ++loop)
	{
		gGameboy.memory.vram.tilemap0[loop] = 0;
	}

	gGameboy.memory.vram.tilemap0[0] = 1;

	zeroMemory(cfb, sizeof(u16) * 2 * SCREEN_WD * SCREEN_HT);

	debugWrite = (void*)(0x80700000 - 12);
	*((u32*)debugWrite) = (int)gGameboy.memory.rom;
	debugWrite = (void*)(0x80700000 - 16);
	*((u32*)debugWrite) = (int)unloadBIOS;

    /*
     * Main game loop
     */
    /*osSyncPrintf("Use the L button and crosshair for menu options.\n");*/
    while (1) {
		pad = ReadController(0);

		if (gGameboy.cpu.cyclesRun == 314568)
		{
			// cycleStep = 1;
		}

		if (pad[0]->button && !lastButton)
		{
			cycleStep = 1024 * 1024 / 30;

			if (pad[0]->button & CONT_A)
			{
				WRITE_REGISTER_DIRECT(
					&gGameboy.memory, 
					REG_SCY, 
					READ_REGISTER_DIRECT(&gGameboy.memory, REG_SCY) + 1
				);
			}
			else if (pad[0]->button & CONT_B)
			{
				WRITE_REGISTER_DIRECT(
					&gGameboy.memory, 
					REG_SCY, 
					READ_REGISTER_DIRECT(&gGameboy.memory, REG_SCY) - 1
				);
			}
		}

		lastButton = pad[0]->button;

		frameTime = lastTime;
		lastTime = osGetTime();

		frameTime = lastTime - frameTime;

		cstring=str;
		
#if !RUN_TESTS
		lastDrawTime = -osGetTime();

		// gameboy is 60 fps N64 is 30
		// we need to emulator two frames
		// for every one frame rendered
		emulateFrame(&gGameboy, NULL);
		emulateFrame(&gGameboy, cfb[draw_buffer]);

		osWritebackDCache(cfb[draw_buffer], sizeof(u16) * SCREEN_WD*SCREEN_HT);

		lastDrawTime += osGetTime();
		sprintf(cstring, "Cycles run %d\nFrame Time %d\nEmu time %d\n%X", 
			gGameboy.cpu.cyclesRun, 
			(int)OS_CYCLES_TO_USEC(frameTime) / 1000, 
			(int)OS_CYCLES_TO_USEC(lastDrawTime) / 1000, 
			runCPU
		);
#endif

		/*
		* pointers to build the display list.
		*/
		theadp = &taskHeader;
		dynamicp = &dynamic;
		glistp = dynamicp->glist;

		/*
		* Tell RCP where each segment is
		*/
		gSPSegment(glistp++, 0, 0x0);	/* physical segment */
		gSPSegment(glistp++, STATIC_SEGMENT, 
				OS_K0_TO_PHYSICAL(staticSegment));
		gSPSegment(glistp++, CFB_SEGMENT, 
				OS_K0_TO_PHYSICAL(cfb[draw_buffer]));
		gSPSegment(glistp++, TEXTURE_SEGMENT, 
				OS_K0_TO_PHYSICAL(textureSegment));

		/*
		* Clear z and color framebuffer.
		*/
		gSPDisplayList(glistp++, clearzbuffer_dl);

		gDPSetFillColor(glistp++, GPACK_RGBA5551(
				(int)1,
				(int)1,
				(int)1,
				1) << 16 |
				GPACK_RGBA5551(
				(int)1,
				(int)1,
				(int)1,
				1));
		gSPDisplayList(glistp++, clearcfb_dl);

		/*
		* Initialize RCP state.
		*/
		gSPDisplayList(glistp++, init_dl);

		gDPPipeSync(glistp++);
		gDPSetCycleType(glistp++, G_CYC_1CYCLE);

		/*
		* Draw Text
		*/
		{
			/* 
			* neccessary after lines
			*/
			gDPSetScissor(glistp++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);

			font_init( &glistp );
			font_set_transparent( 1 );
			
			font_set_scale( 1.0, 1.0 );
			font_set_win( 200, 1 );

			FONTCOL(55, 255, 155, 255);
		
			cstring = str;

			y = 18;

			while (*cstring)
			{
					SHOWFONT(&glistp,cstring,x,y);

					while (*cstring && *cstring != '\n')
					{
							++cstring;
					}

					if (*cstring)
					{
							++cstring;
					}

					y += 20;
			}
		}

		font_finish( &glistp );


		gDPFullSync(glistp++);
		gSPEndDisplayList(glistp++);

	#ifdef DEBUG
	#ifndef __MWERKS__
		assert((glistp - dynamicp->glist) < GLIST_LEN);
	#endif
	#endif

		/*
		* Build graphics task:
		*
		*/
		theadp->t.ucode_boot = (u64 *) rspbootTextStart;
		theadp->t.ucode_boot_size = 
				(u32) rspbootTextEnd - (u32) rspbootTextStart;

		/*
			* RSP output over FIFO to RDP: 
			*/
		theadp->t.ucode = (u64 *) gspF3DEX2_fifoTextStart;
		theadp->t.ucode_data = (u64 *) gspF3DEX2_fifoDataStart;

		/*
		* initial display list: 
		*/
		theadp->t.data_ptr = (u64 *) dynamicp->glist;
		theadp->t.data_size = 
			(u32) ((glistp - dynamicp->glist) * sizeof(Gfx));

		/*
		* Write back dirty cache lines that need to be read by the RCP.
		*/
		osWritebackDCache(&dynamic, sizeof(dynamic));

		/*
		* start up the RSP task
		*/
		osSpTaskStart(theadp);

		/*
		* wait for RDP completion 
		*/
		(void)osRecvMesg(&rdpMessageQ, NULL, OS_MESG_BLOCK);

		/*
		* setup to swap buffers 
		*/
		osViSwapBuffer(cfb[draw_buffer]);

		/*
		* Make sure there isn't an old retrace in queue 
		* (assumes queue has a depth of 1) 
		*/
		while (!MQ_IS_EMPTY(&retraceMessageQ)) {
			(void) osRecvMesg(&retraceMessageQ, NULL, OS_MESG_NOBLOCK);
		}

		/*
		* Wait for Vertical retrace to finish swap buffers 
		*/
		(void) osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);

		draw_buffer ^= 1;
    }
}
