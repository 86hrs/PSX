#include "bios.h"
#include "cpu.h"
#include "interconnect.h"
#include "ram.h"
#include "dma.h"

int main(void) {
    Bios bios("SCPH1001.BIN");
    RAM ram;
    Dma dma;
    Interconnect inter(&bios, &ram, &dma);
    CPU* cpu = new CPU(&inter);

    cpu->run();

    delete cpu;
    return EXIT_SUCCESS;
}
