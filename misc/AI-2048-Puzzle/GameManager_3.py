from tkinter.ttk import setup_master
from matplotlib.pyplot import grid
from Grid_3       import Grid
from ComputerAI_3 import ComputerAI
from PlayerAI_3   import PlayerAI
from Displayer_3  import Displayer
from random       import randint
import time

from pwn import*
#argv = ['p','/home/sb/Desktop/2048.py']
#sh = process(argv=argv)
sh = remote('202.198.27.90',28177)

defaultInitialTiles = 2
defaultProbability = 0.5

actionDic = {
    0: "w",
    1: "s",
    2: "a",
    3: "d"
}

(PLAYER_TURN, COMPUTER_TURN) = (0, 1)

# Time Limit Before Losing
timeLimit = 0
allowance = 0.05

class GameManager:
    def __init__(self, size = 4):
        self.grid = Grid(size)
        self.possibleNewTiles = [2, 4]
        self.probability = defaultProbability
        self.initTiles  = defaultInitialTiles
        self.computerAI = None
        self.playerAI   = None
        self.displayer  = None
        self.over       = False

    def setComputerAI(self, computerAI):
        self.computerAI = computerAI

    def setPlayerAI(self, playerAI):
        self.playerAI = playerAI

    def setDisplayer(self, displayer):
        self.displayer = displayer

    def updateAlarm(self, currTime):
        if currTime - self.prevTime > timeLimit + allowance:
            pass#self.over = True
        else:
            while time.perf_counter() - self.prevTime < timeLimit + allowance:
                pass

            self.prevTime = time.perf_counter()

    ##在这里改交互
    def setMap(self):
        ##sudo apt-get install python3-tk
        #python3 -m pip install matplotlib
        print(sh.recvuntil(b'--------\n').decode())
        _map = []
        for i in range(4):
            line = []
            l = sh.recvline()
            print(l.decode())
            
            dict = l.split(b'|')[1:-1]
            for n in dict:
                if n.strip(b' ') == b'':
                    line.append(0)
                else:
                    line.append(int(n.lstrip(b' ')))
            print(sh.recvline().decode())
            _map.append(line)
        print(sh.recvuntil(b'd ?').decode())
        print(_map)
        for x in range(4):
            for y in range(4):
                self.grid.map[x][y] = int(_map[x][y])

    def success(self):
        for x in range(4):
            for y in range(4):
                if self.grid.map[x][y] == 2048:
                    return True
        return False

    def start(self):
        payload = ''
        self.setMap()                   #init map
        self.displayer.display(self.grid)
        # Player AI Goes First
        turn = PLAYER_TURN
        maxTile = 0

        self.prevTime = time.perf_counter()

        while not self.isGameOver() and not self.over:
            # Copy to Ensure AI Cannot Change the Real Grid to Cheat
            gridCopy = self.grid.clone()

            move = None

            if turn == PLAYER_TURN:
                print("Player's Turn:", end="")
                move = self.playerAI.getMove(gridCopy)
                #pause()
                sh.sendline(actionDic[move])
                # Validate Move
                if move != None and move >= 0 and move < 4:
                    if self.grid.canMove([move]):
                        self.grid.move(move)

                        # Update maxTile
                        maxTile = self.grid.getMaxTile()
                    else:
                        print("Invalid PlayerAI Move")
                        self.over = True
                else:
                    print("Invalid PlayerAI Move - 1")
                    self.over = True
            else:
                print("Compute's Turn")
                self.setMap()
                self.displayer.display(self.grid)
                
            if self.success() == True:
                sh.interactive()

            # Exceeding the Time Allotted for Any Turn Terminates the Game
            self.updateAlarm(time.perf_counter())
            turn = 1 - turn
            
        print(maxTile)

    def isGameOver(self):
        return not self.grid.canMove()

    def getNewTileValue(self):
        if randint(0,99) < 100 * self.probability:
            return self.possibleNewTiles[0]
        else:
            return self.possibleNewTiles[1];
    
    def insertRandonTile(self):
        tileValue = self.getNewTileValue()
        cells = self.grid.getAvailableCells()
        cell = cells[randint(0, len(cells) - 1)]
        self.grid.setCellValue(cell, tileValue)


def main():
    gameManager = GameManager()
    playerAI  	= PlayerAI()
    computerAI  = ComputerAI()
    displayer 	= Displayer()

    gameManager.setDisplayer(displayer)
    gameManager.setPlayerAI(playerAI)
    gameManager.setComputerAI(computerAI)

    gameManager.start()

if __name__ == '__main__':
    main()
