#include "bios.h"
#include "cpu.h"
#include "interconnect.h"
#include "ram.h"

int main(void) {
    Bios bios("SCPH1001.BIN");
    RAM ram;
    Interconnect inter(&bios, &ram);
    CPU cpu(&inter);

    cpu.run();

    return EXIT_SUCCESS;
}
