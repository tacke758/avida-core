# No-ops
nop-A    1
nop-B    1
nop-C    1
nop-X    0

# Flow control operations
if-equ-0 0
if-not-0 0
if-n-equ 1
if-not-0 0
if-n-equ 0
if-grt-0 0
if-grt   0
if->=-0  0
if->=    0
if-les-0 0
if-less  0
if-<=-0  0
if-<=    0
if-bit-1 0

jump-f   0
jump-b   0
jump-p   0
jump-slf 0
call     0
return   0	

# Direct matching
dm-jp-f  0
dm-jp-b  0

# Relative Address
re-jp-f  1
re-jp-b  1

# Absolute Address
abs-jp   0

# Stack and Registrater Operations
push     1
pop      1
flip-stk 0
swap     0
reset	 0
copy-reg 0

# Single Argument Math
shift-r  0
shift-l  0
bit-1    0
set-num  0
inc      1
dec      1
zero	 0	# Can use put instead.
neg	 0
square	 0
sqrt	 0
not	 0

# Double Argument Math
add      1
sub      1
mult	 0
div	 0
mod	 0
nand     1
nor      0
order    0
and      0
xor	 0

# Biological Operations
copy     1
read	 0
write	 0
allocate 1
divide   1
compare  0
if-n-cpy 0
c-alloc  0
c-divide 0
inject	 0

# I/O and Sensory
get      1
stk-get  0
put      1
search-f 1
search-b 1
dm-sch-f 0
dm-sch-b 0

# Other Commands
rotate-l 0
rotate-r 0
set-cmut 0
mod-cmut 0

kill-th  0
fork-th  0
