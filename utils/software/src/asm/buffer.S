# Move input character on port to buffer

m KB_PORT < 0 >

endmacro

init:
	addi s1 , zero , 1024			# Buffer starts at 1024

inc:
	beq t1 , zero , getc 			# do not increment if char was not stored
	addi s1 , s1 , 1 				# add 1 to pointer
	addi t1 , zero , 0				# reset flag to 0, this character has not been stored yet

getc:
	lbc t0 , zero , KB_PORT			# Load the byte from kb input port
	beq t0 , zero , getc			# If it is 0 wait for input

move_to_buffer:
	sb t0 , s1 , 0					# store at s1
	addi t1 , zero , 1  			# Flag that indicates a character was stored
	jal zero , inc					# go to increment
	 
