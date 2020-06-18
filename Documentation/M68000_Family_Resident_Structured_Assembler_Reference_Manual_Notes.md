# M68000_Family_Resident_Structured_Assembler_Reference_Manual Notes

## Assembler Directives

#### Assembly Control Directives
* END - Program END
* INCLUDE - INCLUDE Secondary file
* MASK2 = Assemble for MASK2
* OFFSET - Define Offsets
* ORG - Absolute ORiGin
* SECTION - Relocatable Program SECTION

#### Symbol Definition Directives
* EQU - EQUate symbol value
* REG - Define register list 
* SET - SET symbol value

#### Data Definition/Storage Allocation Directives
* COMLINE - COMmand LINE
* DC - Define Constant
* DCB - Define Constant Block
* DS - Define Storage

#### Listing Control and Output Options
* FAIL - Programmer generated error

### Linkage Editor Control Directives
* IDNT - relocatable IDeNTification record
* XDEF - eXternal symbol DEFinition
* XREF - eXternal symbol REFerence

### Structured Control Statements
#### SYNTAX
* IF -
* FOR - 
* REPEAT - 
* WHILE - 

### Notation
Commands and other I/O are presented in this manual in a modified Backus-Naur Form (BNF) syntax. Certain symbols in the syntax, where noted, are used in reail I/O; however, others are meta-symbols and their meanings are as follows:
< > The angular brackets enclose a symbol, known as a syntactic variable, that is replaced in a command line by one of a classof symbols it represents. In some cases, where noted, angular brackets are required characters.
| This symbol indicates that a choice is to be made. One of several symbols, separated by this symbol, should be selevted.
[ ] Square brackets enclose a symbol that is optional. The enclosed symbol may occur zero or one time. In some cases, where noted, square brackets may occur zero or one time.
[ ]... Square brackets followed by periods enclose a symbol that is optional/repetitive. THe symbol may appear zero or more times.

## Source Program Coding
### Comments
\* At the beginning of a line, starting in column one, where an asterisk (\*) is the first character in the line. The entire line is a comment, and an instruction or directive in this line is not recognized.
  Following the operation and operand fields of an assembler instruction or directive, where it is preceded by at least one space.
**Examples**
* THIS ENTIRE LINE IS A COMMENT
BRA LAB2   THIS COMMENT FOLLOWS AN INSTRUCTION
### Executable Instruction Format
Labels - A label which begins in the fist column of the line may be terminated by either a space or a colon. A label may be preceded by one or more spaces, provided it is then terminated by a colon. Colon is not part of the label.
    Labels are allowed on all instructions and assembler directives which define data structures. For such operations, the label is defined with a value equal to the location counter for the instructions or directive, including a designation for the program section in which the definition appears.
    Labesl are required on assembler directives which define symbols values (SET, EQU, REG). For directives, the label is defined with a value corresponding to the expression in the oerand field.
    Labels on MACRO definistions are saved as the mnemonic by which that macro is subsequently invoked. No memory address is associated with such labels. Label is required on IDNT directive.
### Operation Field
#### Data field sizes
B = Byte (8-bit)
W = Word (16-bit)
L = Longword (32-bit)
S = Byte (8-bit offset for certain branch instructions)

#### Operand Field
Separated from the operation field by at least one space.
When two or more operand subfields appear within a statement, they must be separated by a comma but may not container embedded spaces; e.g., D1, D2 is illegal.
In an instruction like `ADD D1,D2` the fist subfeild `D1` is genearlly applied to the second subfield `D2` and the results placed in the second subfield.
For most two-operand instructions, the general format `opcode srouce,destination` applies.

### Addressing Modes
References to data addresses may be odd only if a byte is referenced. Data references involving words or longwords must be even. Likewise, instructions must begin on an even byte boundary.

Bits for a byte are numbered 7 to 0, with 7 being the most significant bit position and 0 the least significant.
Bits for a word are numbered 15 to 0.
Bits for a longword are numbered from 31 to 0.

