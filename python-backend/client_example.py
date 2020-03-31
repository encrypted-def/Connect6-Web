#!/usr/bin/env python3

import asyncio
import websockets

import asyncio
import websockets

async def hello():
  uri = "ws://localhost:8765"
  async with websockets.connect(uri) as websocket:    
    while True:
      level = input()
      first = input()
      await websocket.send(level + ' ' + first)
      s = await websocket.recv()
      print(s)
      s = await websocket.recv()
      print(s)
      s = await websocket.recv()
      print(s)
      s = await websocket.recv()
      print(s)
      if first == "AI":
        s = await websocket.recv()
        print(s)
      while True:
        print(" > ", end='')
        ss = input()
        if ss == 'ss': break
        if ss != "ss":
          await websocket.send(ss)

        recv = await websocket.recv()
        print(recv)

asyncio.get_event_loop().run_until_complete(hello())