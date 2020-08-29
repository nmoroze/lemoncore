module ram (
  input         clk_i,
  input         read_req_valid_i,
  input [31:0]  read_req_addr_i,
  output reg    read_res_valid_o,
  output [31:0] read_res_data_o,

  input         write_req_valid_i,
  input [31:0]  write_req_addr_i,
  input [31:0]  write_req_data_i,
  input [3:0]   write_req_mask_i,
  output reg    write_res_valid_o
);

`include "memmap.vh"

  parameter SIZE = (ROM_SIZE + RAM_SIZE) / 4; // words

  reg [31:0] mem[SIZE];

  wire [31:0] wdata;
  assign wdata = write_req_data_i << (8 * (write_req_addr_i & 32'b11));

  wire [3:0] wmask;
  assign wmask = write_req_mask_i << (write_req_addr_i & 32'b11);

  reg [31:0] rdata_word;
  assign read_res_data_o =  rdata_word >> (8 * (read_req_addr_i & 32'b11));

  always @(posedge clk_i) begin
    if (read_req_valid_i) begin
      rdata_word <= mem[read_req_addr_i >> 2];
      read_res_valid_o <= 1'b1;
    end else begin
      read_res_valid_o <= 1'b0;
    end

    if (write_req_valid_i) begin
      if (wmask[3])
        mem[write_req_addr_i >> 2][31:24] <= wdata[31:24];
      if (wmask[2])
        mem[write_req_addr_i >> 2][23:16] <= wdata[23:16];
      if (wmask[1])
        mem[write_req_addr_i >> 2][15:8] <= wdata[15:8];
      if (wmask[0])
        mem[write_req_addr_i >> 2][7:0] <= wdata[7:0];

      write_res_valid_o <= 1'b1;
    end else begin
      write_res_valid_o <= 1'b0;
    end
  end

`ifdef SIM
  // Task for loading ROM from simulation
  export "DPI-C" task verilator_load_mem;
  export "DPI-C" task verilator_set_mem_entry;

  task verilator_load_mem;
    input string file;
    $readmemh(file, mem);
  endtask

  task verilator_set_mem_entry;
    input int addr;
    input int data;
    mem[addr >> 2] = data;
  endtask
`else
  // For FPGA we generate a bistream that loads the ROM with a randomly
  // generated pattern. We can then use `icebram` to quickly generate new
  // bitstreams that load our firmware.
  initial begin
    $readmemh("random.mem", mem);
  end
`endif

endmodule
