; prologue
lis $4
.word 4
sub $29, $30, $4 ; $29 points to bottom of stack frame

sw $1, -4($30) ; store a on stack
sub $30, $30, $4
sw $2, -4($30) ; store b on stack
sub $30, $30, $4
; factor ID
lw $3, -4($29)

; Epilogue
add $30, $29, $4 ; pop everything
jr $31
