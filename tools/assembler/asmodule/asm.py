# First pass for the assembler. Resolves labels and directives.
from bitstring import BitArray
from typing import Dict, List

from . import machineconst as mc

SECTION_ALIGNMENT = 64  # section alignment in bytes


def parse_number(number: str) -> int:
    return int(number[1:], 16) if number[0] == "$" else int(number)


def align(address: int, alignment: int) -> int:
    return address + alignment - (address % alignment)


class Assembler:
    """A class modeling the assembler inner workings"""

    def __init__(self, source: str) -> None:
        self.source: str = source
        self.labels: Dict[str, int] = {}
        self.current_section: str = "text"
        # the text emitted by the assembler is stored in one of this sections
        self.sections: Dict[str, str] = {"text": ""}
        self.section_addresses: Dict[str, int] = {
            "text": 0
        }  # the address of the current section
        self.section_start_addresses: Dict[str, int] = {
            "text": 0
        }  # the address of the current section
        self.pseudo_label_addresses: List[int] = []  # a stack
        self.word: int = 0
        self.opcodes: Dict[str, int] = mc.Opcodes
        self.InstructionTypes: Dict[str, List[str]] = mc.InstructionTypes
        self.FunctionCodes: Dict[str, int] = mc.FunctionCodes
        self.Registers: Dict[str, int] = mc.Registers
        self.isfunct7: List[str] = mc.Funct7Set
        self.header: str = ""
        self.org_not_used: bool = True
        self.debug_info: List = []
        self.scratch: Dict[str, str] = {}

    def new_scratch_section(self, name: str) -> None:
        self.scratch.update({name: ""})

    def emit_in_section(self, section: str, tokens: List[str]) -> None:
        if section in self.sections:
            self.sections[section] += " ".join(tokens) + "\n"
        else:
            self.scratch[section] += " ".join(tokens) + "\n"

    def emit_line(self, tokens: List[str]) -> None:
        self.sections[self.current_section] += " ".join(tokens) + "\n"

    def emit_byte_in_section(self, byte: str, section: str) -> None:
        if section in self.sections:
            self.sections[section] += byte + "\n"
        else:
            self.scratch[section] += byte + "\n"

    def remove_comments(self) -> str:
        new_source: str = ""
        for line in self.source.split("\n"):
            if line.lstrip().startswith(";"):
                continue
            elif ";" in line:
                new_source += line.split(";")[0] + "\n"
                continue
            else:
                new_source += line + "\n"
        return new_source

    def index_labels(self) -> None:
        for line in self.source.split("\n"):
            if line == "":
                continue  # skip empty lines
            if line[0].isspace():
                # add to the current address for indexing labels at correct spot
                if line.lstrip().startswith(".org"):
                    arg: str = line.split()[self.word + 1]

                    assert (
                        parse_number(arg) % 4 == 0
                    ), "org address must be aligned to 4 bytes"
                    self.section_addresses[self.current_section] = parse_number(arg)
                    self.section_start_addresses[self.current_section] = parse_number(
                        arg
                    )

                elif line.lstrip().startswith(".section"):
                    new_section = line.split()[self.word + 1]
                    new_address = align(
                        self.section_addresses[self.current_section], SECTION_ALIGNMENT
                    )
                    self.sections.update({new_section: ""})
                    self.section_addresses.update({new_section: new_address})
                    self.section_start_addresses.update({new_section: new_address})
                    self.current_section = new_section

                elif line.lstrip().startswith(".byte"):
                    for _ in line.lstrip().split()[self.word + 1 :]:
                        self.section_addresses[self.current_section] += 1
                
                elif line.lstrip().startswith(".word"):
                    for _ in line.lstrip().split()[self.word + 1 :]:
                        self.section_addresses[self.current_section] += 2
                        
                elif line.lstrip().startswith(".dword"):
                    for _ in line.lstrip().split()[self.word + 1 :]:
                        self.section_addresses[self.current_section] += 4
                        
                elif line.lstrip().startswith(".dblock"):
                    arg: str = line.split()[self.word + 1]

                    assert parse_number(arg) % 4 == 0, (
                        "block allocations must be aligned to 4 bytes: » " + line
                    )
                    self.section_addresses[self.current_section] += parse_number(arg)
                    
                elif line.lstrip().startswith(".string"):
                    arg = line.split(".string")[1]
                    arg = arg.lstrip()[1:-2]
                    # -2 to remove the quotes
                    self.section_addresses[self.current_section] += len(arg) 
                    
                elif line.lstrip().startswith(".align"):
                    arg: str = line.lstrip().split()[self.word + 1]
                    assert parse_number(arg) % 2 == 0, (
                        "alignments must be a multiple of 2 bytes »" + line
                    )
                    new_address = align(
                        self.section_addresses[self.current_section], parse_number(arg)
                    )
                    self.section_addresses[self.current_section] = new_address

                else:
                    if line.split() == []:
                        continue # skip empty lines
                    
                    mnemonic = line.split()[self.word]
                    instruction_size: int = (
                        8 if mnemonic in self.InstructionTypes["pseudo-label"] else 4
                    )
                    # 4 bytes for each instruction

                    if mnemonic in self.InstructionTypes["pseudo-label"]:
                        self.pseudo_label_addresses.append(
                            self.section_addresses[self.current_section]
                        )

                    self.section_addresses[self.current_section] += instruction_size
            else:
                # if line starts with a character other than a space, it is a label

                label: str = line.split(":")[0]
                self.labels.update(
                    {label: self.section_addresses[self.current_section]}
                )

    def resolve_labels(self) -> None:
        self.current_section = "text"
        self.section_addresses[self.current_section] = self.section_start_addresses[self.current_section]

        for line in self.source.split("\n"):
            tokens_in_line = line.lstrip().split()
            if len(tokens_in_line) <= 0:
                continue  # skip empty lines

            if tokens_in_line[0].endswith(":"):
                # it is a label definition and should be removed
                tokens_in_line = tokens_in_line[1:]
                
            if len(tokens_in_line) <= 0:
                # skip empty lines
                continue
            
            # resolve labels first in directives (only dword supports this)
            label_ref_in_line = [
                label for label in self.labels if label in tokens_in_line
            ]

            assert len(label_ref_in_line) <= 1, "only one label reference per line is allowed"

            if tokens_in_line[self.word] == ".section":
                # change current section
                self.current_section = tokens_in_line[self.word + 1]
                self.section_addresses[self.current_section] = self.section_start_addresses[self.current_section]

            if len(label_ref_in_line) <= 0:
                if line.lstrip().startswith(".org"):
                    self.section_addresses[self.current_section] = parse_number(tokens_in_line[self.word + 1])
        
                elif line.lstrip().startswith(".byte"):
                    for _ in line.lstrip().split()[self.word + 1 :]:
                        self.section_addresses[self.current_section] += 1
                
                elif line.lstrip().startswith(".word"):
                    for _ in line.lstrip().split()[self.word + 1 :]:
                        self.section_addresses[self.current_section] += 2
                        
                elif line.lstrip().startswith(".dword"):
                    for _ in line.lstrip().split()[self.word + 1 :]:
                        self.section_addresses[self.current_section] += 4
                        
                elif line.lstrip().startswith(".dblock"):
                    arg: str = line.split()[self.word + 1]

                    assert parse_number(arg) % 4 == 0, (
                        "block allocations must be aligned to 4 bytes: » " + line
                    )
                    self.section_addresses[self.current_section] += parse_number(arg)
                    
                elif line.lstrip().startswith(".string"):
                    arg = line.split(".string")[1]
                    arg = arg.lstrip()[1:-2]
                
                    print(arg, len(arg))
                    self.section_addresses[self.current_section] += len(arg)
                    
                elif line.lstrip().startswith(".align"):
                    arg: str = line.lstrip().split()[self.word + 1]
                    assert parse_number(arg) % 2 == 0, (
                        "alignments must be a multiple of 2 bytes »" + line
                    )
                    new_address = align(
                        self.section_addresses[self.current_section], parse_number(arg)
                    )
                    self.section_addresses[self.current_section] = new_address
                
                else:
                    self.section_addresses[self.current_section] += 4
                
                
                self.emit_line(tokens_in_line)
                continue  # skip if no label is found

            # mutating this variable and making it a string. This is python LOL
            label_ref_in_line = label_ref_in_line[0]
            
            if tokens_in_line[self.word] == ".word":
                tokens_in_line[tokens_in_line.index(label_ref_in_line)] = str(
                    self.labels[label_ref_in_line] & 0xffff
                )
                self.emit_line(tokens_in_line)
                self.section_addresses[self.current_section] += 2

            elif tokens_in_line[self.word] == ".dword":
                tokens_in_line[tokens_in_line.index(label_ref_in_line)] = str(
                    self.labels[label_ref_in_line]
                )
                self.emit_line(tokens_in_line)
                self.section_addresses[self.current_section] += 4
                

            elif tokens_in_line[self.word] in self.InstructionTypes["branch"]:
                pcrel_13 = (
                    self.labels[label_ref_in_line]
                    - self.section_addresses[self.current_section]
                )
                assert (
                    pcrel_13 < 4096 and pcrel_13 >= -4096
                ), "branch target out of range (13 bit signed offset)"
                tokens_in_line[tokens_in_line.index(label_ref_in_line)] = str(pcrel_13)
                self.emit_line(tokens_in_line)
                self.section_addresses[self.current_section] += 4

            elif tokens_in_line[self.word] in self.InstructionTypes["jal"]:
                pcrel_21 = (
                    self.labels[label_ref_in_line]
                    - self.section_addresses[self.current_section]
                )
                # print(
                #     "PCREL21",
                #     pcrel_21,
                #     "LABEL at",
                #     self.labels[label_ref_in_line],
                #     "CURRENT ADDRESS",
                #     self.section_addresses[self.current_section],
                #     line
                # )
                assert (
                    pcrel_21 < 1048576 and pcrel_21 >= -1048576
                ), "jal target out of range (21 bit signed offset)"
                assert (
                    pcrel_21 % 4 == 0
                ), "jal target must be aligned to 4 bytes, as all instructions are 4 byte aligned"
                tokens_in_line[tokens_in_line.index(label_ref_in_line)] = str(pcrel_21)
                self.emit_line(tokens_in_line)
                self.section_addresses[self.current_section] += 4
                
            elif tokens_in_line[self.word] in self.InstructionTypes["longjump"]:
                address = self.labels[label_ref_in_line]
                # print(
                #     "PCREL21",
                #     pcrel_21,
                #     "LABEL at",
                #     self.labels[label_ref_in_line],
                #     "CURRENT ADDRESS",
                #     self.section_addresses[self.current_section],
                #     line
                # )
                assert (
                    address >= 0 and address < 0xffffff
                ), "jump target out of range (24 bit unsigned address)"
                assert (
                    address % 4 == 0
                ), "jump target must be aligned to 4 bytes, as all instructions are 4 byte aligned"
                tokens_in_line[tokens_in_line.index(label_ref_in_line)] = str(address)
                self.emit_line(tokens_in_line)
                self.section_addresses[self.current_section] += 4

            elif tokens_in_line[self.word] in self.InstructionTypes["pseudo-label"]:
                assert len(self.pseudo_label_addresses) > 0, (
                    "no pseudo-label defined » " + line
                )
                stub = (
                    "__STUB__"
                    + str(self.labels[label_ref_in_line])
                    + "|"
                    + str(self.pseudo_label_addresses.pop(0))
                )  # stub info for the assembler to calculate
                #print("LABEL", label_ref_in_line, tokens_in_line[tokens_in_line.index(label_ref_in_line)], "FOR", stub, "CURRENT ADDRESS", self.section_addresses[self.current_section])
                tokens_in_line[tokens_in_line.index(label_ref_in_line)] = stub
                self.emit_line(tokens_in_line)
                self.section_addresses[self.current_section] += 8

            else:
                # stop if there was a label reference in the line but no instruction that supports it
                raise Exception("label reference in line but no instruction that supports it\n" + line + " " + label_ref_in_line + "\n" + str(self.labels) )

    def resolve_pseudo(self) -> None:
        """Pseudo instructions and instructions are only legal in the .text section"""
        self.new_scratch_section("resolve_pseudo")
        expanded_section = "resolve_pseudo"

        for line in self.sections["text"].split("\n"):

            tokens_in_line = line.lstrip().split()
            if len(tokens_in_line) <= 0:
                continue  # skip empty lines

            if tokens_in_line[self.word] in self.InstructionTypes["pseudo-label"]:
                # resolve the label reference in the line. It was converted to a stub in the form:
                # __STUB__label_address|current_section_address

                stub = [stub for stub in tokens_in_line if stub.startswith("__STUB__")]

                assert len(stub) == 1, "only one stub per line is allowed " + str(tokens_in_line)
                stub = stub[0].lstrip("__STUB__")

                label_address = int(stub.split("|")[0])
                instruction_address = int(stub.split("|")[1])
                pcrel = label_address - instruction_address

                if tokens_in_line[self.word] == "la":
                    auipc = ["auipc", tokens_in_line[self.word + 1], str(pcrel >> 8)]
                    addi = [
                        "addi",
                        tokens_in_line[self.word + 1],
                        tokens_in_line[self.word + 1],
                        str(pcrel & 0x0FF),
                    ]

                    self.emit_in_section(expanded_section, auipc)
                    self.emit_in_section(expanded_section, addi)

                elif tokens_in_line[self.word] == "call":
                    auipc = ["auipc", "x1", str(pcrel >> 8)]
                    jalr = ["jalr", "x1", "x1", str(pcrel & 0x0FF)]

                    self.emit_in_section(expanded_section, auipc)
                    self.emit_in_section(expanded_section, jalr)

                elif tokens_in_line[self.word] == "tail":
                    auipc = ["auipc", "x6", str(pcrel >> 8)]
                    jalr = ["jalr", "x0", "x6", str(pcrel & 0x0FF)]

                elif tokens_in_line[self.word] == "lbg":
                    auipc = ["auipc", tokens_in_line[self.word + 1], str(pcrel >> 8)]
                    lb = [
                        "lb",
                        tokens_in_line[self.word + 1],
                        tokens_in_line[self.word + 1],
                        str(pcrel & 0x0FF)
                    ]

                    self.emit_in_section(expanded_section, auipc)
                    self.emit_in_section(expanded_section, lb)

                elif tokens_in_line[self.word] == "lhg":
                    auipc = ["auipc", tokens_in_line[self.word + 1], str(pcrel >> 8)]
                    lh = [
                        "lh",
                        tokens_in_line[self.word + 1],
                        str(pcrel & 0x0FF),
                        tokens_in_line[self.word + 1],
                    ]

                    self.emit_in_section(expanded_section, auipc)
                    self.emit_in_section(expanded_section, lh)

                elif tokens_in_line[self.word] == "sbg":
                    assert (
                        len(tokens_in_line) > self.word + 3
                    ), "sbg instruction must have 3 arguments"

                    auipc = ["auipc", tokens_in_line[self.word + 3], str(pcrel >> 8)]
                    sb = [
                        "sb",
                        tokens_in_line[self.word + 1],
                        tokens_in_line[self.word + 3],
                        str(pcrel & 0x0FF),
                    ]

                    self.emit_in_section(expanded_section, auipc)
                    self.emit_in_section(expanded_section, sb)

                elif tokens_in_line[self.word] == "shg":
                    assert (
                        len(tokens_in_line) > self.word + 3
                    ), "shg instruction must have 3 arguments"
                    auipc = ["auipc", tokens_in_line[self.word + 3], str(pcrel >> 8)]
                    sh = [
                        "sh",
                        tokens_in_line[self.word + 1],
                        str(pcrel & 0x0FF),
                        tokens_in_line[self.word + 3],
                    ]

                    self.emit_in_section(expanded_section, auipc)
                    self.emit_in_section(expanded_section, sh)

                elif tokens_in_line[self.word] == "tail":
                    auipc = ["auipc", "x6", str(pcrel >> 8)]
                    jalr = ["jalr", "x0", "x6", str(pcrel & 0x0FF)]

                    self.emit_in_section(expanded_section, auipc)
                    self.emit_in_section(expanded_section, jalr)
            else:
                self.emit_in_section(expanded_section, tokens_in_line)

        self.debug_info.append("text-> " + self.sections["text"])
        self.debug_info.append("text expanded=> " + self.scratch[expanded_section])
        self.sections["text"] = self.scratch[expanded_section]

    def generate_instruction(self, tokens: List[str]) -> str:
        """Generates the machine code for one instruction.
        Must be called only for text section and after resolving labels and pseudo instructions

        Args:
            tokens (List[str]): a line of tokens conforming one instruction

        Returns:
            str: a string in hexadecimal, the machine code of the instruction IN BIG ENDIAN
        """
        mnemonic = tokens[self.word]
        assert mnemonic in self.FunctionCodes or mnemonic in self.opcodes, (
            "mnemonic not known: " + mnemonic + " in " + str(tokens)
        )

        instruction = BitArray("0x00000000")  # 32 bit instruction

        def make_opcode(opcode):
            return BitArray(uint=opcode, length=7)

        def make_funct3(funct3):
            return BitArray(uint=funct3, length=3)

        def make_register(rd):
            return BitArray(uint=rd, length=5)

        def make_imm12(imm12):
            return BitArray(int=imm12, length=12)

        if mnemonic in self.InstructionTypes["lui"]:
            assert len(tokens) == 3, "lui instruction must have 2 arguments"
            opcode = make_opcode(self.opcodes[mnemonic])
            rd = make_register(self.Registers[tokens[self.word + 1]])
            imm = parse_number(tokens[self.word + 2])
            assert imm < 2 ** 16, "lui immediate must be 16 bits"

            instruction[:16] = imm
            instruction[20:25] = rd
            instruction[25:] = opcode

        elif mnemonic in self.InstructionTypes["auipc"]:
            assert len(tokens) == 3, "auipc instruction must have 2 arguments"
            opcode = make_opcode(self.opcodes[mnemonic])
            rd = make_register(self.Registers[tokens[self.word + 1]])
            imm = parse_number(tokens[self.word + 2])

            instruction[:16] = imm
            instruction[20:25] = rd
            instruction[25:] = opcode
            print("AUIPC: ", tokens, instruction.hex)

        elif mnemonic in self.InstructionTypes["jal"]:
            assert len(tokens) == 3, "jal instruction must have 2 arguments " + str(tokens)
            opcode = make_opcode(self.opcodes[mnemonic])
            rd = make_register(self.Registers[tokens[self.word + 1]])
            pcrel = parse_number(tokens[self.word + 2])
            assert (pcrel) < 1048576 and (
                pcrel
            ) >= -1048576, "pcrel21 must be between -1048576 and 1048575"
            assert pcrel & 0x1 == 0, "pcrel21 must be even"

            pcrel21 = BitArray(int=pcrel >> 1, length=20)
            instruction[:20] = pcrel21
            instruction[20:25] = rd
            instruction[25:] = opcode
        elif mnemonic in self.InstructionTypes["longjump"]:
            assert len(tokens) == 3, "jal instruction must have 2 arguments " + str(tokens)
            opcode = make_opcode(self.opcodes[mnemonic])
            link = 0 if self.Registers[tokens[self.word + 1]] == 0 else 1
            address = parse_number(tokens[self.word + 2])
            assert (address) >= 0 and address <= 0xffffff , "address must be in range"
            assert address & 0x3 == 0, "address must be multiple of 4"

            pcrel21 = BitArray(uint=address >> 2, length=22)
            instruction[:22] = pcrel21
            instruction[22] = 0
            instruction[23] = 0
            instruction[24] = link
            instruction[25:] = opcode

        # TODO: CHeck if this is correct

        elif mnemonic in self.InstructionTypes["jalr"]:
            assert len(tokens) == 4, "jalr instruction must have 3 arguments"
            opcode = make_opcode(self.opcodes[mnemonic])
            funct3 = 0
            rd = make_register(self.Registers[tokens[self.word + 1]])
            rs1 = make_register(self.Registers[tokens[self.word + 2]])
            imm = make_imm12(parse_number(tokens[self.word + 3]))
            assert (
                imm.int < 2048 and imm.int >= -2048
            ), "immetiate must be between -2048 and 2047"

            instruction[:12] = imm
            instruction[12:17] = rs1
            instruction[20:25] = rd
            instruction[25:] = opcode

        elif mnemonic in self.InstructionTypes["branch"]:
            assert len(tokens) == 4, "branch instruction must have 3 arguments"
            opcode = make_opcode(self.opcodes["branch"])
            funct3 = make_funct3(self.FunctionCodes[mnemonic])
            rs1 = make_register(self.Registers[tokens[self.word + 1]])
            rs2 = make_register(self.Registers[tokens[self.word + 2]])
            pcrel = parse_number(tokens[self.word + 3])
            assert (
                pcrel < 4096 and pcrel >= -4096
            ), "pcrel13 must be between -4096 and 4095"
            assert pcrel & 0x1 == 0, "pcrel13 must be even"

            pcrel = pcrel >> 1
            pcrel_a = BitArray(uint=(pcrel & 0xFE0) >> 5, length=7)
            pcrel_b = BitArray(uint=(pcrel & 0x1F), length=5)

            instruction[:7] = pcrel_a
            instruction[7:12] = rs2
            instruction[12:17] = rs1
            instruction[17:20] = funct3
            instruction[20:25] = pcrel_b
            instruction[25:] = opcode

        elif mnemonic in self.InstructionTypes["load"]:
            assert len(tokens) == 4, "load instruction must have 3 arguments " + str(tokens)
            opcode = make_opcode(self.opcodes["load"])
            funct3 = make_funct3(self.FunctionCodes[mnemonic])
            rd = make_register(self.Registers[tokens[self.word + 1]])
            rs1 = make_register(self.Registers[tokens[self.word + 2]])
            offset = BitArray(int=parse_number(tokens[self.word + 3]), length=12)
            assert (
                offset.int < 2048 and offset.int >= -2048
            ), "offset must be between -2048 and 2047"

            instruction[:12] = offset
            instruction[12:17] = rs1
            instruction[17:20] = funct3
            instruction[20:25] = rd
            instruction[25:] = opcode

        elif mnemonic in self.InstructionTypes["store"]:
            assert len(tokens) == 4, "store instruction must have 3 arguments"
            print(tokens)
            opcode = make_opcode(self.opcodes["store"])
            funct3 = make_funct3(self.FunctionCodes[mnemonic])
            rs1 = make_register(self.Registers[tokens[self.word + 2]]) # rs1 is not the value, but the destination
            rs2 = make_register(self.Registers[tokens[self.word + 1]])
            offset = parse_number(tokens[self.word + 3])
            assert (
                offset < 2048 and offset >= -2048
            ), "offset must be between -2048 and 2047"

            offset_a = BitArray(uint=(offset & 0xFE0) >> 5, length=7)
            offset_b = BitArray(uint=(offset & 0x1F), length=5)

            instruction[:7] = offset_a
            instruction[7:12] = rs2
            instruction[12:17] = rs1
            instruction[17:20] = funct3
            instruction[20:25] = offset_b
            instruction[25:] = opcode

        elif mnemonic in self.InstructionTypes["mathi"]:
            assert (
                len(tokens) == 4
            ), "math immediate instruction must have 3 arguments »" + str(tokens)
            opcode = make_opcode(self.opcodes["mathi"])
            funct3 = make_funct3(self.FunctionCodes[mnemonic])
            rd = make_register(self.Registers[tokens[self.word + 1]])
            rs1 = make_register(self.Registers[tokens[self.word + 2]])
            imm = parse_number(tokens[self.word + 3])
            assert imm < 2048 and imm >= -2048, "offset must be between -2048 and 2047"

            instruction[:12] = make_imm12(imm)
            instruction[12:17] = rs1
            instruction[17:20] = funct3
            instruction[20:25] = rd
            instruction[25:] = opcode

            instruction[1] |= 1 if mnemonic in self.isfunct7 else 0

        elif mnemonic in self.InstructionTypes["mathr"]:
            assert len(tokens) == 4, "math register instruction must have 3 arguments"
            opcode = make_opcode(self.opcodes["mathr"])
            funct3 = make_funct3(self.FunctionCodes[mnemonic])
            rd = make_register(self.Registers[tokens[self.word + 1]])
            rs1 = make_register(self.Registers[tokens[self.word + 2]])
            rs2 = self.Registers[tokens[self.word + 3]]

            instruction[7:12] = rs2
            instruction[12:17] = rs1
            instruction[17:20] = funct3
            instruction[20:25] = rd
            instruction[25:] = opcode

            instruction[1] |= 1 if mnemonic in self.isfunct7 else 0

        elif mnemonic in self.InstructionTypes["system"]:
            opcode = make_opcode(self.opcodes["system"])
            funct3 = make_funct3(self.FunctionCodes[mnemonic])
            rd = make_register(self.Registers[tokens[self.word + 1]])

            instruction[25:] = opcode
            instruction[20:25] = rd
            instruction[17:20] = funct3

            if mnemonic == "trap":
                vector = parse_number(tokens[self.word + 2])
                assert (
                    vector < 0x80 and vector >= 0
                ), "trap vector must be between 0 and 0x7F"
                instruction[4:12] = BitArray(uint=vector, length=8)

            elif mnemonic == "ssr":
                assert len(tokens) == 4, "ssr instruction must have 3 arguments"
                rs1 = make_register(self.Registers[tokens[self.word + 2]])
                imm = parse_number(tokens[self.word + 3])
                assert imm < 256, "ssr instruction must have a 8-bit immediate"

                instruction[12:17] = rs1
                instruction[4:12] = BitArray(uint=imm, length=8)

            elif mnemonic == "gsr":
                assert len(tokens) == 3, "gsr instruction must have 2 arguments"
                rs1 = make_register(self.Registers[tokens[self.word + 2]])
                imm = 0

                instruction[12:17] = rs1
                instruction[4:12] = BitArray(uint=imm, length=8)

            elif mnemonic == "gpsr":
                assert len(tokens) == 2, "gpsr instruction must have 1 arguments"

        elif mnemonic in self.InstructionTypes["supervisor"]:
            opcode = make_opcode(self.opcodes["supervisor"])
            funct3 = make_funct3(self.FunctionCodes[mnemonic])

            instruction[17:20] = funct3
            instruction[25:] = opcode

            if mnemonic == "rti":
                assert len(tokens) == 1, "rti instruction takes no argument"

            elif mnemonic == "spsr":
                assert len(tokens) == 3, "gpsr instruction must have 2 arguments"
                rd = make_register(self.Registers[tokens[self.word + 1]])
                imm = parse_number(tokens[self.word + 2])
                assert imm < 65536, "spsr instruction must have a 16-bit immediate"

                instruction[:16] = BitArray(uint=imm, length=16)
                instruction[20:25] = rd

        else:
            raise Exception("Unknown mnemonic: " + mnemonic)

        self.debug_info.append(str(tokens) + ":" + instruction.hex)

        return instruction.hex

    def resolve_directives(self, section: str) -> int:
        """Resolves all directives in the given section and emits the resolved section. Shall be called last"""

        self.new_scratch_section("resolved_directives_" + section)
        resolved_section = "resolved_directives_" + section

        self.section_addresses[section] = self.section_start_addresses[section]
        address = self.section_addresses[section]

        for line in self.sections[section].split("\n"):

            tokens_in_line = line.split()
            if len(tokens_in_line) <= 0:
                continue  # skip empty lines

            if tokens_in_line[self.word] == ".org":
                self.header = BitArray(
                    int=parse_number(tokens_in_line[self.word + 1]), length=24
                ).hex
                assert self.org_not_used, "org directive can only be used once"
                self.org_not_used = False
                continue
            elif tokens_in_line[self.word] == ".section":
                continue

            elif tokens_in_line[self.word] == ".align":
                arg = parse_number(tokens_in_line[self.word + 1])
                assert arg > 0 and arg < 256, "alignment must be between 1 and 255"

                while address % arg != 0:
                    self.emit_byte_in_section("00", resolved_section)
                    address += 1

                continue

            elif tokens_in_line[self.word] == ".byte":

                for token in tokens_in_line[self.word + 1 :]:
                    byte = parse_number(token)
                    assert byte >= 0 and byte < 256, "byte must be between 0 and 255"
                    self.emit_byte_in_section(
                        BitArray(uint=byte, length=8).hex, resolved_section
                    )
                    address += 1

                continue

            elif tokens_in_line[self.word] == ".word":

                for token in tokens_in_line[self.word + 1 :]:
                    word = parse_number(token)
                    assert (
                        word >= 0 and word < 65536
                    ), "word must be between 0 and 65535"
                    hi = BitArray(uint=(word & 0xFF00) >> 8, length=8).hex
                    lo = BitArray(uint=(word & 0x00FF), length=8).hex
                    self.emit_byte_in_section(lo, resolved_section)
                    self.emit_byte_in_section(hi, resolved_section)
                    address += 2

                continue

            elif tokens_in_line[self.word] == ".dword":

                for token in tokens_in_line[self.word + 1 :]:
                    dword = parse_number(token)
                    assert (
                        dword >= 0 and dword < 4294967296
                    ), "dword must be between 0 and 4294967295"
                    byte1 = BitArray(uint=(dword & 0xFF000000) >> 24, length=8).hex
                    byte2 = BitArray(uint=(dword & 0x00FF0000) >> 16, length=8).hex
                    byte3 = BitArray(uint=(dword & 0x0000FF00) >> 8, length=8).hex
                    byte4 = BitArray(uint=(dword & 0x000000FF), length=8).hex
                    self.emit_byte_in_section(byte4, resolved_section)
                    self.emit_byte_in_section(byte3, resolved_section)
                    self.emit_byte_in_section(byte2, resolved_section)
                    self.emit_byte_in_section(byte1, resolved_section)
                    address += 4

                continue

            elif tokens_in_line[self.word] == ".dblock":
                arg = parse_number(tokens_in_line[self.word + 1])
                assert arg > 0, "dblock must be greater than zero"
                
                byte = "00"
                if len(tokens_in_line) == 3:
                    byte = parse_number(tokens_in_line[self.word + 2])
                    assert byte < 256, "optional dblock arg must be a byte"
                    byte = hex(byte)[2:]
                
                    
                for _ in range(arg):
                    self.emit_byte_in_section(byte, resolved_section)
                    address += 1

                continue

            elif tokens_in_line[self.word] == ".string":
                string = line.split(".string")[1]
                string = string.lstrip()[1:-1]

                for char in string:
                    self.emit_byte_in_section(
                        BitArray(uint=ord(char), length=8).hex, resolved_section
                    )
                    address += 1

                continue

            else:
                # at this point, if it not a directive it must be an instruction
                instruction = self.generate_instruction(tokens_in_line)

                byte1 = instruction[0:2]
                byte2 = instruction[2:4]
                byte3 = instruction[4:6]
                byte4 = instruction[6:8]

                # store in little endian

                self.emit_byte_in_section(byte4, resolved_section)
                self.emit_byte_in_section(byte3, resolved_section)
                self.emit_byte_in_section(byte2, resolved_section)
                self.emit_byte_in_section(byte1, resolved_section)

                address += 4
                continue

        # return the resolved section
        self.debug_info.append(section + "-> " + self.sections[section])
        self.debug_info.append(
            section + "resolved => " + self.scratch[resolved_section]
        )
        self.sections[section] = self.scratch[resolved_section]

        return address

    def assemble_raw(self) -> str:
        """Assembles the source file and emits the binary file"""
        self.source = self.remove_comments()
            
        self.index_labels()
        self.resolve_labels()
        self.resolve_pseudo()

        for section in self.sections:
            end_of_section = self.resolve_directives(section)
            print(
                section
                + " end: "
                + str(end_of_section)
                + " aligned to "
                + str(align(end_of_section, SECTION_ALIGNMENT))
            )
            for _ in range(end_of_section, align(end_of_section, SECTION_ALIGNMENT)):
                self.emit_byte_in_section("00", section)

        assembled_hex = ""
        for section in self.sections:
            assembled_hex += self.sections[section]

        if self.org_not_used:
            self.header = BitArray(uint=0, length=24).hex

        assembled_hex = self.header + assembled_hex

        return assembled_hex
