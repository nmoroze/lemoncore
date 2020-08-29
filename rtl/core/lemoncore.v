module lemoncore (
  input         clk_i,
  input         rst_i,

  output [31:0] instr_req_addr_o,
  output        instr_req_valid_o,
  input [31:0]  instr_res_data_i,
  input         instr_res_valid_i,
  input         instr_res_error_i,

  output [31:0] mem_read_req_addr_o,
  output        mem_read_req_valid_o,
  input [31:0]  mem_read_res_data_i,
  input         mem_read_res_valid_i,
  input         mem_read_res_error_i,

  output [31:0] mem_write_req_addr_o,
  output [31:0] mem_write_req_data_o,
  output [3:0]  mem_write_req_mask_o,
  output        mem_write_req_valid_o,
  input         mem_write_res_valid_i,
  input         mem_write_res_error_i,

  input         irq_timer_i,
  input         irq_external_i,
  input         irq_software_i
);

  `include "control_signals.vh"
  `include "csrs.vh"

  // Could make these changeable params, but we'd need to add some logic to make
  // sure they are honored by power-on-reset
  localparam BOOT_ADDRESS = 32'h0;
  localparam EXCEPTION_ADDRESS = 32'h0;

  /*
   * Interrupt numbers
   */
  localparam SOFTWARE_IRQ = 3;
  localparam TIMER_IRQ = 7;
  localparam EXTERNAL_IRQ = 11;

  /*
   * Control state machine boilerplate
   */
  localparam CTRL_STATE_FETCH = 0;
  localparam CTRL_STATE_DECODE = 1;
  localparam CTRL_STATE_EX = 2;
  localparam CTRL_STATE_MEM = 3;
  localparam CTRL_STATE_WB = 4;
  localparam CTRL_STATE_ERR = 3'b111;

  reg [2:0]   ctrl_state;
  reg [2:0]   ctrl_state_next;
  wire [2:0]  fetch_ctrl_state_next;
  wire [2:0]  decode_ctrl_state_next;
  wire [2:0]  ex_ctrl_state_next;
  reg [2:0]   mem_ctrl_state_next;
  wire [2:0]  wb_ctrl_state_next;

  always @(*) begin
    if (exception) begin
      ctrl_state_next = CTRL_STATE_FETCH;
    end else begin
      case (ctrl_state)
        CTRL_STATE_FETCH: ctrl_state_next = fetch_ctrl_state_next;
        CTRL_STATE_DECODE: ctrl_state_next = decode_ctrl_state_next;
        CTRL_STATE_EX: ctrl_state_next = ex_ctrl_state_next;
        CTRL_STATE_MEM: ctrl_state_next = mem_ctrl_state_next;
        CTRL_STATE_WB: ctrl_state_next = wb_ctrl_state_next;
        default: ctrl_state_next = CTRL_STATE_ERR;
      endcase
    end
  end

  always @(posedge clk_i) begin
    if (rst_i) begin
      ctrl_state <= CTRL_STATE_FETCH;
    end else begin
      ctrl_state <= ctrl_state_next;
    end
  end

  always @(posedge clk_i) begin
    if (rst_i) begin
      pc_q <= BOOT_ADDRESS;
    end else if (ctrl_state_next == CTRL_STATE_FETCH) begin
      // Latch next PC right before transitioning to fetch
      if (exception) begin
        if (mtvec_q[0] == 1'b1 && irq) begin
          // vectored mode
          pc_q <= {mtvec_q[31:2] + (mcause_q[29:0] << 2), 2'b00};
        end else begin
          // normal mode
          pc_q <= {mtvec_q[31:2], 2'b00};
        end
      end else if (mret) begin
        pc_q <= mepc_q;
      end else if (ctrl_state != CTRL_STATE_FETCH) begin
        // If we're blocked in fetch waiting for memory reply, don't continue to
        // step PC
        pc_q <= pc_d;
      end
    end
  end

  /*
   * Fetch stage logic
   */
  reg [31:0] pc_q /*verilator public*/;
  reg [31:0] pc_d;
  reg [31:0] instr_q;
  wire       misaligned_instr;
  wire       access_fault_instr;

  // Set instruction fetch outputs
  assign instr_req_addr_o = pc_q;
  // If we get an interrupt during fetch state, need valid to go low while we
  // remain in fetch state in order to indicate we'd like to fetch a new instr
  assign instr_req_valid_o = ~irq & ~misaligned_instr & (ctrl_state == CTRL_STATE_FETCH);

  assign misaligned_instr = pc_q[1:0] != 2'b00;
  assign access_fault_instr = instr_res_error_i;

  // hang on this state until we get a valid instruction fetch response
  assign fetch_ctrl_state_next = (instr_req_valid_o && instr_res_valid_i)
                                 ? CTRL_STATE_DECODE : CTRL_STATE_FETCH;

  always @(posedge clk_i) begin
    if (ctrl_state == CTRL_STATE_FETCH &&
                 fetch_ctrl_state_next == CTRL_STATE_DECODE) begin
      // latch current instruction on transition
      instr_q <= instr_res_data_i;
    end
  end

  /*
   * Decode stage logic
   */

  // Register indices
  wire [4:0] rs1, rs2, rd;

  // Reg/imm data flops
  wire [31:0] rd1_q, rd2_q;
  reg [31:0]  imm_q;
  wire [31:0] imm_d;

  // WB stage wires
  wire we;
  reg [31:0] wdata;

  // Decoder control signals
  wire [2:0] alu_op;
  wire a_src, b_src;
  wire negate_b;
  wire mem_w, reg_w;
  wire [2:0] ext_sel;
  reg [1:0] next_pc_q;
  wire [1:0] next_pc_d;
  wire [1:0] wb_src;
  wire shift_type;
  wire illegal_instr;
  wire illegal_instr_decode;
  wire nop;
  wire ecall;
  wire ebreak;
  wire mret;
  wire [1:0] csr;
  wire csr_use_imm;
  wire [11:0] csr_num;
  wire [4:0] csr_zimm;

  decoder decoder(
    .instr_i(instr_q),
    .rs1_o(rs1),
    .rs2_o(rs2),
    .rd_o(rd),
    .imm_o(imm_d),
    .alu_op_o(alu_op),
    .a_src_o(a_src),
    .b_src_o(b_src),
    .negate_b_o(negate_b),
    .mem_w_o(mem_w),
    .reg_w_o(reg_w),
    .ext_sel_o(ext_sel),
    .next_pc_o(next_pc_d),
    .wb_src_o(wb_src),
    .shift_type_o(shift_type),
    .illegal_instr_o(illegal_instr_decode),
    .nop_o(nop),
    .ecall_o(ecall),
    .ebreak_o(ebreak),
    .mret_o(mret),
    .csr_o(csr),
    .csr_imm_o(csr_use_imm),
    .csr_index_o(csr_num),
    .csr_zimm_o(csr_zimm)
  );

  assign illegal_instr = illegal_instr_decode | (illegal_csr_num & is_csr);

  regfile regfile(
    .clk_i(clk_i),
    .rs1_i(rs1),
    .rs2_i(rs2),
    .rd1_o(rd1_q),
    .rd2_o(rd2_q),
    .we_i(we),
    .ws_i(rd),
    .wd_i(wdata)
  );

  assign decode_ctrl_state_next = nop ? CTRL_STATE_FETCH :  CTRL_STATE_EX;

  always @(posedge clk_i) begin
    if (rst_i) begin
      imm_q <= 32'b0;
      next_pc_q <= NEXT_PC_ALU; // important for correct boot
    end else if (ctrl_state == CTRL_STATE_DECODE) begin
      imm_q <= imm_d;
      next_pc_q <= next_pc_d;
    end
  end

  /*
   * Execute stage logic
   */
  reg  [31:0] alu_result_q, store_data_q;
  wire [31:0] alu_result_d, store_data_d;

  wire [31:0] alu_a;
  wire [31:0] alu_b;

  assign alu_a = a_src == A_SRC_PC ? pc_q : rd1_q;
  assign alu_b = b_src == B_SRC_IMM ? imm_q : (negate_b ? -rd2_q : rd2_q);

  alu alu(
    .op_i(alu_op),
    .a_i(alu_a),
    .b_i(alu_b),
    .shift_type_i(shift_type),
    .out_o(alu_result_d)
  );

  // Also ext rs2 if needed for sub-word store
  ext store_ext(
    .in_i(rd2_q),
    .sel_i(ext_sel),
    .out_o(store_data_d)
  );

  assign ex_ctrl_state_next = mret ? CTRL_STATE_FETCH :
                              (mem_w || wb_src == WB_SRC_MEM) ?
                              CTRL_STATE_MEM : CTRL_STATE_WB;

  always @(posedge clk_i) begin
    if (rst_i) begin
      alu_result_q <= BOOT_ADDRESS; // important for correct boot
      store_data_q <= 32'b0;
    end else if (ctrl_state == CTRL_STATE_EX) begin
      alu_result_q <= alu_result_d;
      store_data_q <= store_data_d;
    end
  end

  /*
   * Memory Stage
   */
  reg  [31:0] mem_rdata_q;
  wire [31:0] mem_rdata_d;

  assign mem_read_req_addr_o = alu_result_q;
  assign mem_write_req_addr_o = alu_result_q;
  assign mem_write_req_data_o = store_data_q;
  assign mem_write_req_mask_o = (ext_sel == 3'b010) ? 4'b1111 : // sw
                                (ext_sel == 3'b001) ? 4'b0011 : // sh
                                (ext_sel == 3'b000) ? 4'b0001 : // sb
                                4'b0; // shouldn't happen/don't care

  wire read_req_outstanding;
  wire write_req_outstanding;
  assign read_req_outstanding = (wb_src == WB_SRC_MEM) && ctrl_state == CTRL_STATE_MEM;
  assign write_req_outstanding = mem_w && ctrl_state == CTRL_STATE_MEM;

  wire misaligned_load, misaligned_store;
  assign misaligned_load = ((ext_sel[1:0] == 2'b10) ? (mem_read_req_addr_o[1:0] != 2'b00) :  // lw
                            (ext_sel[1:0] == 2'b01) ? (mem_read_req_addr_o[0] != 1'b0) :     // lh[u]
                            1'b0) & read_req_outstanding;
  assign misaligned_store = ((ext_sel[1:0] == 2'b10) ? (mem_write_req_addr_o[1:0] != 2'b00) :  // sw
                             (ext_sel[1:0] == 2'b01) ? (mem_write_req_addr_o[0] != 1'b0) :     // sh[u]
                             1'b0) & write_req_outstanding;

  wire access_fault_load, access_fault_store;
  assign access_fault_load = mem_read_res_error_i & read_req_outstanding;
  assign access_fault_store = mem_write_res_error_i & write_req_outstanding;

  assign mem_read_req_valid_o = ~(misaligned_load | irq) & read_req_outstanding;
  assign mem_write_req_valid_o = ~(misaligned_store | irq) & write_req_outstanding;

  // Transition on appropriate memory response
  always @(*) begin
    if (read_req_outstanding && mem_read_res_valid_i) begin
      mem_ctrl_state_next = CTRL_STATE_WB;
    end else if (write_req_outstanding && mem_write_res_valid_i) begin
      mem_ctrl_state_next = CTRL_STATE_FETCH;
    end else begin
      mem_ctrl_state_next = CTRL_STATE_MEM;
    end
  end

  ext memext(
    .in_i(mem_read_res_data_i),
    .sel_i(ext_sel),
    .out_o(mem_rdata_d)
  );

  always @(posedge clk_i) begin
    if (rst_i) begin
      mem_rdata_q <= 32'b0;
    end else if (ctrl_state == CTRL_STATE_MEM && mem_read_res_valid_i) begin
      mem_rdata_q <= mem_rdata_d;
    end
  end

  /*
   * Writeback stage
   */

  // Regfile is instantiated with the decode stage logic
  // Just need to fill in 'we' and 'wdata' signals here

  // don't commit if we get an IRQ here
  assign we = (ctrl_state == CTRL_STATE_WB && reg_w && !irq);
  always @(*) begin
    case (wb_src)
      WB_SRC_PC:  wdata = pc_q + 32'd4;
      WB_SRC_MEM: wdata = mem_rdata_q;
      WB_SRC_ALU: wdata = alu_result_q;
      WB_SRC_CSR: wdata = csr_read_q;
      default: wdata = 32'b0;
    endcase
  end

  assign wb_ctrl_state_next = CTRL_STATE_FETCH;

  /*
   * Close the loop back to fetch stage
   */
  always @(*) begin
    case (next_pc_q)
      NEXT_PC_ALU: pc_d = {alu_result_q[31:1], 1'b0};
      NEXT_PC_INC: pc_d = pc_q + 32'd4;
      NEXT_PC_BR0: pc_d = (alu_result_q == 32'b0) ? pc_q + imm_q : pc_q + 32'd4;
      NEXT_PC_BR1: pc_d = (alu_result_q == 32'b0) ? pc_q + 32'd4 : pc_q + imm_q;
    endcase
  end

  /*
   * CSRs & exception handling
   */
  reg mstatus_mie /*verilator public*/, mstatus_mpie /*verilator public*/;
  reg [31:0] mepc_q, mepc_d /*verilator public*/;
  reg [31:0] mcause_q /*verilator public*/, mcause_d;
  reg [31:0] mtval_q /*verilator public*/, mtval_d;
  reg [31:0] mtvec_q, mtvec_d /*verilator public*/;
  wire       mip_external /*verilator public*/, mip_software /*verilator public*/, mip_timer /*verilator public*/;
  reg        mie_external /*verilator public*/, mie_software /*verilator public*/, mie_timer /*verilator public*/;
  reg [31:0] mscratch_q /*verilator public*/;

  assign mip_external = irq_external_i;
  assign mip_software = irq_software_i;
  assign mip_timer = irq_timer_i;

  wire irq = mstatus_mie & ((mie_external & irq_external_i) |
                            (mie_software & irq_software_i) |
                            (mie_timer    & irq_timer_i));
  wire is_csr;
  assign is_csr = csr != 2'b0;

  reg [31:0] csr_read_q, csr_read_d;
  reg        illegal_csr_num_read;
  always @(*) begin
    illegal_csr_num_read = 1'b0;
    csr_read_d = 32'd0; // don't care

    if ((csr_num >= CSR_NUM_MHPMCOUNTER3  && csr_num <= CSR_NUM_MHPMCOUNTER31) ||
        (csr_num >= CSR_NUM_MHPMCOUNTER3H && csr_num <= CSR_NUM_MHPMCOUNTER31H) ||
        (csr_num >= CSR_NUM_HPMCOUNTER3   && csr_num <= CSR_NUM_HPMCOUNTER31) ||
        (csr_num >= CSR_NUM_HPMCOUNTER3H  && csr_num <= CSR_NUM_HPMCOUNTER31H) ||
        (csr_num >= CSR_NUM_MHPMEVENT3    && csr_num <= CSR_NUM_MHPMEVENT31)) begin
      // all extra hardware counters hardwired to zero
      // we don't individually enumerate cases b/c there are a lot, and they're
      // sequential
      csr_read_d = 32'd0;
    end else begin
      // individually enumerate each of the remaining CSRs, and trigger illegal
      // instruction exception for an illegal number
      case (csr_num)
        // Machine information registers
        CSR_NUM_MVENDORID: csr_read_d = 32'd0;
        CSR_NUM_MARCHID:   csr_read_d = 32'd0;
        CSR_NUM_MIMPID:    csr_read_d = 32'd0;
        CSR_NUM_MHARTID:   csr_read_d = 32'd0;

        // Machine trap setup
        CSR_NUM_MSTATUS: csr_read_d = {25'b0, mstatus_mpie, 3'b0, mstatus_mie, 2'b0};
        CSR_NUM_MISA:    csr_read_d = 32'h4070; // RV32I only, TODO: show pieces
        CSR_NUM_MIE:     csr_read_d = {21'b0, mie_external, 3'b0, mie_timer, 3'b0, mie_software, 2'b0};
        CSR_NUM_MTVEC:   csr_read_d = mtvec_q;

        // Machine trap handling
        CSR_NUM_MSCRATCH: csr_read_d = mscratch_q;
        CSR_NUM_MEPC:     csr_read_d = mepc_q;
        CSR_NUM_MCAUSE:   csr_read_d = mcause_q;
        CSR_NUM_MTVAL:    csr_read_d = mtval_q;
        CSR_NUM_MIP:      csr_read_d = {21'b0, mip_external, 3'b0, mip_timer, 3'b0, mip_software, 2'b0};

        // Counters/timers
        CSR_NUM_CYCLE,
        CSR_NUM_MCYCLE:    csr_read_d = cycles_q[31:0];
        CSR_NUM_CYCLEH,
        CSR_NUM_MCYCLEH:   csr_read_d = cycles_q[63:32];
        CSR_NUM_INSTRET,
        CSR_NUM_MINSTRET:  csr_read_d = instret_q[31:0];
        CSR_NUM_INSTRETH,
        CSR_NUM_MINSTRETH: csr_read_d = instret_q[63:32];

        // TODO: should implement mcountinhibit?

        default: illegal_csr_num_read = 1'b1;
      endcase
    end
  end

  wire [31:0] csr_operand;
  assign csr_operand = csr_use_imm ? {27'b0, csr_zimm} : rd1_q;
  reg [31:0]   csr_update;
  always @(*) begin
    case (csr)
      CSR_RW: csr_update = csr_operand;
      CSR_RS: csr_update = csr_read_d | csr_operand;
      CSR_RC: csr_update = csr_read_d ^ csr_operand;
      default: csr_update = csr_read_d; // don't care
    endcase
  end

  always @(posedge clk_i) begin
    csr_read_q <= csr_read_d;
  end

  wire exception;
  always @(posedge clk_i) begin
    if (rst_i) begin
      mtvec_q <= EXCEPTION_ADDRESS;
      mie_external <= 1'b0;
      mie_software <= 1'b0;
      mie_timer <= 1'b0;
      mstatus_mie <= 1'b0;
    end
    if (exception) begin
      // Changes that occur automatically on exception
      mstatus_mpie <= mstatus_mie;
      mstatus_mie <= 1'b0;
      mcause_q <= mcause_d;
      mepc_q <= pc_q;
      mtval_q <= mtval_d;
    end else if (ctrl_state == CTRL_STATE_EX) begin
      if (is_csr && !no_csr_write) begin
        case (csr_num)
          // Fill in writable CSRs
          CSR_NUM_MSTATUS: begin
            mstatus_mie <= csr_update[3];
            mstatus_mpie <= csr_update[7];
          end
          CSR_NUM_MIE: begin
            mie_external <= csr_update[EXTERNAL_IRQ];
            mie_software <= csr_update[SOFTWARE_IRQ];
            mie_timer    <= csr_update[TIMER_IRQ];
          end
          CSR_NUM_MTVEC: mtvec_q <= csr_update;
          CSR_NUM_MSCRATCH: mscratch_q <= csr_update;
          CSR_NUM_MEPC: mepc_q <= csr_update;
          CSR_NUM_MCAUSE: mcause_q <= csr_update;
          CSR_NUM_MTVAL: mtval_q <= csr_update;
          default: begin
            // Empty block to prevent incomplete case lint warning
          end
        endcase
      end else if (mret) begin
        mstatus_mie <= mstatus_mpie;
      end
    end
  end

  // Performance counters
  reg [63:0] cycles_q, instret_q;
  always @(posedge clk_i) begin
    if (rst_i) begin
      cycles_q <= 64'b0;
    end else if (ctrl_state == CTRL_STATE_EX && is_csr && !no_csr_write &&
                (csr_num == CSR_NUM_MCYCLE || csr_num == CSR_NUM_MCYCLEH)) begin
      if (csr_num == CSR_NUM_MCYCLE) begin
        cycles_q[31:0] <= csr_update;
      end else begin
        cycles_q[63:32] <= csr_update;
      end
    end else begin
      cycles_q <= cycles_q + 64'b1;
    end
  end

  // if we're transitioning from a non-fetch state to the fetch state, and we're
  // not handling an exception, we've just retired an instruction
  wire instret;
  assign instret = (ctrl_state != CTRL_STATE_FETCH) &&
                    (ctrl_state_next == CTRL_STATE_FETCH) &&
                    !exception;

  always @(posedge clk_i) begin
    if (rst_i) begin
      instret_q <= 64'b0;
    end else if (is_csr && !no_csr_write &&
                (csr_num == CSR_NUM_MINSTRET || csr_num == CSR_NUM_MINSTRETH)) begin
      if (ctrl_state == CTRL_STATE_EX) begin
        // only perform update when we reach ex state, but should still bypass
        // the next condition
        if (csr_num == CSR_NUM_MINSTRET) begin
          instret_q[31:0] <= csr_update;
        end else begin
          instret_q[63:32] <= csr_update;
        end
      end
    end else if (instret) begin
      instret_q <= instret_q + 64'b1;
    end
  end

  reg illegal_csr_num_write;
  always @(*) begin
    if ((csr_num >= CSR_NUM_MHPMCOUNTER3  && csr_num <= CSR_NUM_MHPMCOUNTER31) ||
        (csr_num >= CSR_NUM_MHPMCOUNTER3H && csr_num <= CSR_NUM_MHPMCOUNTER31H) ||
        (csr_num >= CSR_NUM_HPMCOUNTER3   && csr_num <= CSR_NUM_HPMCOUNTER31) ||
        (csr_num >= CSR_NUM_HPMCOUNTER3H  && csr_num <= CSR_NUM_HPMCOUNTER31H) ||
        (csr_num >= CSR_NUM_MHPMEVENT3    && csr_num <= CSR_NUM_MHPMEVENT31)) begin
      // Extra performance counters are R/W
      illegal_csr_num_write = 1'b0;
    end else begin
      case (csr_num)
        // R/W CSRs
        CSR_NUM_MSTATUS,
        CSR_NUM_MISA,
        CSR_NUM_MIE,
        CSR_NUM_MTVEC,
        CSR_NUM_MSCRATCH,
        CSR_NUM_MEPC,
        CSR_NUM_MCAUSE,
        CSR_NUM_MTVAL,
        CSR_NUM_MIP,
        CSR_NUM_MCYCLE,
        CSR_NUM_MINSTRET,
        CSR_NUM_MCYCLEH,
        CSR_NUM_MINSTRETH: illegal_csr_num_write = 1'b0;
        // All remaining are RO
        default: illegal_csr_num_write = 1'b1;
      endcase
    end
  end

  // don't write CSR if we have a CSRRW with rd = x0, or CSRRWI with zimm = 5'b0
  wire no_csr_write;
  assign no_csr_write = (csr == CSR_RW) && (csr_use_imm ? (csr_zimm == 5'b0) :
                                                          (rs1 == 5'b0));
  wire illegal_csr_write;
  assign illegal_csr_write = illegal_csr_num_write && !no_csr_write;

  reg illegal_csr_read;
  always @(*) begin
    illegal_csr_read = 1'b0;
    if (illegal_csr_num_read) begin
      if (csr == CSR_RW && rd == 5'b0) begin
        illegal_csr_read = 1'b0;
      end else begin
        illegal_csr_read = 1'b1;
      end
    end
  end

  wire illegal_csr_num;
  assign illegal_csr_num = illegal_csr_read || illegal_csr_write;

  always @(*) begin
    mcause_d = mcause_q;
    if (irq_external_i) begin
      mcause_d[31] = 1'b1;
      mcause_d[30:0] = EXTERNAL_IRQ;
    end else if (irq_software_i) begin
      mcause_d[31] = 1'b1;
      mcause_d[30:0] = SOFTWARE_IRQ;
    end else if (irq_timer_i) begin
      mcause_d[31] = 1'b1;
      mcause_d[30:0] = TIMER_IRQ;
    end else if (access_fault_instr) begin
      mcause_d = 32'd1;
    end else if (illegal_instr) begin
      mcause_d = 32'd2;
    end else if (misaligned_instr) begin
      mcause_d = 32'd0;
    end else if (ecall) begin
      mcause_d = 32'd11;
    end else if (ebreak) begin
      mcause_d = 32'd3;
    end else if (misaligned_store) begin
      mcause_d = 32'd6;
    end else if (misaligned_load) begin
      mcause_d = 32'd4;
    end else if (access_fault_store) begin
      mcause_d = 32'd7;
    end else if (access_fault_load) begin
      mcause_d = 32'd5;
    end
  end

  always @(*) begin
    mtval_d = mtval_q;
    if (irq_external_i) begin
      mtval_d = 32'd0;
    end else if (irq_software_i) begin
      mtval_d = 32'd0;
    end else if (irq_timer_i) begin
      mtval_d = 32'd0;
    end else if (access_fault_instr) begin
      mtval_d = pc_q;
    end else if (illegal_instr) begin
      mtval_d = instr_q;
    end else if (misaligned_instr) begin
      mtval_d = pc_q;
    end else if (ecall) begin
      mtval_d = 32'd0;
    end else if (ebreak) begin
      mtval_d = 32'd0;
    end else if (misaligned_store) begin
      mtval_d = mem_write_req_addr_o;
    end else if (misaligned_load) begin
      mtval_d = mem_read_req_addr_o;
    end else if (access_fault_store) begin
      mtval_d = mem_write_req_addr_o;
    end else if (access_fault_load) begin
      mtval_d = mem_read_req_addr_o;
    end
  end

  assign exception = (misaligned_instr & ctrl_state == CTRL_STATE_FETCH) |
                     (access_fault_instr & ctrl_state == CTRL_STATE_FETCH) |
                     (illegal_instr & ctrl_state == CTRL_STATE_DECODE) |
                     (ecall & ctrl_state == CTRL_STATE_DECODE) |
                     (ebreak & ctrl_state == CTRL_STATE_DECODE) |
                     (misaligned_load & ctrl_state == CTRL_STATE_MEM) |
                     (misaligned_store & ctrl_state == CTRL_STATE_MEM) |
                     (access_fault_load & ctrl_state == CTRL_STATE_MEM) |
                     (access_fault_store & ctrl_state == CTRL_STATE_MEM) |
                     irq;

endmodule
