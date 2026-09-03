program u32asm
    implicit none
    character(len=256) :: input_file, output_file
    character(len=256) :: line
    integer :: arg_count, len_in, ios, unit_in, unit_out, i, ic
    integer(kind=1) :: opcode, operand
    
    arg_count = command_argument_count()
    if (arg_count < 1) then
        print *, "Error: No source file provided."
        print *, "Usage: ./u32asm <source.u32>"
        stop
    end if
    
    call get_command_argument(1, input_file)
    input_file = trim(input_file)
    
    len_in = len_trim(input_file)
    if (len_in > 4 .and. input_file(len_in-3:len_in) == '.u32') then
        output_file = input_file(1:len_in-4) // '.u32bin'
    else
        output_file = trim(input_file) // '.u32bin'
    end if
    
    print *, "[U32 Assembler] Pass 1: Parsing source file -> ", trim(input_file)
    print *, "[U32 Assembler] Pass 2: Generating binary -> ", trim(output_file)
    
    open(newunit=unit_in, file=input_file, status='old', action='read', iostat=ios)
    if (ios /= 0) then
        print *, "[U32 Assembler Error] Cannot open source file: ", trim(input_file)
        stop
    end if
    
    ! Open with access='stream' to avoid Fortran record markers
    open(newunit=unit_out, file=output_file, status='replace', action='write', form='unformatted', access='stream', iostat=ios)
    if (ios /= 0) then
        print *, "[U32 Assembler Error] Failed to create output binary: ", trim(output_file)
        close(unit_in)
        stop
    end if
    
    do
        read(unit_in, '(A)', iostat=ios) line
        if (ios /= 0) exit
        line = adjustl(line)
        
        do i = 1, len_trim(line)
            ic = iachar(line(i:i))
            if (ic >= 97 .and. ic <= 122) then
                line(i:i) = achar(ic - 32)
            end if
        end do
        
        if (len_trim(line) == 0 .or. line(1:1) == '#' .or. index(line, ':') > 0) cycle
        
        if (index(line, 'CLR') > 0) then
            opcode = int(Z'03', kind=1)
            operand = int(Z'01', kind=1)
            write(unit_out) opcode
            write(unit_out) operand
        else if (index(line, 'INC') > 0) then
            opcode = int(Z'01', kind=1)
            operand = int(Z'01', kind=1)
            write(unit_out) opcode
            write(unit_out) operand
        else if (index(line, 'DEC') > 0) then
            opcode = int(Z'02', kind=1)
            operand = int(Z'01', kind=1)
            write(unit_out) opcode
            write(unit_out) operand
        else if (index(line, 'DISPATCH') > 0) then
            opcode = int(Z'06', kind=1)
            operand = int(Z'01', kind=1)
            write(unit_out) opcode
            write(unit_out) operand
        else if (index(line, 'JZ') > 0) then
            opcode = int(Z'07', kind=1)
            operand = int(Z'01', kind=1)
            write(unit_out) opcode
            write(unit_out) operand
        else if (index(line, 'JMP') > 0) then
            opcode = int(Z'08', kind=1)
            operand = int(Z'00', kind=1)
            write(unit_out) opcode
            write(unit_out) operand
        else if (index(line, 'HLT') > 0) then
            opcode = int(Z'05', kind=1)
            operand = int(Z'00', kind=1)
            write(unit_out) opcode
            write(unit_out) operand
        end if
    end do
    
    close(unit_in)
    close(unit_out)
    
    print *, "[U32 Assembler] Complete. Output saved to ", trim(output_file)
end program u32asm
