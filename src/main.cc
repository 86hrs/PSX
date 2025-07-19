#include "bios.h"
#include "cpu.h"
#include "interconnect.h"
#include "map.h"

int main(void) {
  Bios bios("SCPH1001.BIN");
  Interconnect inter(&bios);
  CPU cpu(&inter);

  cpu.run();

  return EXIT_SUCCESS;
}
