# This is for instruction sets of type BASE_REDCODE _only_
SET_TYPE  BASE_REDCODE

# Traditional Redcode instructions.  There are all used in the form of:
#   <instruction> <A> <B>
#
# Note that both operands are not always used.

dat 1    # Place for storing data; kill any thread which executes it.
mov 1    # Copy A to B.
add 1    # B = A + B
sub 1    # B = A - B
jmp 1    # IP = A
jmz 1    # if (B == 0) IP = A
jmn 1    # if (B != 0) IP = A
djn 1    # B-- ; if (B != 0) IP = A
cmp 1    # if (A == B) IP++
spl   0  # start a new thread up at location 'A'. (multi-threaded runs only)
slt 1    # if (A < B) IP++


# Avida-specific Instructions...

mal 1    # Allocate 'A' lines of extra space
div 1    # Split off child at 'A'
get 1
put 1
nand 1

# When using arguments, those without prefixes are direct values.
#  '@' indicates memory, '$' indirect memory, '>' Pre-decremented indirect.
#
# Thus, "mov 13 @4" would mean copy the number '13' into line 4 of memory.
#
# While, "mov 13 $4" would copy '13' into the memory *pointed to* by line 4.
#
# And "mov 13 >4" does the same, but first decrements the value already at
# that line (not very useful since, in this case, it gets copied over, but
# its more useful in other situations....)
#
# Finally "move 13 4" is an error...

