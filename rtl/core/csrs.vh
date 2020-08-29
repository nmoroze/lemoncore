/*
 * CSR numbers
 */
// Machine information registers
localparam CSR_NUM_MVENDORID = 12'hF11;
localparam CSR_NUM_MARCHID   = 12'hF12;
localparam CSR_NUM_MIMPID    = 12'hF13;
localparam CSR_NUM_MHARTID   = 12'hF14;

// Machine trap setup
localparam CSR_NUM_MSTATUS = 12'h300;
localparam CSR_NUM_MISA    = 12'h301;
localparam CSR_NUM_MIE     = 12'h304;
localparam CSR_NUM_MTVEC   = 12'h305;

// Machine trap handling
localparam CSR_NUM_MSCRATCH = 12'h340;
localparam CSR_NUM_MEPC     = 12'h341;
localparam CSR_NUM_MCAUSE   = 12'h342;
localparam CSR_NUM_MTVAL    = 12'h343;
localparam CSR_NUM_MIP      = 12'h344;

// Counters/timers
// M mode
localparam CSR_NUM_MCYCLE        = 12'hB00;
localparam CSR_NUM_MINSTRET      = 12'hB02;
localparam CSR_NUM_MHPMCOUNTER3  = 12'hB03;
localparam CSR_NUM_MHPMCOUNTER31 = 12'hB1F;

localparam CSR_NUM_MCYCLEH        = 12'hB80;
localparam CSR_NUM_MINSTRETH      = 12'hB82;
localparam CSR_NUM_MHPMCOUNTER3H  = 12'hB83;
localparam CSR_NUM_MHPMCOUNTER31H = 12'hB9F;

localparam CSR_NUM_MHPMEVENT3  = 12'h323;
localparam CSR_NUM_MHPMEVENT31 = 12'h323;

// Shadows
localparam CSR_NUM_CYCLE    = 12'hC00;
localparam CSR_NUM_INSTRET  = 12'hC02;
localparam CSR_NUM_HPMCOUNTER3  = 12'hC03;
localparam CSR_NUM_HPMCOUNTER31 = 12'hC1F;

localparam CSR_NUM_CYCLEH   = 12'hC80;
localparam CSR_NUM_INSTRETH = 12'hC82;
localparam CSR_NUM_HPMCOUNTER3H  = 12'hC83;
localparam CSR_NUM_HPMCOUNTER31H = 12'hC9F;

/*
 * CSR ops
 */

localparam CSR_RW = 2'b01;
localparam CSR_RS = 2'b10;
localparam CSR_RC = 2'b11;