#### Symbols
A<sub>n</sub> Address register number "n" (0-7).
D<sub>n</sub> Data register number "n" (0-7).
PC Program counter.
B,W,L Byte, word, longword, data sizes.
d(A<sub>n</sub>) Address register indirect with displacment (d).
d(A<sub>n</sub>, R<sub>i</sub>) Address register indirect with index (R<sub>i</sub>) plus displacement (d).
d(PC) Program counter with displacement (d).
d(PC, R<sub>i</sub>) Program counter with index (R<sub>i</sub>) plus displacement (d).
<absolute> Absolute expression.
<simple> Simple relocatable expression.
<complex> Complex relocatable expression.
<ea> Effective address expression.
<iea> Indirect effective address expression.

**Addressing modes** defined for the M<F3>68000  
1. Register indirect  A<sub>n</sub>
                    D<sub>n</sub>
2. Memory address
  1. Simple indirect         (A<sub>n</sub>)
     Exmaple: The address of the operand is in the address register specified by the register field.
      ```
      MOVE #5,(A5)   Move value 5 to word whose address is container in A5
      SUB.L (A1),D0  Subtract from D0 the value in the longword whose address is contained in A1.
      ```
  2. Predecrement            -(A<sub>n</sub>)
     Example: The address of the operand is in the address register specified by the register field. Before the operand is used, it is decremented by one, two, or four, depending upon whether the operand size is byte (.B), word (.W), or long (.L).
      ```
      CLR -(A2)        Subtract 2 from A2; clear word whose address is now in A2.
      CMP.L -(A0),D0   Substract 4 from A0; compare longword whose address is now in A0 with contents of D0.
      ```
  3. Postincrement           (A<sub>n</sub>)+
     Example: The address of the operand is in the address register specified by the register field. After the operand address is used, it is incremented by one, two, or four, depending upon whether the size of the operand is byte (.B), word (.W), or long (.L).
      ```
      MOVE.B (A2)+,D2  Move byte whose address is in A2 to low order byte of D2; increment A2 by 1.
      MOVE.L (A4)+,D3  Move longword whose address in in A4 to D3; increment A4 by 4.
      ```
  4. Indirect with           <absolute>(A<sub>n</sub>)
     displacement (16-bit)   <complex>(A<sub>n</sub>)
     Example:  The address of the operand is the sum of the address in the address register and the sign-extended displacement.
     ```
     AVALUE   EQU 5       AVALUE is equated to 5 (for use in the next instruction).
     CLR.B    AVALUE(A0)  Clear byte whose address is given by adding value of AVALUE (=5) to contents of A0.
     MOVE     #2,10(A2)   Move value 2 to word whose address is given by adding 10 to contents of A2.
     ```
  5. Indirect with index     <absolute>(A<sub>n</sub>, R<sub>i</sub>)
     (16- or 32-bit) plus
     displacement (8-bit)
     Example: The address of the operand is the sum of the address in the address register, the sign-extended displacement, and the contents of the index (A or D) register.
     ```
     ADD    AVALUE(A1,D2),D5 Add to low order word of D5 the word whose address is given by addition of contents of A1, the low order word of index register (D2), and the displacement (AVALUE).
     MOVE.L D5,$20(A2,A3.L)  Move entire contents of D5 to longword whose address is given by addition of contents of A2, contents of entire index register (A3), and the displacement ($20).
     ```
