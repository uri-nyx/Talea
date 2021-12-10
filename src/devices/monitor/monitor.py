import os
import sys

class LineMonitor():
    def __init__(self):
        self.quit = False
        self.out_p = "/tmp/from_monitor_fifo"
        self.in_p = "/tmp/to_monitor_fifo"
        
    
    def eval(self, command: str) -> str:
        tokens = command.split()
        
        if len(tokens) > 0:
            order = tokens[0]
        else:
            return " Void --> ok."
        
        if order == "resume":
            return self.resume()
        elif order == "h":
            return self.pause()
        elif order == "s":
            return self.step()
        elif order == "m":
            a, b = self.twoargs(tokens)
            return self.setmem(a, b)
        elif order == "r":
            a, b = self.twoargs(tokens)
            return self.setreg(a, b)
        elif order == "d":
            a, b = self.twoargs(tokens)
            return self.core(a, b)
        elif order == "st":
            return self.stack()
        elif order == "c":
            return self.control()
        elif order == "q":
            self.resume()
            self.quit = True
            return " bye! --> ok."
        
        else:
            return " order `" + order + "` not recognized --> err."
    
    def print(self, msg: str) -> None:
        print(msg)
    
    def read(self) -> str:
        return str(input("lmon> "))
    
    def run(self) -> None:
        self.reset_out()
        while self.quit == False:
            command = self.read()
            self.print(self.eval(command))
            self.reset_out()
            
    def reset_out(self) -> None:
        open(self.out_p, "wb+").close()
    
    def resume(self):
        ordcode = 1
        try:
            with open(self.out_p, "wb+") as out:
                out.write(bytes([ordcode]))
                
            return " Resumed Execution --> ok."
        except Exception as e:
            return str(e) + " --> err."
        
    def pause(self):
        ordcode = 2
        try:
            with open(self.out_p, "wb+") as out:
                out.write(bytes([ordcode]))
                
            return " Paused Execution --> ok."
        except Exception as e:
            return str(e) + " --> err."
    
    def step(self):
        ordcode = 3
        try:
            with open(self.out_p, "wb+") as out:
                out.write(bytes([ordcode]))
                
            return " Executed 1 instruction --> ok."
        except Exception as e:
            return str(e) + " --> err."
        
            
    def setmem(self, addr, value):
        ordcode = 4   
        try:
            with open(self.out_p, "wb+") as out:
                out.write(bytes([ordcode, addr, value]))
                
            return " $" + f'{addr:0>4X}' + " => " + str(value) + " --> ok."
        except Exception as e:
            return str(e) + " --> err."
        
    def setreg(self, addr, value):
        ordcode = 5   
        try:
            with open(self.out_p, "wb+") as out:
                out.write(bytes([ordcode, addr, value]))
                
            return " R::" + get_key(addr, reg_names) + " => " + str(value) + " --> ok."
        except Exception as e:
            return str(e) + " --> err."
        
    def core(self, addr, value):
        ordcode = 6   
        try:
            with open(self.out_p, "wb+") as out:
                out.write(bytes([ordcode, addr, value]))
            
            with open(self.in_p, "rb") as inb:
                size = value - addr
                d = list(inb.read(size))
                inb.close()
            
            table = "@$ x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xa xb xc xd xe xf\n"
            row = addr >> 4
            i = 0
            coredump = d
            for byte in coredump:
                if i == 0:
                    table += f'{row:0>2X}' + " " f'{byte:0>2X}' + " "
                    i += 1
                    
                elif i == 15:
                    table += f'{byte:0>2X}' + " \n"
                    i = 0
                    row += 1
                else:
                    table += f'{byte:0>2X}' + " "
                    i += 1
                    
            return table + " \nDumped Memory: $" + str(addr) + " :: $" + str(value) + " --> ok."
        except Exception as e:
            return str(e) + " --> err."
        
    def stack(self):
        ordcode = 7   
        try:
            with open(self.out_p, "wb+") as out:
                out.write(bytes([ordcode]))
            
            with open(self.in_p, "rb") as inb:
                stack = list(inb.read(256))
            
            table = "@$ x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xa xb xc xd xe xf\n"
            i = 0
            row = 0
            for byte in stack[1:]:
                if i == 0:
                    table += f'{row:0>1X}' + ". " + f'{byte:0>2X}' + " "
                    i += 1
                    
                elif i == 15:
                    table += f'{byte:0>2X}' + " \n"
                    i = 0
                    row += 1
                else:
                    table += f'{byte:0>2X}' + " "
                    i += 1
                    
            return table + "\n Dumped Stack => Stack Pointer  :: " +  f'{stack[0]:0>2X}' + " --> ok."
        except Exception as e:
            return str(e) + " --> err."
    
    def control(self):
        ordcode = 8   
        try:
            with open(self.out_p, "wb+") as out:
                out.write(bytes([ordcode]))
            
            with open(self.in_p, "rb") as inb:
                c = list(inb.read(11))
                
            co = c[:3]
            c= c[3:]
                
            table =  "Acc :: " + f'{c[0]:0>2X}' + " Bcc :: " f'{c[1]:0>2X}' + "\n"
            table += "R1  :: " + f'{c[2]:0>2X}' + " R2  :: " f'{c[3]:0>2X}' + "\n"
            table += "R3  :: " + f'{c[4]:0>2X}' + " R4  :: " f'{c[4]:0>2X}' + "\n"
            table += "Hx  :: " + f'{c[6]:0>2X}' + " Lx  :: " f'{c[7]:0>2X}' + "\n"
            table += "Sc  :: " + f'{co[2]:0>2X}' + " Pc  :: " + f'{co[0]:0>4X}' + "\n"
            table += "Status  :: " + f'{co[1]:0>8b}\n'
            return table + " Dumped Control Panel --> ok."
        except Exception as e:
            return str(e) + " --> err."
    
    def twoargs(self, tokens):
        try:
            if tokens[1] in reg_names:
                addr = reg_names[tokens[1]]
            else:
                addr = int(tokens[1], 0)
            value = int(tokens[2], 0)
            
            return addr, value
        except Exception:
            return " stm addr value --> err."
        
reg_names = {
    "pc" : 8,
    "sc" : 9,
    "status" : 10,
    "acc": 0,
    "bcc": 1,
    "r1" : 2,
    "r2" : 3,
    "r3" : 4,
    "r4" : 5,
    "hx" : 6,
    "lx" : 7
}
    
def get_key(val, dict):
    for key, value in dict.items():
         if val == value:
             return key


def main(argv):
    
    mon = LineMonitor()
    
    if len(argv) > 1:
        command = " ".join(argv[1:])
        mon.print(mon.eval(command))
        mon.reset_out()
    else:
        mon.run()


if __name__ == "__main__":
    
    main(sys.argv)