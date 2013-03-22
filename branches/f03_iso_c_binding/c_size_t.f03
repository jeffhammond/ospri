program c_size_t
    use iso_c_binding
    implicit none

    integer(C_INT)       :: i
    integer(C_SIZE_T)    :: j

    do i = 0, 100
        j = 2**i
        print*,j
    end do

    return
end program c_size_t
