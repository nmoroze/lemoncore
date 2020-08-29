module sync (
  input clk,
  input in,
  output out
);

  reg [1:0] out_q;
  always @(posedge clk) begin
    out_q <= {out_q[0], in};
  end

  assign out = out_q[1];

endmodule
