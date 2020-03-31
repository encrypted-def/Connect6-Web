#!/usr/bin/env python3

import asyncio
import websockets
import subprocess, sys
import random
import threading
import time
import sqlite3

EMPTY = 0
AI = 1
HUMAN = 2
BLOCK = 3

DEBUG = False
TIMEOUT = 600

def process_send(p, s):
  print(s, file=p.stdin)
  p.stdin.flush()

def process_recv(p):
  return p.stdout.readline().strip()


def isWin(board, player):
  dx = [1,1,1,0]
  dy = [-1,0,1,1]
  for i in range(19):
    for j in range(19):
      for dir in range(4):
        if not (0 <= i+5*dx[dir] <= 18): continue
        if not (0 <= j+5*dy[dir] <= 18): continue        
        if all(board[i+k*dx[dir]][j+k*dy[dir]] == player for k in range(6)):
          return True
  return False

def printBoard(board):
  print("   00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18")
  for i in range(19):
    s = ''
    if i < 10: s += '0'
    s += str(i) + ' '
    for j in range(19):
      if board[i][j] == "EMPTY": s += ".  "
      else: s += str(board[i][j]) + "  "
    print(s)

def mySleep():
  time.sleep(5)
  return 3
async def conn(websocket, path):
  loop = asyncio.get_event_loop()
  if DEBUG: print("connection established..")
  CONNECT6_BINARY = '../cpp-backend/Connect6'
  try:
    p = subprocess.Popen(CONNECT6_BINARY, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)    
  except:
    return None
  while True:
    board = [[EMPTY]*19 for i in range(19)]
    HumanFirst = True
    try:
      client_recv = await asyncio.wait_for(websocket.recv(), timeout=TIMEOUT) # level AI/HUMAN
      if DEBUG: print(client_recv)
      client_recv_chunk = client_recv.split()
      if len(client_recv_chunk) != 2:
        assert(0) #raise SettingError
      if client_recv_chunk[0] not in ['1','2','3','4','5']:
        assert(0) #raise SettingError
      if client_recv_chunk[1] not in ["HUMAN", "AI"]:
        assert(0) #raise WrongFirstPlayerError 
      level = int(client_recv_chunk[0])
      await asyncio.wait_for(loop.run_in_executor(None, process_send, p, "SETTING " + client_recv_chunk[0]), timeout=TIMEOUT)
      HumanFirst = (client_recv_chunk[1] == "HUMAN")
      binary_recv = await asyncio.wait_for(loop.run_in_executor(None, process_recv, p), timeout=TIMEOUT) # BLOCK OK
      if DEBUG: print(binary_recv)
      if binary_recv != "SETTING OK":
        assert(0) #raise SettingError
      cnt = 4
      while cnt: # put 4 blocks
        i = random.randint(0, 18)
        j = random.randint(0, 18)
        if board[i][j] == BLOCK: continue
        cnt -= 1
        board[i][j] = BLOCK
        block_msg = "BLOCK {} {}".format(i,j)
        await asyncio.wait_for(loop.run_in_executor(None, process_send, p, block_msg), timeout=TIMEOUT)
        binary_recv = await asyncio.wait_for(loop.run_in_executor(None, process_recv, p), timeout=TIMEOUT) # BLOCK OK
        if DEBUG: print(binary_recv)
        if binary_recv != "BLOCK OK":
          assert(0) #raise BlockError
        await asyncio.wait_for(websocket.send(block_msg), timeout=TIMEOUT)

      if not HumanFirst:
        if DEBUG: print("MYMOVE 1")
        await asyncio.wait_for(loop.run_in_executor(None, process_send, p, "MYMOVE 1"), timeout=TIMEOUT)
        binary_recv = await asyncio.wait_for(loop.run_in_executor(None, process_recv, p), timeout=TIMEOUT)
        if DEBUG: print(binary_recv)
        _, _, _, x, y = binary_recv.split()
        x = int(x)
        y = int(y)
        if not (0 <= x <= 18 or 0 <= y <= 18): assert(0) #raise OutOfBoundsError
        if board[x][y] != EMPTY: assert(0) #raise NonEmptyError
        board[x][y] = AI
        await asyncio.wait_for(websocket.send("{} {}".format(x,y)), timeout=TIMEOUT)

      winner = EMPTY
      while True:
        ########## HUMAN MOVE ################
        #if DEBUG: printBoard(board)
        # 5 7  / 1 6 3 7
        client_recv = await asyncio.wait_for(websocket.recv(), timeout=TIMEOUT)
        if DEBUG: print(client_recv)
        client_recv_chunk = client_recv.split()
        if len(client_recv_chunk) == 2:
          if not HumanFirst:
            assert(0) # raise WrongHumanMoveNumberError
          HumanFirst = False
        elif len(client_recv_chunk) == 4:
          if HumanFirst:
            assert(0) #raise WrongHumanMoveNumberError
        else:
          assert(0) #raise WrongHumanMoveNumberError
        for i in range(len(client_recv_chunk)//2):
          x = int(client_recv_chunk[2*i])
          y = int(client_recv_chunk[2*i+1])
          if not (0 <= x <= 18 or 0 <= y <= 18): assert(0) #raise OutOfBoundsError
          if board[x][y] != EMPTY: assert(0) #raise NonEmptyError
          board[x][y] = HUMAN

        #if DEBUG: printBoard(board)
        if isWin(board, HUMAN):
          winner = HUMAN
          break

        ########## AI MOVE ###########
        binary_send_chunk = client_recv_chunk[:]
        binary_send = "OPMOVE {} ".format(len(client_recv_chunk)//2) + client_recv
        if DEBUG: print(binary_send)
        await asyncio.wait_for(loop.run_in_executor(None, process_send, p, binary_send), timeout=TIMEOUT)
        binary_recv = await asyncio.wait_for(loop.run_in_executor(None, process_recv, p), timeout=TIMEOUT)
        binary_recv = binary_recv.strip() # BLOCK OK
        if DEBUG: print(binary_recv)
        if binary_recv != "OPMOVE OK": assert(0) #raise OpMoveError
        if DEBUG: print("MYMOVE 2")
        await asyncio.wait_for(loop.run_in_executor(None, process_send, p, "MYMOVE 2"), timeout=TIMEOUT)
        binary_recv = await asyncio.wait_for(loop.run_in_executor(None, process_recv, p), timeout=TIMEOUT)
        if DEBUG: print(binary_recv)
        binary_recv_chunk = binary_recv.split()
        if binary_recv_chunk[0] != "MYMOVE" or binary_recv_chunk[1] != "OK" or binary_recv_chunk[2] != "2" or len(binary_recv_chunk) != 7:
          assert(0) #raise OpMoveError
        for i in range(2):
          x = int(binary_recv_chunk[3+2*i])
          y = int(binary_recv_chunk[4+2*i])
          if not (0 <= x <= 18 or 0 <= y <= 18): assert(0) #raise OutOfBoundsError
          if board[x][y] != EMPTY: assert(0) #raise NonEmptyError
          board[x][y] = AI

        client_send = ' '.join(binary_recv_chunk[3:7]) # x0 y0 x1 y1
        await asyncio.wait_for(websocket.send(client_send), timeout=TIMEOUT)


        #if DEBUG: printBoard(board)
        if isWin(board, AI):
          winner = AI
          break
      try:
        if winner != EMPTY:
          sqlconn = sqlite3.connect('sqlite.db')
          c = sqlconn.cursor()
          if winner == HUMAN:
            c.execute("UPDATE statistic SET cnt = cnt + 1 WHERE level={} AND cwin=0".format(level))
          elif winner == AI:
            c.execute("UPDATE statistic SET cnt = cnt + 1 WHERE level={} AND cwin=1".format(level))
          sqlconn.commit()
          sqlconn.close()
      except:
        pass

    except Exception as e:
      if DEBUG: print("Error : ", e)
      break # connection closed
  try:
    await asyncio.wait_for(loop.run_in_executor(None, process_send, p, "QUIT"), timeout=TIMEOUT) # QUIT
    await asyncio.wait_for(loop.run_in_executor(None, process_recv, p), timeout=TIMEOUT) # OK
    p.wait()
  except:
    pass

start_server = websockets.serve(conn, "0.0.0.0", 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
