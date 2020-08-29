localparam A_SRC_PC  /* verilator public */ = 0;
localparam A_SRC_RS1 /* verilator public */ = 1;
localparam B_SRC_IMM /* verilator public */ = 0;
localparam B_SRC_RS2 /* verilator public */ = 1;

localparam ALU_OP_ADD  /* verilator public */ = 3'b000;
localparam ALU_OP_SHL  /* verilator public */ = 3'b001;
localparam ALU_OP_CMP  /* verilator public */ = 3'b010;
localparam ALU_OP_CMPU /* verilator public */ = 3'b011;
localparam ALU_OP_XOR  /* verilator public */ = 3'b100;
localparam ALU_OP_SHR  /* verilator public */ = 3'b101;
localparam ALU_OP_OR   /* verilator public */ = 3'b110;
localparam ALU_OP_AND  /* verilator public */ = 3'b111;

localparam NEXT_PC_ALU /* verilator public */ = 0;
localparam NEXT_PC_INC /* verilator public */ = 1;
localparam NEXT_PC_BR0 /* verilator public */ = 2; // zero ? pc + imm : pc + 4
localparam NEXT_PC_BR1 /* verilator public */ = 3; // zero ? pc + 4   : pc + imm

localparam WB_SRC_PC  /* verilator public */ = 0;
localparam WB_SRC_MEM /* verilator public */ = 1;
localparam WB_SRC_ALU /* verilator public */ = 2;
localparam WB_SRC_CSR /* verilator public */ = 3;
