module gpio (
  input             clk_i,
  input             rst_i,

  input             write_req_valid_i,
  input [31:0]      write_req_addr_i,

  // Lint complains since we only use the least significant bit of the written
  // data, but we're keeping the port at its full width so the interfaces are
  // consistent (and we may use the rest in the future).
  /* verilator lint_off UNUSED */
  input [31:0]      write_req_data_i,
  input [3:0]       write_req_mask_i,
  /* verilator lint_on UNUSED */

  output reg        write_res_valid_o,
  output reg        write_res_error_o,

  input             read_req_valid_i,
  input [31:0]      read_req_addr_i,
  output reg [31:0] read_res_data_o,
  output reg        read_res_valid_o,
  output reg        read_res_error_o,

  input [2:0]       buttons_i,
  output reg [4:0]  user_leds_o,
  output reg        done_led_o,
  output reg        exception_led_o
  );

`include "memmap.vh"

  // Writes
  always @(posedge clk_i) begin
    if (rst_i) begin
      user_leds_o <= 5'b0;
      done_led_o <= 1'b0;
      exception_led_o <= 1'b0;
      write_res_valid_o <= 1'b0;
    end else if (write_req_valid_i) begin
      if (write_req_addr_i == GPIO_BASE) begin
        if (write_req_mask_i[0])
          user_leds_o <= write_req_data_i[4:0];
        write_res_valid_o <= 1'b1;
        write_res_error_o <= 1'b0;
      end else if (write_req_addr_i == GPIO_BASE + 32'h4) begin
        if (write_req_mask_i[0]) begin
          done_led_o <= write_req_data_i[0];
          exception_led_o <= write_req_data_i[1];
        end
        write_res_valid_o <= 1'b1;
        write_res_error_o <= 1'b0;
      end else begin
        write_res_valid_o <= 1'b0;
        write_res_error_o <= 1'b1;
      end
    end else begin
      write_res_valid_o <= 1'b0;
      write_res_error_o <= 1'b0;
    end
  end

  // Reads
  always @(*) begin
    read_res_data_o = 32'b0;
    read_res_valid_o = 1'b0;
    read_res_error_o = 1'b0;

    if (read_req_valid_i) begin
      if (read_req_addr_i == GPIO_BASE) begin
        read_res_data_o = {27'b0, user_leds_o};
        read_res_valid_o = 1'b1;
      end else if (read_req_addr_i == GPIO_BASE + 32'h4) begin
        read_res_data_o = {30'b0, exception_led_o, done_led_o};
        read_res_valid_o = 1'b1;
      end else if (read_req_addr_i == GPIO_BASE + 32'h8) begin
        read_res_data_o = {29'b0, buttons_i};
        read_res_valid_o = 1'b1;
      end else begin
        read_res_error_o = 1'b1;
      end
    end
  end


endmodule
