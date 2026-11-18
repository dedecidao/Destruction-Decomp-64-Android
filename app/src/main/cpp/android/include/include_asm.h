/* splat writes include/include_asm.h during `make extract`. The port builds no
   assembly, so the macros are empty here (the game sources the port compiles
   have no INCLUDE_ASM left). */
#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H
#define INCLUDE_ASM(FOLDER, NAME)
#define INCLUDE_RODATA(FOLDER, NAME)
#endif
