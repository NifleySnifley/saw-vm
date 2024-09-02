import csv
from argparse import ArgumentParser

parser = ArgumentParser()

parser.add_argument(
    "-i", "--input-file",
    help="The CSV file to read opcodes from.",
    default="opcodes.csv"
)

parser.add_argument(
    "-o", "--output-file",
    help="The C Header file to write to.",
    default="include/opcode.h"
)

args = parser.parse_args()

FILE_TEMPLATE = """\
/**
 * opcode.h
 *
 * This file is auto-generated by the script 'scripts/opcode_gen.py'.
 *
 * Please do not modify it by hand.
 *
 */

#ifndef OPCODE_H
#define OPCODE_H

#include <stdint.h>
typedef uint8_t opcode_t;

#include "vm.h"

/**
 * A string array of all opcode names.
 */
static const char *OPCODE_NAMES[] = {
{{opcode_names}}
};

/**
 * The smallest possible value for an opcode.
 */
#define OPCODE_MIN {{opcode_min}}

/**
 * The largest possible value for an opcode.
 */
#define OPCODE_MAX {{opcode_max}}

{{opcodes}}

/**
 * Functions to execute each opcode.
 * 
 * Not all of these will be implemented, as some are inline.
 */
{{opcode_functions}}

#endif
"""

OPCODE_TEMPLATE = """\
/**
 * {{opcode}}
 * 
 * {{details}}
 */
#define OP_{{opcode}} {{value}}

"""

with open(args.input_file, "r", encoding='utf-8') as fp:
    opcodes = [opcode for opcode in csv.DictReader(fp)]

    opcodes_text = ""
    opcode_names = ""
    opcode_functions = ""

    for i, opcode in enumerate(opcodes):
        opcodes_text += OPCODE_TEMPLATE.replace("{{opcode}}", opcode['opcode'].upper()).replace(
            "{{details}}", opcode['details']).replace("{{value}}", hex(i))

        if i == len(opcodes) - 1:
            opcode_names += f"\t\"{opcode['opcode']}\""
        else:
            opcode_names += f"\t\"{opcode['opcode']}\",\n"

        opcode_functions += f"void saw_insn_{opcode['opcode']}(vm_t *vm);\n"

    with open(args.output_file, "w", encoding="utf-8") as fp:
        fp.write(FILE_TEMPLATE.replace("{{opcodes}}", opcodes_text).replace("{{opcode_names}}", opcode_names).replace(
            "{{opcode_min}}", str(0)).replace("{{opcode_max}}", str(len(opcodes) - 1)).replace("{{opcode_functions}}", opcode_functions))
