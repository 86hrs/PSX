#include "bios.h"
#include "commandbuffer.h"
#include "cpu.h"
#include "dma.h"
#include "gpu.h"
#include "interconnect.h"
#include "ram.h"
#include <cstdlib>

int main(void) {
  Bios *bios = new Bios("SCPH1001.BIN");
  RAM *ram = new RAM();
  Dma *dma = new Dma();
  CommmandBuffer *cb = new CommmandBuffer();
  GPU *gpu = new GPU(cb);
  Interconnect *inter = new Interconnect(bios, ram, dma, gpu);
  CPU *cpu = new CPU(inter);

  while(1) {
    cpu->run();
  }

  delete bios, ram, dma, cb, gpu, inter, cpu;

  return EXIT_SUCCESS;
}
