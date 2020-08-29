module decoder (
  input [31:0]      instr_i,
  output [4:0]      rs1_o,
  output [4:0]      rs2_o,
  output [4:0]      rd_o,
  output reg [31:0] imm_o,
  output reg [2:0]  alu_op_o,
  output            a_src_o,
  output            b_src_o,
  output            negate_b_o,
  output            mem_w_o,
  output            reg_w_o,
  output [2:0]      ext_sel_o,
  output reg [1:0]  next_pc_o,
  output reg [1:0]  wb_src_o,
  output            shift_type_o,
  output            illegal_instr_o,
  output            nop_o,
  output reg        ecall_o,
  output reg        ebreak_o,
  output            mret_o,
  output [1:0]      csr_o,
  output            csr_imm_o,
  output [11:0]     csr_index_o,
  output [4:0]      csr_zimm_o
);

  `include "control_signals.vh"

  // TODO: The decoder does not have logic to check for all illegal instruction
  // conditions - not sure if this is necessary to be in specs.
  // It's missing at least the following checks:
  // - all of funct7 not invalid for immediate shift/R-type arithmetic
  // - only accept fully valid patterns for fence, ecall, and ebreak
  // - don't allow invalid funct3 bits for CSRs

  // Assign commonly used, named slices of instruction
  wire [6:0] opcode;
  wire [2:0] funct3;
  wire [6:0] funct7;
  assign opcode = instr_i[6:0];
  assign funct3 = instr_i[14:12];
  assign funct7 = instr_i[31:25];

  // Some illegal instruction checks
  wire illegal_jalr;
  wire illegal_load;
  wire illegal_store;
  reg illegal_alu;

  assign illegal_jalr = (is_jalr) && (funct3 != 3'b000);
  assign illegal_load = (is_load) && (
                        (ext_sel_o == 3'b011) ||
                        (ext_sel_o == 3'b110) ||
                        (ext_sel_o == 3'b111));
  assign illegal_store = (is_store) && (
                         (ext_sel_o != 3'b000) &&
                         (ext_sel_o != 3'b001) &&
                         (ext_sel_o != 3'b010));
  always @(*) begin
    illegal_alu = 1'b0;
    if (is_i_arith) begin
      if (funct3 == ALU_OP_SHR) begin
        illegal_alu = (funct7 != 7'b0100000) &&
                      (funct7 != 7'b0000000);
      end else if (funct3 == ALU_OP_SHL) begin
        illegal_alu = (funct7 != 7'b0000000);
      end
    end else if (is_r_arith) begin
      if (funct3 == ALU_OP_ADD || instr_i[14:12] == ALU_OP_SHR) begin
        illegal_alu = (funct7 != 7'b0100000) &&
                      (funct7 != 7'b0000000);
      end else begin
        illegal_alu = (funct7 != 7'b0000000);
      end
    end
  end

  assign illegal_instr_o = (illegal_instr_type ||
                           illegal_branch ||
                           illegal_jalr ||
                           illegal_load ||
                           illegal_store ||
                           illegal_alu) && !(is_wfi || is_mret);

  // Instruction type logic
  reg illegal_instr_type;
  reg is_lui;
  reg is_auipc;
  reg is_jal;
  reg is_jalr;
  reg is_branch;
  reg is_load;
  reg is_store;
  reg is_i_arith;
  reg is_r_arith;
  reg is_fence;
  reg is_sys;
  always @(*) begin
    illegal_instr_type = 1'b0;
    is_lui = 1'b0;
    is_auipc = 1'b0;
    is_jal = 1'b0;
    is_jalr = 1'b0;
    is_branch = 1'b0;
    is_load = 1'b0;
    is_store = 1'b0;
    is_i_arith = 1'b0;
    is_r_arith = 1'b0;
    is_fence = 1'b0;
    is_sys = 1'b0;
    case (opcode)
      7'b0110111: is_lui = 1'b1;
      7'b0010111: is_auipc = 1'b1;
      7'b1101111: is_jal = 1'b1;
      7'b1100111: is_jalr = 1'b1;
      7'b0000011: is_load = 1'b1;
      7'b0010011: is_i_arith = 1'b1;
      7'b1100011: is_branch = 1'b1;
      7'b0100011: is_store = 1'b1;
      7'b0110011: is_r_arith = 1'b1;
      7'b0001111: is_fence = 1'b1;
      7'b1110011: is_sys = 1'b1;
      default: begin
        illegal_instr_type = 1'b1;
      end
    endcase
  end

  // Immediate logic
  always @(*) begin
    if (is_lui || is_auipc) begin // Type U
      imm_o = {instr_i[31:12], 12'b0};
    end else if (is_jal) begin   // Type J
      imm_o = {{12{instr_i[31]}}, instr_i[19:12], instr_i[20], instr_i[30:21], 1'b0};
    end else if (is_branch) begin // Type B
      imm_o = {{20{instr_i[31]}}, instr_i[7], instr_i[30:25], instr_i[11:8], 1'b0};
    end else if (is_i_arith || is_jalr || is_load) begin  // Type I
      imm_o = {{20{instr_i[31]}}, instr_i[31:20]};
    end else if (is_store) begin // Type S
      imm_o = {{20{instr_i[31]}}, funct7, instr_i[11:7]};
    end else begin
      imm_o = 32'b0;  // don't care about imm for all other instr types
    end
  end

  // ALU OP
  always @(*) begin
    if (is_r_arith || is_i_arith) begin
      // we only use these bits as alu op for R-type instructions and one
      // specific variant of I-type
      alu_op_o = funct3;
    end else if (is_lui || is_auipc) begin
      alu_op_o = ALU_OP_ADD;
    end else if (is_jal) begin
      alu_op_o = ALU_OP_AND;
    end else if (is_jalr) begin
      alu_op_o = ALU_OP_ADD;
    end else if (is_branch) begin
      case(funct3)
        3'b000,
        3'b001: alu_op_o = ALU_OP_XOR;
        3'b100,
        3'b101: alu_op_o = ALU_OP_CMP;
        3'b110,
        3'b111: alu_op_o = ALU_OP_CMPU;
        default: begin
          // don't care/illegal instruction (caught in NextPC logic)
          alu_op_o = ALU_OP_XOR;
        end
      endcase
    end else if (is_store || is_load) begin
      alu_op_o = ALU_OP_ADD;
    end else begin
      // only thing left are system instructions, which is a don't care for
      // ALU
      alu_op_o = ALU_OP_AND; // (don't care)
    end
  end

  // Asrc and Bsrc
  // only PC if we have auipc instruction
  assign a_src_o = is_auipc ? A_SRC_PC : A_SRC_RS1;
  assign b_src_o = (is_branch || is_r_arith) ? B_SRC_RS2 : B_SRC_IMM;

  // MemW and RegW
  assign mem_w_o = is_store;
  assign reg_w_o = !(is_store || is_branch);

  // Extension
  assign ext_sel_o = funct3;

  // NextPC
  reg illegal_branch;
  always @(*) begin
    illegal_branch = 1'b0;
    if (is_branch) begin
      case (funct3)
        3'b000,
        3'b101,
        3'b111: next_pc_o = NEXT_PC_BR0;
        3'b001,
        3'b100,
        3'b110: next_pc_o = NEXT_PC_BR1;
        default: begin
          illegal_branch = 1'b1;
          next_pc_o = 0; // (don't care)
        end
      endcase
    end else if (is_jal) begin
      next_pc_o = NEXT_PC_BR0;
    end else if (is_jalr) begin
      next_pc_o = NEXT_PC_ALU;
    end else begin
      next_pc_o = NEXT_PC_INC;
    end
  end

  // WB src
  always @(*) begin
    if (is_load) begin
      wb_src_o = WB_SRC_MEM;
    end else if (is_jal || is_jalr) begin
      wb_src_o = WB_SRC_PC;
    end else if (csr_o != 2'b0) begin
      wb_src_o = WB_SRC_CSR;
    end else begin
      wb_src_o = WB_SRC_ALU;
    end
  end

  // B negate (only for sub instruction)
  assign negate_b_o = (is_r_arith &&
                       funct3 == ALU_OP_ADD &&
                       instr_i[30] == 1'b1) ? 1'b1 : 1'b0;

  // Shift type (arithmetic or logical)
  assign shift_type_o = instr_i[30];

  // Register indices
  assign rs1_o = (is_jal || is_lui || is_auipc) ?
                     5'b0 : instr_i[19:15];
  assign rs2_o = instr_i[24:20];
  assign rd_o = instr_i[11:7];

  // ecall and breakpoint
  always @(*) begin
    ecall_o = 1'b0;
    ebreak_o = 1'b0;
    if (is_sys && funct3 == 3'b0) begin
      if (instr_i[31:20] == 12'b0) begin
        ecall_o = 1'b1;
      end else if (instr_i[31:20] == 12'b1) begin
        ebreak_o = 1'b1;
      end
    end
  end

  // CSR signals
  assign csr_o = (is_sys) ? instr_i[13:12] : 2'b0;
  assign csr_imm_o = instr_i[14];
  assign csr_index_o = instr_i[31:20];
  assign csr_zimm_o = instr_i[19:15];

  // Other signal instructions
  wire is_wfi;
  wire is_mret;

  assign is_wfi  = instr_i == 32'b0001000_00101_00000_000_00000_1110011;
  assign is_mret = instr_i == 32'b0011000_00010_00000_000_00000_1110011;

  assign nop_o = is_fence | is_wfi; // fence and wfi are nops
  assign mret_o = is_mret;

endmodule
