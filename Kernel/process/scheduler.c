#include <scheduler.h>
#include <processLinkedList.h>
#include <defs.h>

static uint64_t quantum = 0;
static ProcessLinkedPtr processList = NULL;

void initScheduler(ProcessLinkedPtr list) {
    processList = list;
}

#define QUANTUM 10

uint64_t schedule(uint64_t rsp){
    return rsp; // por ahora no hace nada
    /*
    if(processList == NULL) {
        return rsp; // No hay procesos para programar
    }
    
    if (quantum == 0) {
        PCB* currentProcess = getNextProcess(processList);
        quantum = QUANTUM; // dsp ver de calcular segun la prioridad
        return currentProcess->registers->rsp;
    }
    // Si hay tiempo restante, se reduce y se devuelve el rsp actual
    quantum--;
    return rsp;
    */
}