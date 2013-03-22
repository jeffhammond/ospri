program test
    use iso_c_binding
    implicit none

    integer(c_int)       :: i
    integer(c_size_t)    :: j

    do i = 0, 64
        j = 2**i
        print*,'i = ',i,', j = ',j
    end do

end program test
