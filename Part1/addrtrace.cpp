#include<stdio.h>
#include "pin.H"

typedef unsigned long long ull;


FILE* trace;
PIN_LOCK pinLock;
static ull memCount=0;

/*
 * The following is the analysis function called by
 * the instrumentation function that converts the 
 * the memory accesses into machine accesses, and traces
 * the machine accesses to a binary trace file.
 */
VOID RecordMemAccess(THREADID tid, VOID* addr, USIZE accSize)
{
    PIN_GetLock(&pinLock,tid+1);
    ull id=(ull)tid;
    ull memp=(ull)addr;
    ull sze=(ull)accSize;
    ull bound=((memp>>6)+1)<<6;
    while(sze>0)
    {  
        fwrite(&id,sizeof(ull),1,trace);
        fwrite(&memp,sizeof(ull),1,trace);
        fflush(trace);
        if(8<=sze && (memp+8)<=bound)
        {
            memp+=8;
            sze-=8;
        }
        else if(4<=sze && (memp+4)<=bound)
        {
            memp+=4;
            sze-=4;
        }
        else if(2<=sze && (memp+2)<=bound)
        {
            memp+=2;
            sze-=2;
        }
        else if(1<=sze && (memp+1)<=bound)
        {
            memp+=1;
            sze-=1;
        }
        memCount++;
        bound=((memp>>6)+1)<<6;
    }
	PIN_ReleaseLock(&pinLock);
}

/*
 * The following is the instrumentation function
 * used for instrumentating the memory operand accesses.
 */
VOID Instruction(INS ins, VOID *v)
{
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    USIZE accSize;
    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
        	accSize=INS_MemoryOperandSize(ins,memOp);
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess,
                IARG_THREAD_ID,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32,accSize,
                IARG_END);
        }
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
        	accSize=INS_MemoryOperandSize(ins,memOp);
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess,
                IARG_THREAD_ID,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32,accSize,
                IARG_END);
        }
    }
}


/* 
 *The following function is to print the total machine
 * access count and perform clean-up
 */
VOID Fini(INT32 code, VOID *v)
{
    printf("Total Machine Access Count : %llu\n",memCount);
    fclose(trace);
}


int main(int argc, char * argv[])
{
    trace = fopen("memAccessTrace.out", "wb");
    
    PIN_InitLock(&pinLock);

    // Initialize pin
    if (PIN_Init(argc, argv))
    {
        printf("Initialization Error!");
    }
	

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
