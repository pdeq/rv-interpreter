## How to use this interpreter

**Option 1**
`./interpreter` will create a GUI
- Write your instructions in the left box
- "Run" button will execute entire assembly
- "Step" button will execute next line, beginning at start
- "Reset" will reset internal PC, memory, and registers

**Option 2**
`/.interpreter program.s` will print the values of the registers after running `program.s`

**Instructions supported:**

ADD, SUB, AND, OR, XOR, SLT, SLTU, SLL, SRA, SRL, ADDW, SLLW, SRLW, SUBW, SRAW, ADDI, ANDI, ORI, XORI, SLTI, ADDIW, SLLI, SLLIW, SRLI, SRLIW, SRAI, SRAIW, SLTIU, LD, LW, LH, LB, SD, SW, SH, SB, LUI, AUIPC, BEQ, BNE, BLT, BLTU, BGE, BGEU, BGT, BGTU, BLE, BLEU, BEQZ, BNEZ, BLEZ, BGEZ, BLTZ, BGTZ, MV, NOT, NEG, NEGW, SEXT.W, SEQZ, SNEZ, SLTZ, SGTZ, JAL, JALR, J, JR, RET