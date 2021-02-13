module rvfi_wrapper (
  input clock,
  input reset,
  `RVFI_OUTPUTS
  );

  (* keep *) wire [31:0] instr_req_addr;
  (* keep *) wire instr_req_valid;
  (* keep *) `rvformal_rand_reg [31:0] instr_res_data;
  (* keep *) `rvformal_rand_reg instr_res_valid;
  (* keep *) `rvformal_rand_reg instr_res_error;

  (* keep *) wire [31:0] mem_read_req_addr;
  (* keep *) wire mem_read_req_valid;
  (* keep *) `rvformal_rand_reg [31:0] mem_read_res_data;
  (* keep *) `rvformal_rand_reg mem_read_res_valid;
  (* keep *) `rvformal_rand_reg mem_read_res_error;

  (* keep *) wire [31:0] mem_write_req_addr;
  (* keep *) wire [31:0] mem_write_req_data;
  (* keep *) wire [3:0] mem_write_req_mask;
  (* keep *) wire mem_write_req_valid;
  (* keep *) `rvformal_rand_reg  mem_write_res_valid;
  (* keep *) `rvformal_rand_reg mem_write_res_error;

  (* keep *) `rvformal_rand_reg irq_timer;
  (* keep *) `rvformal_rand_reg irq_external;
  (* keep *) `rvformal_rand_reg irq_software;

  lemoncore uut (
    .clk_i(clock),
    .rst_i(reset),

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
    .mem_write_req_mask_o(mem_write_req_mask),
    .mem_write_req_valid_o(mem_write_req_valid),
    .mem_write_res_valid_i(mem_write_res_valid),
    .mem_write_res_error_i(mem_write_res_error),

    .irq_timer_i(irq_timer),
    .irq_external_i(irq_external),
    .irq_software_i(irq_software),

    `RVFI_CONN
    );

  endmodule
