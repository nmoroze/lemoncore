localparam [31:0] ROM_BASE  = 32'h0;
localparam [31:0] ROM_SIZE  = 32'h1000;

localparam [31:0] RAM_BASE  = ROM_BASE + ROM_SIZE; // 0x1000
localparam [31:0] RAM_SIZE  = 32'h2000;

localparam [31:0] GPIO_BASE = RAM_BASE + RAM_SIZE; // 0x3000
localparam [31:0] GPIO_SIZE = 32'hC;

localparam [31:0] TIMER_BASE = GPIO_BASE + GPIO_SIZE; // 0x300C
localparam [31:0] TIMER_SIZE = 32'h4;
