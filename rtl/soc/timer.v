module timer (
  input  clk_i,
  input  rst_i,

  input write_req_i,

  output timer_irq_o
);

`ifndef SIM
  localparam [13:0] TICKS_PER_MS = 14'd12_000; // 12 mhz clock
`else
  localparam [13:0] TICKS_PER_MS = 14'd100;
`endif

  reg [13:0] counter;

  always @(posedge clk_i) begin
    if (rst_i || write_req_i) begin
      counter <= 14'b0;
    end else if (counter != TICKS_PER_MS) begin
      counter <= counter + 14'b1;
    end
  end

  assign timer_irq_o = counter == TICKS_PER_MS;

endmodule
