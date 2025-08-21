#include "bios.h"
#include "cpu.h"
#include "gpu.h"
#include "interconnect.h"
#include "ram.h"
#include "dma.h"
#include "commandbuffer.h"

int main(void) {
    Bios bios("SCPH1001.BIN");
    RAM ram;
    Dma dma;
    CommmandBuffer cb;
    GPU gpu(&cb);
    Interconnect inter(&bios, &ram, &dma, &gpu);
    CPU* cpu = new CPU(&inter);

    cpu->run();

    delete cpu;
    return EXIT_SUCCESS;
}
