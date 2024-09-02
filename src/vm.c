#include "vm.h"
#include "opcode.h"
#include "error.h"

void saw_vm_init(vm_t *vm, FILE *fp)
{
    vm->running = 1;
    vm->fp = fp;
    saw_stack_init(&vm->stack);
}

void saw_insn_push_byte(vm_t *vm)
{
    saw_byte_t byte;

    if (fread(&byte, sizeof(saw_byte_t), 1, vm->fp) != sizeof(saw_byte_t))
        SAW_ERROR("Failed to read a byte!");

    saw_stack_push(&vm->stack, (saw_stack_element_t)byte);
}

void saw_insn_dup(vm_t *vm)
{
    saw_long_t byte = saw_stack_pop(&vm->stack);

    saw_stack_push(&vm->stack, byte);
    saw_stack_push(&vm->stack, byte);
}

void saw_insn_swap(vm_t *vm)
{
    saw_long_t b = saw_stack_pop(&vm->stack);
    saw_long_t a = saw_stack_pop(&vm->stack);

    saw_stack_push(&vm->stack, b);
    saw_stack_push(&vm->stack, a);
}

void saw_insn_add(vm_t *vm)
{
    saw_long_t b = saw_stack_pop(&vm->stack);
    saw_long_t a = saw_stack_pop(&vm->stack);

    saw_stack_push(&vm->stack, a + b);
}

void saw_insn_sub(vm_t *vm)
{
    saw_long_t b = saw_stack_pop(&vm->stack);
    saw_long_t a = saw_stack_pop(&vm->stack);

    saw_stack_push(&vm->stack, a - b);
}

void saw_insn_multiply(vm_t *vm)
{
    saw_long_t b = saw_stack_pop(&vm->stack);
    saw_long_t a = saw_stack_pop(&vm->stack);

    saw_stack_push(&vm->stack, a * b);
}

void saw_insn_stackdump(vm_t *vm)
{
    puts("#======== Stack Dump ========#");
    printf("#\tStack Top:\t%2d   #\n#\t\t\t     #\n", vm->stack.top);

    for (int i = 0; i < vm->stack.top + 1; i++)
    {
        printf("#\tStack[%d] =\t%2d   #\n", i, vm->stack.arr[i]);
    }

    puts("#============================#");
}

void saw_insn_branch(vm_t *vm)
{
    saw_long_t offset = saw_stack_pop(&vm->stack);
    fseek(vm->fp, offset, 0);
}

void saw_insn_push_ip(vm_t *vm)
{
    long ip = ftell(vm->fp) - 1; // Gets the current ip
    saw_stack_push(&vm->stack, ip); // We want to push the offset of the next instruction.
}

void saw_vm_step(vm_t *vm)
{
    opcode_t opcode;
    long offset = ftell(vm->fp);

    if (fread(&opcode, sizeof(opcode_t), 1, vm->fp) != sizeof(opcode))
        SAW_ERROR("Failed to read next opcode! This binary may be missing a %s instruction!", OPCODE_NAMES[OP_HALT]);

    if (opcode < OPCODE_MIN || opcode > OPCODE_MAX)
        SAW_ERROR("Invalid opcode '0x%X'!", opcode);

    printf("[saw-vm]: 0x%X %s stack=%d\n", offset, OPCODE_NAMES[opcode], vm->stack.top + 1);

    switch (opcode)
    {
    case OP_NOP:
        break;
    case OP_PUSH_BYTE:
        saw_insn_push_byte(vm);
        break;
    case OP_POP:
        saw_stack_pop(&vm->stack);
        break;
    case OP_DUP:
        saw_insn_dup(vm);
        break;
    case OP_SWAP:
        saw_insn_swap(vm);
        break;
    case OP_ADD:
        saw_insn_add(vm);
        break;
    case OP_SUB:
        saw_insn_sub(vm);
        break;
    case OP_MULTIPLY:
        saw_insn_multiply(vm);
        break;
    case OP_STACKDUMP:
        saw_insn_stackdump(vm);
        break;
    case OP_BRANCH:
        saw_insn_branch(vm);
        break;
    case OP_PUSH_IP:
        saw_insn_push_ip(vm);
        break;
    case OP_HALT:
        vm->running = 0;
        break;
    default:
        SAW_ERROR("Unimplemented opcode '%s'!", OPCODE_NAMES[opcode]);
        break;
    }
}

void saw_vm_free(vm_t *vm)
{
    saw_stack_free(&vm->stack);
}