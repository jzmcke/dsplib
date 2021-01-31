import asyncio
import websockets

async def hello():
    uri = "ws://localhost:8000"
    async with websockets.connect(uri) as websocket:
        name = input("Add a value to transmit:")
        await websocket.send(name)
        print(f"> {name}")

        greeting = await websocket.recv()
        print(f"< {greeting}")
    

asyncio.get_event_loop().run_until_complete(hello())