3. Special address
    Special address mode use the effective address register field to specify the special addressing mode instead of a register number.
  1. PC with              <simple>            Expression is an address
     displacement (16-bit)                    (not a displacement) which must
                                              be backward, within current
                                              relocatable section.
                          <absolute>(PC)      Forced PC-relative.
                          <simple>(PC)        Must fit within 16-bit
                          <complex>(PC)       signed field; resolved
                                              at assembly or link time.

                         <absolute>(PC)      Forced PC-relative.
                         <simple>(PC)        Must fit within 16-bit signed
                                             field; resolved at assembly or lint time.
     Example: The address of the operand is the sum of the address in the program conuter and the sign-extended displacement integer. The assembler calculates this sign-extended displacment by subtracting the address of displacment word from the value of the operand field.
     ```
     JMP  TAG(PC)    Force the jump to address TAG to be PC-relative.
     ```

  2. PC with index      <simple>(R<sub>i</sub>) Expression is an address
     (16- or 32-bit) plus                       which must be backward, within
     displacement (8-bit)                       current relocatable section. Also, due
                                                to linker constraints, any odd-addressed
                                                labels, used with externally defined labesl,
                                                will generate a "break to odd address" error.

                        <absolute>(PC, R<sub>i</sub>) Force PC-relative; expression
                        <simple>(PC, R<sub>i</sub>)   expression must be within current
                                                      program section.
     Example: The address is the sum of the address in the PC, the sign-extended displacement value, and the contents of the index (A or D) register.
     ```
     MOVE     T(D2), TABLE      Moves word at loation (T plus contents of D2) to word location defined by TABLE. T must be a relocatable symbol.
     JMP      TABLE(A2.W)       Transfers control to location defined by TABLE plus the lower 16-bit content of A2 with sign extension. TABLE must be a relocatable symbol.
     JMP      TAG(PC,A2.W)      Forces evalucation of TAG to be PC-relative with index.
     ```
      
  3. Absolute           <absolute>              Expression must be forward
     (16- or 32-bit)    <complex>               forward reference or
                        <simple>                not in current program section.

  4. Immediate (8-, 16-, #<absolute>         Due to linker constraints, any odd
     or 32-bit)          #<simple>           addressed labels, used with externally
                         #<complex>          defined labels, used to "break to odd address" error.
     Example: An absolute number may be specified as an operand by immediately preceding a number or expression with an immediate character. The immediate character (#) is used to designate an absolute number other than a displacement or an absolute address.
     ```
     MOVE     #1,D0     Move value 1 to low order word of D0.
     SUB.L    #1,D0     Substract value 1 from the entire contents of D0.
     ```

### Symbols
Symbols recognized by the assembler consist of one or more valid characters, of which the first eight are significant. The first character must be an uppercase letter (A-Z) or a period (.). Each remaning character may be an uppercase letter, a digit (0-9), a dollar sign ($), a period(.), or an underscore (\_). Lowercase letters can also be used at times.

Numbers recognized by the assembler include decimal, hexadecimal, octal, and binary values. Decimal numbers (the default) are specified by a string of decimal digits (0-9); hexadecimal numbers are speficied by a dollar sign ($) followed by a string of hexadecimal digits (0-9, A-F); octal numbers are specified by the commercial "at" sign (@) followed by a string of octal digits (0-7); binary numbers are specified by a percent sign (%) followed by a string of binary digits (0-1).
Examples:
  Decimal - 12345
  Hexadecimal - $12345
  Octal - @12345
  Binary - %10111

Floating-point numbers can also be specified explicitly as a series of hexadecimal digits preceded by a colon (:). This floating-point hex format can be used to exactly represent the mantissa, exponent, and sign bit for a given floating-point number.
Examples:
  Floating-point  - sx.xxxxxxxxxxxxxxxxEsyyy (maximumb size)
      where: s is an optional sign; x and y are decimal digits
      Example: 1234.56E-33
  Floating-point hex - xxxxx...
      where: xxxxx... is a sequence of hex digits (up to 8 digits for .S precision, up to 16 for .D, and up to 24 for .X or .P)

One or more ASCII characters enclosed by apostrophes (') constitute an ASCII string. ASCII strings are left-justified and zero-filled (if necessary), whether stored or used as immediate operands. This left justification is to a word boundary if one or two characters are specified, or to a longword boundary if the string contains more than two characters. (In order to specify an apostrophe within a literal or string, two successive apostrophes must appear where the single apostrophe is intended to appear.)
Examples:
    DC.L 'ABCD'
    DC.L '''79'
    DC.W '*'
    DC.L 'I''M'

### Definitions
Effective Address: The effective address is a term that describes the address of an operand that is stored in memory.<sup>[1](https://study.com/academy/lesson/addressing-modes-definition-types-examples.html)</sup>
