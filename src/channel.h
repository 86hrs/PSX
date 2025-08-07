#include <cstdint>
#include <optional>

// DMA transfer direction
enum Direction : uint32_t {
    ToRam = 0,
    FromRam = 1,
};

// DMA transfer step
enum Step : uint32_t {
    Increment = 0,
    Decrement = 1,
};

// DMA transfer synchronization mode
enum Sync : uint32_t {
    // Transfer starts when the CPU writes to the Trigger bit
    // and
    // transfers everything at once
    Manual = 0,
    // Sync blocks to DMA requests
    Request = 1,
    // Used to transfer GPU command lists
    LinkedList = 2,
};

struct Channel {
    bool enable;
    Direction direction;
    Step step;
    Sync sync;

    // Used to start the DMA transfer when 'sync' is 'Manual'
    bool trigger;
    // If true that DMA "chops" the transfer and lets the CPU run
    // in the gaps;
    bool chop;
    // Chopping DMA window size (log2 number of words)
    uint8_t chop_dma_sz;
    // Chopping CPU window size (log2 number of cycles)
    uint8_t chop_cpu_sz;
    // Unkwon 2 RW bits in configuration register
    uint8_t dummy;

    // DMA start address
    uint32_t base;

    // Size of a block in words
    uint16_t block_size;

    // Block count, ONly used when `sync` is `Request`
    uint16_t block_count;

    void set_control(uint32_t p_val);
    uint32_t get_control();

    uint32_t get_base();
    void set_base(uint32_t p_val);

    uint32_t block_control();
    void set_block_control(uint32_t p_val);

    bool active();

    Direction get_direction();

    Step get_step();

    Sync get_sync();

    // Set the channel status to "completed" state
    void done();

    // Return the DMA transfer size in bytes or None for Linked
    // list mode
    std::optional<uint32_t> transfer_size();

    Channel();
    ~Channel() = default;
};
