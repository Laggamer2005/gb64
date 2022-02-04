
#include <os_internal.h>
#include "faulthandler.h"
#include "render.h"
#include "debug_out.h"
#include "../controller.h"
#include "version.h"
#include "sprite.h"

#define FAULT_STACK_SIZE     0x200
#define HANG_DELAY           OS_USEC_TO_CYCLES(3 * 1000000)   

static OSThread faultThread;
static u64      faultThreadStack[FAULT_STACK_SIZE / sizeof(u64)];
static OSTime   lastHeartbeatTime;

static void     faultHandlerProc(void *);

void installFaultHandler(OSThread *targetThread)
{
	/*
	 * Create main thread
	 */
	osCreateThread(&faultThread, 4, faultHandlerProc, targetThread,
		   faultThreadStack + FAULT_STACK_SIZE / sizeof(u64), 11);

	osStartThread(&faultThread);
}

void dumpThreadInfo(OSThread *targetThread)
{
    DEBUG_PRINT_F("  pc=0x%08x\n", targetThread->context.pc);
    DEBUG_PRINT_F("  badvaddr=0x%08x\n", targetThread->context.badvaddr);
    DEBUG_PRINT_F("  ra=0x%x\n", (u32)targetThread->context.ra);
    DEBUG_PRINT_F("  cause=0x%x\n", targetThread->context.cause);
    // DEBUG_PRINT_F("  GB_PC=0x%x\n", (u32)targetThread->context.t1);
    // DEBUG_PRINT_F("  t4=0x%x\n", (u32)targetThread->context.t4);
    DEBUG_PRINT_F("  version=" EMU_VERSION "\n");
}

static void faultHandlerProc(void* arg)
{
    OSThread *targetThread = (OSThread*)arg;
    OSMesgQueue timerQueue;
    OSMesg timerMessages[10];
    OSTimer timer;
    
    osCreateMesgQueue(&timerQueue, timerMessages, 10);
    osSetTimer(&timer, 0, HANG_DELAY, &timerQueue, NULL);

    faultHandlerHeartbeat();

    while (1) {
        OSMesg msg;
        osRecvMesg(&timerQueue, &msg, OS_MESG_BLOCK);

        if (osGetTime() - lastHeartbeatTime >= HANG_DELAY) {
            DEBUG_PRINT_F("Main thread frozen:\n");
            dumpThreadInfo(targetThread);
            break;
        }

        OSThread *curr = __osGetNextFaultedThread(NULL);

        if (curr) {
            DEBUG_PRINT_F("Main thread faulted:\n");
            dumpThreadInfo(curr);
            break;
        }
    }

    osSetThreadPri(NULL, 11);
    
    OSContPad	**pad;

    while (1)
    {
        initSprites();
		pad = ReadController(0);

        if (pad[0]->button & START_BUTTON)
        {
            DEBUG_PRINT_F("Main thread:\n");
            dumpThreadInfo(targetThread);
        }

		renderFrame(1);
    }
}

void faultHandlerHeartbeat()
{
    lastHeartbeatTime = osGetTime();
}