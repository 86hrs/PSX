#include "bios.h"
#include "commandbuffer.h"
#include "cpu.h"
#include "dma.h"
#include "gpu.h"
#include "interconnect.h"
#include "ram.h"
#include <cstdlib>

int main(void) {
  Bios bios("SCPH1001.BIN");
  RAM ram;
  Dma dma;
  CommmandBuffer cb;
  GPU gpu(&cb);
  Interconnect inter(&bios, &ram, &dma, &gpu);
  CPU *cpu = new CPU(&inter);

  while(1) {
    cpu->run();
  }

  return EXIT_SUCCESS;
}
