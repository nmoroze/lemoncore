module lemonsoc (
  input  CLK,

  output LED1,
  output LED2,
  output LED3,
  output LED4,
  output LED5,
  output LEDG_N,
  output LEDR_N,

  input  BTN1,
  input  BTN2,
  input  BTN3,
  input  BTN_N
);

`include "memmap.vh"

  wire  rst_n;
  wire  rst;
  wire [2:0] btn;

  sync sync_rst(
    .clk(CLK),
    .in(BTN_N),
    .out(rst_n)
  );
  assign rst = ~rst_n;

  sync sync_btn1(
    .clk(CLK),
    .in(BTN1),
    .out(btn[0])
  );
  sync sync_btn2(
    .clk(CLK),
    .in(BTN2),
    .out(btn[1])
  );
  sync sync_btn3(
    .clk(CLK),
    .in(BTN3),
    .out(btn[2])
  );

  wire        instr_req_valid;
  wire [31:0] instr_req_addr;
  reg         instr_res_valid;
  reg [31:0]  instr_res_data;
  reg         instr_res_error;

  wire        mem_read_req_valid;
  wire [31:0] mem_read_req_addr;
  reg [31:0]  mem_read_res_data;
  reg         mem_read_res_valid;
  reg         mem_read_res_error;

  wire        mem_write_req_valid;
  wire [31:0] mem_write_req_addr;
  wire [31:0] mem_write_req_data;
  wire [3:0]  mem_write_req_mask;
  reg         mem_write_res_valid;
  reg         mem_write_res_error;

  reg         imem_read_req_valid;
  reg [31:0]  imem_read_req_addr;
  reg [31:0]  imem_read_res_data;
  reg         imem_read_res_valid;

  reg         dmem_read_req_valid;
  reg [31:0]  dmem_read_req_addr;
  reg [31:0]  dmem_read_res_data;
  reg         dmem_read_res_valid;

  reg         ram_read_req_valid;
  reg [31:0]  ram_read_req_addr;
  wire [31:0] ram_read_res_data;
  wire        ram_read_res_valid;

  reg         ram_write_req_valid;
  reg [31:0]  ram_write_req_addr;
  reg [31:0]  ram_write_req_data;
  reg [3:0]   ram_write_req_mask;
  wire        ram_write_res_valid;

  reg         gpio_read_req_valid;
  reg [31:0]  gpio_read_req_addr;
  wire [31:0] gpio_read_res_data;
  wire        gpio_read_res_valid;
  wire        gpio_read_res_error;

  reg         gpio_write_req_valid;
  reg [31:0]  gpio_write_req_addr;
  reg [31:0]  gpio_write_req_data;
  reg [3:0]   gpio_write_req_mask;
  wire        gpio_write_res_valid;
  wire        gpio_write_res_error;

  reg         timer_write_req_valid;

  always @(*) begin
    gpio_write_req_valid =1'b0;
    gpio_write_req_addr = 32'b0;
    gpio_write_req_data = 32'b0;
    gpio_write_req_mask = 4'b0;

    ram_write_req_valid =1'b0;
    ram_write_req_addr = 32'b0;
    ram_write_req_data = 32'b0;
    ram_write_req_mask = 4'b0;

    timer_write_req_valid = 1'b0;

    if (mem_write_req_addr >= GPIO_BASE && mem_write_req_addr < GPIO_BASE + GPIO_SIZE) begin
      // GPIO
      gpio_write_req_addr = mem_write_req_addr;
      gpio_write_req_data = mem_write_req_data;
      gpio_write_req_valid = mem_write_req_valid;
      gpio_write_req_mask = mem_write_req_mask;
      mem_write_res_valid = gpio_write_res_valid;
      mem_write_res_error = gpio_write_res_error;
    end else if (mem_write_req_addr >= RAM_BASE && mem_write_req_addr < RAM_BASE + RAM_SIZE) begin
      // RAM
      ram_write_req_addr = mem_write_req_addr;
      ram_write_req_data = mem_write_req_data;
      ram_write_req_valid = mem_write_req_valid;
      ram_write_req_mask = mem_write_req_mask;
      mem_write_res_valid = ram_write_res_valid;
      mem_write_res_error = 1'b0;
    end else if (mem_write_req_addr >= TIMER_BASE && mem_write_req_addr < TIMER_BASE + TIMER_SIZE) begin
      timer_write_req_valid = mem_write_req_valid;
      mem_write_res_valid = 1'b1;
      mem_write_res_error = 1'b0;
    end else begin
      mem_write_res_valid = 1'b0;
      mem_write_res_error = 1'b1;
    end
  end

  always @(*) begin
    imem_read_req_valid = 1'b0;
    imem_read_req_addr = 32'b0;

    // Lint complains because ROM_BASE is 0, and so the first comparison is meaningless.
    // want to keep as-is just in case we did happen to change the ROM base, however, so
    // we turn off the relevant lint error.

    /* verilator lint_off UNSIGNED */
    if (instr_req_addr >= ROM_BASE && instr_req_addr < ROM_BASE + ROM_SIZE) begin
    /*verilator lint_on UNSIGNED */
      imem_read_req_addr = instr_req_addr;
      imem_read_req_valid = instr_req_valid;
      instr_res_data = imem_read_res_data;
      instr_res_valid = imem_read_res_valid;
      instr_res_error = 1'b0;
    end else begin
      instr_res_data = 32'b0;
      instr_res_valid = 1'b0;
      instr_res_error = 1'b1;
    end
  end

  always @(*) begin
    gpio_read_req_valid =1'b0;
    gpio_read_req_addr = 32'b0;

    dmem_read_req_valid =1'b0;
    dmem_read_req_addr = 32'b0;

    if (mem_read_req_addr >= GPIO_BASE && mem_read_req_addr < GPIO_BASE + GPIO_SIZE) begin
      // GPIO
      gpio_read_req_addr = mem_read_req_addr;
      gpio_read_req_valid = mem_read_req_valid;
      mem_read_res_data = gpio_read_res_data;
      mem_read_res_valid = gpio_read_res_valid;
      mem_read_res_error = gpio_read_res_error;
    end else if (mem_read_req_addr >= RAM_BASE && mem_read_req_addr < RAM_BASE + RAM_SIZE) begin
      // RAM
      dmem_read_req_addr = mem_read_req_addr;
      dmem_read_req_valid = mem_read_req_valid;
      mem_read_res_data = dmem_read_res_data;
      mem_read_res_valid = dmem_read_res_valid;
      mem_read_res_error = 1'b0;
    end else begin
      mem_read_res_data = 32'b0;
      mem_read_res_valid = 1'b0;
      mem_read_res_error = 1'b1;
    end
  end

  // Arbitrate access to RAM between instruction and data ports.
  // Instruction requests always get priority
  always @(*) begin
    ram_read_req_valid = 1'b0;
    ram_read_req_addr = 32'b0;

    imem_read_res_data = 32'b0;
    imem_read_res_valid = 1'b0;

    dmem_read_res_data = 32'b0;
    dmem_read_res_valid = 1'b0;

    if (instr_req_valid) begin
      ram_read_req_valid = imem_read_req_valid;
      ram_read_req_addr = imem_read_req_addr;
      imem_read_res_data = ram_read_res_data;
      imem_read_res_valid = ram_read_res_valid;
    end else if (mem_read_req_valid) begin
      ram_read_req_valid = dmem_read_req_valid;
      ram_read_req_addr = dmem_read_req_addr;
      dmem_read_res_data = ram_read_res_data;
      dmem_read_res_valid = ram_read_res_valid;
    end
  end

  ram ram (
    .clk_i(CLK),

    .read_req_valid_i(ram_read_req_valid),
    .read_req_addr_i(ram_read_req_addr),
    .read_res_valid_o(ram_read_res_valid),
    .read_res_data_o(ram_read_res_data),

    .write_req_valid_i(ram_write_req_valid),
    .write_req_addr_i(ram_write_req_addr),
    .write_req_data_i(ram_write_req_data),
    .write_req_mask_i(ram_write_req_mask),
    .write_res_valid_o(ram_write_res_valid)
  );

  wire [4:0] user_leds;
  wire       done_led;
  wire       exception_led;
  gpio gpio (
    .clk_i(CLK),
    .rst_i(rst),

    .write_req_valid_i(gpio_write_req_valid),
    .write_req_addr_i(gpio_write_req_addr),
    .write_req_data_i(gpio_write_req_data),
    .write_req_mask_i(gpio_write_req_mask),
    .write_res_valid_o(gpio_write_res_valid),
    .write_res_error_o(gpio_write_res_error),

    .read_req_valid_i(gpio_read_req_valid),
    .read_req_addr_i(gpio_read_req_addr),
    .read_res_data_o(gpio_read_res_data),
    .read_res_valid_o(gpio_read_res_valid),
    .read_res_error_o(gpio_read_res_error),

    .buttons_i(btn),
    .user_leds_o(user_leds),
    .done_led_o(done_led),
    .exception_led_o(exception_led)
  );

  wire irq_timer;
  timer timer (
    .clk_i(CLK),
    .rst_i(rst),

    .write_req_i(timer_write_req_valid),

    .timer_irq_o(irq_timer)
  );

  assign LED1 = user_leds[0];
  assign LED2 = user_leds[1];
  assign LED3 = user_leds[2];
  assign LED4 = user_leds[3];
  assign LED5 = user_leds[4];
  assign LEDG_N = ~done_led;
  assign LEDR_N = ~exception_led;

  lemoncore lemon (
    .clk_i(CLK),
    .rst_i(rst),
    .instr_req_addr_o(instr_req_addr),
    .instr_req_valid_o(instr_req_valid),
    .instr_res_data_i(instr_res_data),
    .instr_res_valid_i(instr_res_valid),
    .instr_res_error_i(instr_res_error),

    .mem_read_req_addr_o(mem_read_req_addr),
    .mem_read_req_valid_o(mem_read_req_valid),
    .mem_read_res_data_i(mem_read_res_data),
    .mem_read_res_valid_i(mem_read_res_valid),
    .mem_read_res_error_i(mem_read_res_error),

    .mem_write_req_addr_o(mem_write_req_addr),
    .mem_write_req_data_o(mem_write_req_data),
    .mem_write_req_valid_o(mem_write_req_valid),
    .mem_write_req_mask_o(mem_write_req_mask),
    .mem_write_res_valid_i(mem_write_res_valid),
    .mem_write_res_error_i(mem_write_res_error),

    .irq_external_i(1'b0),
    .irq_timer_i(irq_timer),
    .irq_software_i(1'b0)
  );

endmodule
