#ifndef _RVR_VM_H_

/*
   RvnicRaven - bytecode vm

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

/*
   To create implementation (the function definitions) add
      #define RVR_VM_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _RVR_VM_H_


typedef struct
{
   uint8_t *code;
   void *mem_base;

   int32_t code_size;
   uint32_t pc_entry;

   //Registers
   uint8_t *pc;
   int32_t regs[32];
}RvR_vm;

typedef intptr_t (*RvR_vm_func_call) (RvR_vm *vm, intptr_t *args);

void RvR_vm_create(RvR_vm *vm, RvR_rw *code);
void RvR_vm_free(RvR_vm *vm);
void RvR_vm_disassemble(RvR_vm *vm);
void RvR_vm_run(RvR_vm *vm, uint32_t instr);
void RvR_vm_stack_free(); //Call after all vms have been destroyed

#endif

#ifdef RVR_VM_IMPLEMENTATION
#ifndef RVR_VM_IMPLEMENTATION_ONCE
#define RVR_VM_IMPLEMENTATION_ONCE

#ifndef RVR_VOM_COMPUTED_GOTO
#define RVR_VM_COMPUTED_GOTO 0
#endif

#ifndef RVR_VM_STACK_SIZE
#define RVR_VM_STACK_SIZE (1 << 20)
#endif

#ifndef RvR_vm_unreachable
#define RvR_vm_unreachable __builtin_unreachable
#endif

#if !RVR_VM_COMPUTED_GOTO

#define case_OP_INVALID  default
#define case_OP_LB case RVR_VM_OP_LB
#define case_OP_LH case RVR_VM_OP_LH
#define case_OP_LW case RVR_VM_OP_LW
#define case_OP_LBU case RVR_VM_OP_LBU
#define case_OP_LHU case RVR_VM_OP_LHU
#define case_OP_ADDI case RVR_VM_OP_ADDI
#define case_OP_SLLI case RVR_VM_OP_SLLI
#define case_OP_SLTI case RVR_VM_OP_SLTI
#define case_OP_SLTIU case RVR_VM_OP_SLTIU
#define case_OP_XORI case RVR_VM_OP_XORI
#define case_OP_SRLI case RVR_VM_OP_SRLI
#define case_OP_SRAI case RVR_VM_OP_SRAI
#define case_OP_ORI case RVR_VM_OP_ORI
#define case_OP_ANDI case RVR_VM_OP_ANDI
#define case_OP_SB case RVR_VM_OP_SB
#define case_OP_SH case RVR_VM_OP_SH
#define case_OP_SW case RVR_VM_OP_SW
#define case_OP_BEQ case RVR_VM_OP_BEQ
#define case_OP_BNE case RVR_VM_OP_BNE
#define case_OP_BLT case RVR_VM_OP_BLT
#define case_OP_BGE case RVR_VM_OP_BGE
#define case_OP_BLTU case RVR_VM_OP_BLTU
#define case_OP_BGEU case RVR_VM_OP_BGEU
#define case_OP_JALR case RVR_VM_OP_JALR
#define case_OP_SYSCALL case RVR_VM_OP_SYSCALL
#define case_OP_ADD case RVR_VM_OP_ADD
#define case_OP_SUB case RVR_VM_OP_SUB
#define case_OP_SLL case RVR_VM_OP_SLL
#define case_OP_SLT case RVR_VM_OP_SLT
#define case_OP_SLTU case RVR_VM_OP_SLTU
#define case_OP_XOR case RVR_VM_OP_XOR
#define case_OP_SRL case RVR_VM_OP_SRL
#define case_OP_SRA case RVR_VM_OP_SRA
#define case_OP_OR case RVR_VM_OP_OR
#define case_OP_AND case RVR_VM_OP_AND
#define case_OP_MUL case RVR_VM_OP_MUL
#define case_OP_MULH case RVR_VM_OP_MULH
#define case_OP_MULHSU case RVR_VM_OP_MULHSU
#define case_OP_MULHU case RVR_VM_OP_MULHU
#define case_OP_DIV case RVR_VM_OP_DIV
#define case_OP_DIVU case RVR_VM_OP_DIVU
#define case_OP_REM case RVR_VM_OP_REM
#define case_OP_REMU case RVR_VM_OP_REMU
#define case_OP_AUIPC case RVR_VM_OP_AUIPC
#define case_OP_LUI case RVR_VM_OP_LUI
#define case_OP_JAL case RVR_VM_OP_JAL

#endif

typedef enum
{
   RVR_VM_OP_LB = 0,
   RVR_VM_OP_LH = 1,
   RVR_VM_OP_LW = 2,
   RVR_VM_OP_LBU = 3,
   RVR_VM_OP_LHU = 4,
   RVR_VM_OP_ADDI = 5,
   RVR_VM_OP_SLLI = 6,
   RVR_VM_OP_SLTI = 7,
   RVR_VM_OP_SLTIU = 8,
   RVR_VM_OP_XORI = 9,
   RVR_VM_OP_SRLI = 10,
   RVR_VM_OP_SRAI = 11,
   RVR_VM_OP_ORI = 12,
   RVR_VM_OP_ANDI = 13,
   RVR_VM_OP_SB = 14,
   RVR_VM_OP_SH = 15,
   RVR_VM_OP_SW = 16,
   RVR_VM_OP_BEQ = 17,
   RVR_VM_OP_BNE = 18,
   RVR_VM_OP_BLT = 19,
   RVR_VM_OP_BGE = 20,
   RVR_VM_OP_BLTU = 21,
   RVR_VM_OP_BGEU = 22,
   RVR_VM_OP_JALR = 23,
   RVR_VM_OP_SYSCALL = 24,
   RVR_VM_OP_ADD = 25,
   RVR_VM_OP_SUB = 26,
   RVR_VM_OP_SLL = 27,
   RVR_VM_OP_SLT = 28,
   RVR_VM_OP_SLTU = 29,
   RVR_VM_OP_XOR = 30,
   RVR_VM_OP_SRL = 31,
   RVR_VM_OP_SRA = 32,
   RVR_VM_OP_OR = 33,
   RVR_VM_OP_AND = 34,
   RVR_VM_OP_MUL = 35,
   RVR_VM_OP_MULH = 36,
   RVR_VM_OP_MULHSU = 37,
   RVR_VM_OP_MULHU = 38,
   RVR_VM_OP_DIV = 39,
   RVR_VM_OP_DIVU = 40,
   RVR_VM_OP_REM = 41,
   RVR_VM_OP_REMU = 42,
   RVR_VM_OP_AUIPC = 43,
   RVR_VM_OP_LUI = 44,
   RVR_VM_OP_JAL = 45,
}rvr_vm_opcode;

static int rvr_vm_syscall_term = 0;
static void *rvr_vm_stack = NULL;

static uint32_t rvr_vm_syscall(RvR_vm *vm, uint32_t code);
static void rvr_vm_disassemble_instruction(uint32_t op);

void RvR_vm_create(RvR_vm *vm, RvR_rw *code)
{
   if(vm==NULL)
      return;

   if(rvr_vm_stack==NULL)
   {
      rvr_vm_stack = RvR_malloc(RVR_VM_STACK_SIZE);
      memset(rvr_vm_stack, 0, RVR_VM_STACK_SIZE);
   }

   memset(vm, 0, sizeof(*vm));

   vm->pc_entry = RvR_rw_read_u32(code);
   size_t size = RvR_rw_read_u32(code);
   uint32_t entry_count = RvR_rw_read_u32(code);

   vm->code_size = size;
   vm->code = RvR_malloc(size);

   for(int i = 0; i<entry_count; i++)
   {
      uint32_t offset = RvR_rw_read_u32(code);
      uint32_t addr = RvR_rw_read_u32(code);
      uint32_t filesz = RvR_rw_read_u32(code);
      uint32_t memsz = RvR_rw_read_u32(code);

      size_t pos = RvR_rw_tell(code);
      memset(vm->code + addr, 0, memsz);
      RvR_rw_seek(code, offset, SEEK_SET);
      RvR_rw_read(code, vm->code + addr, 1, filesz);
      RvR_rw_seek(code, pos, SEEK_SET);
   }

   vm->mem_base = vm->code;
}

void RvR_vm_free(RvR_vm *vm)
{
   if(vm==NULL)
      return;

   RvR_free(vm->code);
}

void RvR_vm_stack_free()
{
   if(rvr_vm_stack!=NULL)
      RvR_free(rvr_vm_stack);
}

void RvR_vm_disassemble(RvR_vm *vm)
{
   if(vm==NULL)
      return;

   for(int i = 0; i<vm->code_size / 4; i++)
   {
      RvR_log("%8d|", i * 4);

      int32_t op = ((uint32_t *)vm->code)[i + 1024];
      rvr_vm_disassemble_instruction(op);
   }
}

void RvR_vm_run(RvR_vm *vm, uint32_t instr)
{
   if(vm==NULL)
      return;

   vm->pc = (uint8_t *)vm->code + instr;
   vm->regs[2] = ((intptr_t)rvr_vm_stack - (intptr_t)vm->mem_base) + RVR_VM_STACK_SIZE;
   uint32_t op;
   int32_t rs1;
   int32_t rs2;
   int32_t rd;
   int32_t imm;

#if RVR_VM_COMPUTED_GOTO

   void *dispatch_table[64] = 
   {
      &&case_OP_LB,
      &&case_OP_LH,
      &&case_OP_LW,
      &&case_OP_LBU,
      &&case_OP_LHU,
      &&case_OP_ADDI,
      &&case_OP_SLLI,
      &&case_OP_SLTI,
      &&case_OP_SLTIU,
      &&case_OP_XORI,
      &&case_OP_SRLI,
      &&case_OP_SRAI,
      &&case_OP_ORI,
      &&case_OP_ANDI,
      &&case_OP_SB,
      &&case_OP_SH,
      &&case_OP_SW,
      &&case_OP_BEQ,
      &&case_OP_BNE,
      &&case_OP_BLT,
      &&case_OP_BGE,
      &&case_OP_BLTU,
      &&case_OP_BGEU,
      &&case_OP_JALR,
      &&case_OP_SYSCALL,
      &&case_OP_ADD,
      &&case_OP_SUB,
      &&case_OP_SLL,
      &&case_OP_SLT,
      &&case_OP_SLTU,
      &&case_OP_XOR,
      &&case_OP_SRL,
      &&case_OP_SRA,
      &&case_OP_OR,
      &&case_OP_AND,
      &&case_OP_MUL,
      &&case_OP_MULH,
      &&case_OP_MULHSU,
      &&case_OP_MULHU,
      &&case_OP_DIV,
      &&case_OP_DIVU,
      &&case_OP_REM,
      &&case_OP_REMU,
      &&case_OP_AUIPC,
      &&case_OP_LUI,
      &&case_OP_JAL,
   }

#define DISPATCH() vm->pc += 4; vm->regs[0] = 0; op = *((uint32_t *)vm->pc); goto *dispatch_table[op & 63]
#define DISPATCH_BRANCH() vm->regs[0] = 0; op = *((uint32_t *)vm->pc); goto *dispatch_table[op & 63]

   DISPATCH_BRANCH();

#else
#define DISPATCH() vm->pc += 4; goto next
#define DISPATCH_BRANCH() goto next
#endif

   for(;;)
   {
#if !RVR_VM_COMPUTED_GOTO
      next:
      vm->regs[0] = 0;
      op = *((uint32_t *)vm->pc);
#endif

      switch(op & 63)
      {
case_OP_LB:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = op >> 16;
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = *(((int8_t *)vm->mem_base) + imm + vm->regs[rs1]);

      DISPATCH();
case_OP_LH:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = op >> 16;
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = *((int16_t *)(((uint8_t *)vm->mem_base) + imm + vm->regs[rs1]));

      DISPATCH();
case_OP_LW:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = op >> 16;
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = *((int32_t *)(((uint8_t *)vm->mem_base) + imm + vm->regs[rs1]));

      DISPATCH();
case_OP_LBU:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = op >> 16;
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = *(((uint8_t *)vm->mem_base) + imm + vm->regs[rs1]);

      DISPATCH();
case_OP_LHU:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = op >> 16;
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = *((uint16_t *)(((uint8_t *)vm->mem_base) + imm + vm->regs[rs1]));

      DISPATCH();
case_OP_ADDI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = vm->regs[rs1] + imm;

      DISPATCH();
case_OP_SLLI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = (uint32_t)vm->regs[rs1] << imm;

      DISPATCH();
case_OP_SLTI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = vm->regs[rs1]<imm;

      DISPATCH();
case_OP_SLTIU:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = (uint32_t)vm->regs[rs1] < (uint32_t)imm;

      DISPATCH();
case_OP_XORI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = vm->regs[rs1] ^ imm;

      DISPATCH();
case_OP_SRLI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = (uint32_t)vm->regs[rs1] >> imm;

      DISPATCH();
case_OP_SRAI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = vm->regs[rs1] >> imm;

      DISPATCH();
case_OP_ORI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = vm->regs[rs1] | imm;

      DISPATCH();
case_OP_ANDI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      vm->regs[rd] = vm->regs[rs1] & imm;

      DISPATCH();
case_OP_SB:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      *(((uint8_t *)vm->mem_base) + imm + vm->regs[rs1]) = (uint8_t)vm->regs[rs2];

      DISPATCH();
case_OP_SH:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      *((uint16_t *)(((uint8_t *)vm->mem_base) + imm + vm->regs[rs1])) = (uint16_t)vm->regs[rs2];

      DISPATCH();
case_OP_SW:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      *((uint32_t *)(((uint8_t *)vm->mem_base) + imm + vm->regs[rs1])) = (uint32_t)vm->regs[rs2];

      DISPATCH();
case_OP_BEQ:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      if(vm->regs[rs1]==vm->regs[rs2])
      {
         vm->pc += imm;
         DISPATCH_BRANCH();
      }

      DISPATCH();
case_OP_BNE:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      if(vm->regs[rs1]!=vm->regs[rs2])
      {
         vm->pc += imm;
         DISPATCH_BRANCH();
      }

      DISPATCH();
case_OP_BLT:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      if(vm->regs[rs1]<vm->regs[rs2])
      {
         vm->pc += imm;
         DISPATCH_BRANCH();
      }

      DISPATCH();
case_OP_BGE:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      if(vm->regs[rs1]>=vm->regs[rs2])
      {
         vm->pc += imm;
         DISPATCH_BRANCH();
      }

      DISPATCH();
case_OP_BLTU:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      if((uint32_t)vm->regs[rs1]<(uint32_t)vm->regs[rs2])
      {
         vm->pc += imm;
         DISPATCH_BRANCH();
      }

      DISPATCH();
case_OP_BGEU:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      if((uint32_t)vm->regs[rs1]>=(uint32_t)vm->regs[rs2])
      {
         vm->pc += imm;
         DISPATCH_BRANCH();
      }

      DISPATCH();
case_OP_JALR:
      {
         rs1 = (op >> 6) & 31;
         rd = (op >> 11) & 31;
         imm = (op >> 16);
         imm = (imm << 16) >> 16; //sign extend
         uint8_t *pc = vm->pc + 4;
         vm->pc = vm->code + ((vm->regs[rs1] + imm) & -2);
         vm->regs[rd] = (intptr_t)pc - (intptr_t)vm->mem_base;
      }

      DISPATCH_BRANCH();
case_OP_SYSCALL:
      //I format
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      rvr_vm_syscall_term = 0;

      switch(imm)
      {
      case 0:
         vm->regs[10] = rvr_vm_syscall(vm, vm->regs[17]);
         if(rvr_vm_syscall_term)
            return;
         break;
      case 1:
         return;
      }

      DISPATCH();
case_OP_ADD:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] + vm->regs[rs2];

      DISPATCH();
case_OP_SUB:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] - vm->regs[rs2];

      DISPATCH();
case_OP_SLL:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] << (vm->regs[rs2] & 31);

      DISPATCH();
case_OP_SLT:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] < vm->regs[rs2];

      DISPATCH();
case_OP_SLTU:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = (uint32_t)vm->regs[rs1] < (uint32_t)vm->regs[rs2];

      DISPATCH();
case_OP_XOR:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] ^ vm->regs[rs2];

      DISPATCH();
case_OP_SRL:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = (uint32_t)vm->regs[rs1] >> (vm->regs[rs2] & 31);

      DISPATCH();
case_OP_SRA:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] >> (vm->regs[rs2] & 31);

      DISPATCH();
case_OP_OR:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] | vm->regs[rs2];

      DISPATCH();
case_OP_AND:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] & vm->regs[rs2];

      DISPATCH();
case_OP_MUL:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] * vm->regs[rs2];

      DISPATCH();
case_OP_MULH:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = ((int64_t)vm->regs[rs1] * (int64_t)vm->regs[rs2]) >> 32;

      DISPATCH();
case_OP_MULHSU:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = ((int64_t)vm->regs[rs1] * (uint64_t)vm->regs[rs2]) >> 32;

      DISPATCH();
case_OP_MULHU:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = ((uint64_t)vm->regs[rs1] * (uint64_t)vm->regs[rs2]) >> 32;

      DISPATCH();
case_OP_DIV:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] / vm->regs[rs2];

      DISPATCH();
case_OP_DIVU:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = (uint32_t)vm->regs[rs1] / (uint32_t)vm->regs[rs2];

      DISPATCH();
case_OP_REM:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = vm->regs[rs1] % vm->regs[rs2];

      DISPATCH();
case_OP_REMU:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      rd = (op >> 16) & 31;
      vm->regs[rd] = (uint32_t)vm->regs[rs1] % (uint32_t)vm->regs[rs2];

      DISPATCH();
case_OP_AUIPC:
      rd = (op >> 6) & 31;
      imm = op & 4294963200;
      vm->regs[rd] = (intptr_t)vm->pc - (intptr_t)vm->mem_base;
      vm->regs[rd] += imm;

      DISPATCH();
case_OP_LUI:
      rd = (op >> 6) & 31;
      imm = op & 4294963200;
      vm->regs[rd] = imm;

      DISPATCH();
case_OP_JAL:
      rd = (op >> 6) & 31;
      imm = op >> 11;
      imm = (imm << 11) >> 11; //sign extend
      vm->regs[rd] = ((intptr_t)vm->pc - (intptr_t)vm->mem_base) + 4;
      vm->pc += imm;

      DISPATCH_BRANCH();
      default: //The code is faster if this is here. WTF?
         printf("msiing instruction %d\n", op & 63);
      }
   }
}

static uint32_t rvr_vm_syscall(RvR_vm *vm, uint32_t code)
{
   switch(code)
   {
   case 0: //exit
      rvr_vm_syscall_term = 1;
      break;
   case 1: //memchr
   {
      void *res = memchr((uint8_t *)vm->mem_base + vm->regs[10], vm->regs[11], vm->regs[12]);
      return res==NULL?0:(intptr_t)res - (intptr_t)vm->mem_base;
   }
   case 2: //memcmp
      return memcmp((uint8_t *)vm->mem_base + vm->regs[10], (uint8_t *)vm->mem_base + vm->regs[11], vm->regs[12]);
   case 3: //memcpy
      return (intptr_t)memcpy((uint8_t *)vm->mem_base + vm->regs[10], (uint8_t *)vm->mem_base + vm->regs[11], vm->regs[12]) - (intptr_t)vm->mem_base;
   case 4: //memmove
      return (intptr_t)memmove((uint8_t *)vm->mem_base + vm->regs[10], (uint8_t *)vm->mem_base + vm->regs[11], vm->regs[12]) - (intptr_t)vm->mem_base;
   case 5: //memset
      return (intptr_t)memset((uint8_t *)vm->mem_base + vm->regs[10], vm->regs[11], vm->regs[12]) - (intptr_t)vm->mem_base;
   case 6: //strcat
      return (intptr_t)strcat((char *)((uint8_t *)vm->mem_base + vm->regs[10]), (char *)((uint8_t *)vm->mem_base + vm->regs[11])) - (intptr_t)vm->mem_base;
   case 7: //strchr
   {
      void *res = strchr((char *)((uint8_t *)vm->mem_base + vm->regs[10]), vm->regs[11]);
      return res==NULL?0:(intptr_t)res - (intptr_t)vm->mem_base;
   }
   case 8: //strcmp
      return strcmp((char *)((uint8_t *)vm->mem_base + vm->regs[10]), (char *)((uint8_t *)vm->mem_base + vm->regs[11]));
   case 9: //strcpy
      return (intptr_t)strcpy((char *)((uint8_t *)vm->mem_base + vm->regs[10]), (char *)((uint8_t *)vm->mem_base + vm->regs[11])) - (intptr_t)vm->mem_base;
   case 10: //strcspn
      return strcspn((char *)((uint8_t *)vm->mem_base + vm->regs[10]), (char *)((uint8_t *)vm->mem_base + vm->regs[11]));
   case 11: //strlen
      return strlen((char *)((uint8_t *)vm->mem_base + vm->regs[10]));
   case 12: //strncat
      return (intptr_t)strncat((char *)((uint8_t *)vm->mem_base + vm->regs[10]), (char *)((uint8_t *)vm->mem_base + vm->regs[11]), vm->regs[12]) - (intptr_t)vm->mem_base;
   case 13: //strncmp
      return strncmp((char *)((uint8_t *)vm->mem_base + vm->regs[10]), (char *)((uint8_t *)vm->mem_base + vm->regs[11]), vm->regs[12]);
   case 14: //strncpy
      return (intptr_t)strncmp((char *)((uint8_t *)vm->mem_base + vm->regs[10]), (char *)((uint8_t *)vm->mem_base + vm->regs[11]), vm->regs[12]) - (intptr_t)vm->mem_base;
   case 15: //strpbrk
   {
      void *res = strpbrk((char *)((uint8_t *)vm->mem_base + vm->regs[10]), (char *)((uint8_t *)vm->mem_base + vm->regs[11]));
      return res==NULL?0:(intptr_t)res - (intptr_t)vm->mem_base;
   }
   case 16: //strrchr
   {
      void *res = strrchr((char *)((uint8_t *)vm->mem_base + vm->regs[10]), vm->regs[11]);
      return res==NULL?0:(intptr_t)res - (intptr_t)vm->mem_base;
   }
   case 17: //strspn
      return strspn((char *)((uint8_t *)vm->mem_base + vm->regs[10]), (char *)((uint8_t *)vm->mem_base + vm->regs[11]));
   case 18: //abort
      rvr_vm_syscall_term = 1;
      break;
   case 19: //atoi
      return atoi((char *)((uint8_t *)vm->mem_base + vm->regs[10]));
   case 20: //free
      RvR_free((uint8_t *)vm->mem_base + vm->regs[10]);
      break;
   case 21: //malloc
   {
      void *res = RvR_malloc(vm->regs[10]);
      return res==NULL?0:(intptr_t)res - (intptr_t)vm->mem_base;
   }
   case 22: //rand
      return rand();
   case 23: //realloc
   {
      void *res = RvR_realloc((uint8_t *)vm->mem_base + vm->regs[10], vm->regs[11]);
      return res==NULL?0:(intptr_t)res - (intptr_t)vm->mem_base;
   }
   case 24: //puts
      return puts((char *)((uint8_t *)vm->mem_base + vm->regs[10]));
   case 25: //putchar
      return putchar(vm->regs[10]);
   }
   return 0;
}

static void rvr_vm_disassemble_instruction(uint32_t op)
{
   //TODO: add the remaining instructions
   RvR_log("%8x|", op);
   static const char *reg_names[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

   int32_t rs1;
   int32_t rs2;
   int32_t rd;
   int32_t imm;

   switch(op & 63)
   {
   case RVR_VM_OP_LB:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = op >> 16;
      imm = (imm << 16) >> 16; //sign extend
      RvR_log("lb %s,%d(%s)\n", reg_names[rd], imm, reg_names[rs1]);
      break;
   case RVR_VM_OP_LW:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = op >> 16;
      imm = (imm << 16) >> 16; //sign extend
      RvR_log("lw %s,%d(%s)\n", reg_names[rd], imm, reg_names[rs1]);
      break;
   case RVR_VM_OP_LBU:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = op >> 16;
      imm = (imm << 16) >> 16; //sign extend
      RvR_log("lbu %s,%d(%s)\n", reg_names[rd], imm, reg_names[rs1]);
      break;
   case RVR_VM_OP_ADDI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      RvR_log("addi %s,%s,%d\n", reg_names[rd], reg_names[rs1], imm);
      break;
   case RVR_VM_OP_ANDI:
      rs1 = (op >> 6) & 31;
      rd = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      RvR_log("andi %s,%s,%d\n", reg_names[rd], reg_names[rs1], imm);
      break;
   case RVR_VM_OP_SB:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      RvR_log("sb %s,%d(%s)\n", reg_names[rs2], imm, reg_names[rs1]);
      break;
   case RVR_VM_OP_SW:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      RvR_log("sw %s,%d(%s)\n", reg_names[rs2], imm, reg_names[rs1]);
      break;
   case RVR_VM_OP_BEQ:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      RvR_log("beq %s,%s,%d\n", reg_names[rs1], reg_names[rs2], imm);
      break;
   case RVR_VM_OP_BNE:
      rs1 = (op >> 6) & 31;
      rs2 = (op >> 11) & 31;
      imm = (op >> 16);
      imm = (imm << 16) >> 16; //sign extend
      RvR_log("bne %s,%s,%d\n", reg_names[rs1], reg_names[rs2], imm);
      break;
   case RVR_VM_OP_AUIPC:
      rd = (op >> 6) & 31;
      imm = op & 4294963200;
      RvR_log("auipc %s,%d\n", reg_names[rd], imm >> 12);
      break;
   case RVR_VM_OP_LUI:
      rd = (op >> 6) & 31;
      imm = op & 4294963200;
      RvR_log("lui %s,%d\n", reg_names[rd], imm >> 12);
      break;
   case RVR_VM_OP_JAL:
      rd = (op >> 6) & 31;
      imm = op >> 11;
      imm = (imm << 11) >> 11; //sign extend
      RvR_log("jal %s,%d\n", reg_names[rd], imm);
      break;
   default:
      RvR_log("%d\n", op & 63);
   }
}

#endif
#endif
