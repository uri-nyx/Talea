from sys import argv

def main(argv):

    with open(argv[1], "r") as h:
        hexdump = h.readlines()

    little_endian = ""
    for line in hexdump:
        b0 = line[6:8]
        b1 = line[4:6]
        b2 = line[2:4]
        b3 = line[0:2]

        little_endian += b0 + b1 + b2 + b3 + "\n"

    with open(argv[1], "w") as h:
        h.write(little_endian)

    print("swapped bytes")

main(argv)
