module ext (
  input [31:0] in_i,
  input [2:0] sel_i,
  output reg [31:0] out_o
);

  localparam EXT_SEL_B = 3'b000;
  localparam EXT_SEL_H = 3'b001;
  localparam EXT_SEL_W = 3'b010;
  localparam EXT_SEL_BU = 3'b100;
  localparam EXT_SEL_HU = 3'b101;

  always @(*) begin
    case (sel_i)
      EXT_SEL_B: out_o = {{24{in_i[7]}}, in_i[7:0]};
      EXT_SEL_H: out_o = {{16{in_i[15]}}, in_i[15:0]};
      EXT_SEL_W: out_o = in_i;
      EXT_SEL_BU: out_o = {24'b0, in_i[7:0]};
      EXT_SEL_HU: out_o = {16'b0, in_i[15:0]};
      default: out_o = 32'b0;
    endcase
  end

endmodule
