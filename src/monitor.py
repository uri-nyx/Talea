from textual.app import App
from textual.widgets import Placeholder


class TuiMonitor(App):

    async def on_mount(self) -> None:
        await self.view.dock(Placeholder(), edge="left", size=16)
        await self.view.dock(Placeholder(), Placeholder(), edge="top")

class LineMonitor():
    def __init__(self):
        self.quit = False
    
    def run(self):
        pass
    
    def eval(self, command: str) -> str:
        tokens = command.split()
        
        if len(tokens) > 0:
            order = tokens[0]
        else:
            return " Void --> ok."
        
        if order == "stm":
            try:
                addr = int(tokens[1], 0)
                value = int(tokens[2], 0)
            except Exception:
                return " stm addr value --> err."
            
            try:
                # memory[addr] = value
                return " $" + f'{addr:0>4X}' + " => " + str(value) + " --> ok."
            except Exception as e:
                return str(e) + " --> err."
        
        elif order == "q":
            self.quit = True
            return " bye! --> ok."
        
        else:
            return " order `" + order + "` not recognized --> err."
    
    def print(self, msg: str) -> None:
        print(msg)
    
    def read(self) -> str:
        return str(input("lmon> "))
    
    def run(self) -> None:
        
        while self.quit == False:
            command = self.read()
            self.print(self.eval(command))
        
        



def Tui():
    TuiMonitor.run(log="textual.log")

def Line():
    LineMonitor().run()
    
Line()