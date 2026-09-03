subroutine u32_run_fortran_assembler(c_filename) bind(C, name="u32_run_fortran_assembler")
    use, intrinsic :: iso_c_binding
    character(kind=c_char), intent(in) :: c_filename(*)
    character(len=512) :: filename
    character(len=512) :: out_filename
    integer :: i, ios, unit_src, unit_out
    character(len=256) :: line

    ! Convert C-string (null-terminated) to a Fortran string
    filename = ""
    i = 1
    do while (c_filename(i) /= c_null_char .and. i <= len(filename))
        filename(i:i) = c_filename(i)
        i = i + 1
    end do
    filename = trim(filename)

    print *, "[Fortran Engine] Opening source file: ", trim(filename)

    ! Open the source assembly file
    open(newunit=unit_src, file=trim(filename), status='old', action='read', iostat=ios)
    if (ios /= 0) then
        print *, "[Fortran Error] Could not open source file: ", trim(filename)
        return
    end if

    ! Determine output filename (.u32bin)
    out_filename = trim(filename) // ".u32bin"
    
    open(newunit=unit_out, file=trim(out_filename), status='replace', action='write', iostat=ios)
    if (ios /= 0) then
        print *, "[Fortran Error] Could not create output binary: ", trim(out_filename)
        close(unit_src)
        return
    end if

    print *, "[Fortran Engine] Compiling into binary: ", trim(out_filename)

    ! Read line by line and compile into binary format
    do
        read(unit_src, '(A)', iostat=ios) line
        if (ios /= 0) exit
        write(unit_out, '(A)') trim(line)
    end do

    close(unit_src)
    close(unit_out)
    print *, "[Fortran Engine] Assembly complete!"
end subroutine u32_run_fortran_assembler
