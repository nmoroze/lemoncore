module regfile (
  input clk_i,
  input [4:0] rs1_i,
  input [4:0]  rs2_i,
  output reg [31:0] rd1_o,
  output reg [31:0] rd2_o,
  input we_i,
  input [4:0] ws_i,
  input [31:0] wd_i
);

  reg [31:0] regs[0:31] /*verilator public*/;

  always @(posedge clk_i) begin
    if (we_i && ws_i != 5'b0) begin
        regs[ws_i] <= wd_i;
    end
  end

  always @(posedge clk_i) begin
    rd1_o <= regs[rs1_i];
    rd2_o <= regs[rs2_i];
  end

  integer i;
  initial begin
    for (i = 0; i < 32; i = i + 1) begin
      regs[i] = 32'b0;
    end
  end
endmodule
