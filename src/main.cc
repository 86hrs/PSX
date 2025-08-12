#include "bios.h"
#include "cpu.h"
#include "gpu.h"
#include "interconnect.h"
#include "ram.h"
#include "dma.h"

int main(void) {
    Bios bios("SCPH1001.BIN");
    RAM ram;
    Dma dma;
    GPU gpu;
    Interconnect inter(&bios, &ram, &dma, &gpu);
    CPU* cpu = new CPU(&inter);

    cpu->run();

    delete cpu;
    return EXIT_SUCCESS;
}
