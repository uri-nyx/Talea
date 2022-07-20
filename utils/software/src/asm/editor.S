# The simplest editor for the Taleä system
# It does not deal with files, but writes sectors instead

m KB_PORT < 0 >
m VIDEO_PORT < 2 >

m CharX++ <
	addi $5 , $5 , 1
>

m CharNewLine <
	andi $5 , $5 , 255; # clear low byte
	addi $5 , $5 , 256  # add 0x0100 to high byte	
>

endmacro

init:
	# Initialize a buffer to move the characters there

	addi s1 , zero , 1024 						# Load pointer in s1
	addi s1 , zero , 1024 					# Start of buffer in s2
	addi t3 , zero , 13				# Newline code for comparation

getc:
	lbc t1 , zero , KB_PORT			# Load from port 0 
	beq t1 , zero , getc			# If equal to 0, return to getc
									# If not 0, store
store:
	sb t1 , s1 , 0                  # Store the caracter at pointer location
	addi s1 , s1 , 1 				# Increment the pointer

display:
	add s3 , s2 , zero 		        # Initialize counter for a loop through buffer at start

display_loop:						# for every character in buffer
	lbu t2 , s3 , 0 				# Load the byte pointed by s3
	beq t2 , zero , getc            # if byte is 0, start polling again
	bne t2 , t3 , print 			# if byte is not 13, print, else, add new line and go to the end of loop
	CharNewLine
	jal zero , endloop

print:
	ori t2 , t2 , 1024 				# Load 0x0400 to t2, command to print character to screen
	shc t2 , zero , VIDEO_PORT		# Send it to video port
	CharX++							# increment x position for character

endloop:
	addi s3 , zero , 1 				# increment pointer in buffer
	jal zero , display_loop			# iterate

	
