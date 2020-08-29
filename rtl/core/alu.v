module alu (
  input [2:0] op_i,
  input [31:0] a_i,
  input [31:0] b_i,
  input shift_type_i,
  output reg [31:0] out_o
);

  `include "control_signals.vh"

  always @(*) begin
    case (op_i)
      ALU_OP_ADD: out_o = a_i + b_i;
      ALU_OP_SHL: out_o = a_i << b_i[4:0];
      ALU_OP_CMP: out_o = $signed(a_i) < $signed(b_i) ? 32'b1 : 32'b0;
      ALU_OP_CMPU: out_o = a_i < b_i ? 32'b1 : 32'b0;
      ALU_OP_XOR: out_o = a_i ^ b_i;
      ALU_OP_SHR: out_o = shift_type_i ? a_i >>> b_i[4:0] : a_i >> b_i[4:0];
      ALU_OP_OR: out_o = a_i | b_i;
      ALU_OP_AND: out_o = a_i & b_i;
      default: out_o = 32'b0;
    endcase
  end

endmodule
