import os
import asyncio
import asyncudp
from websockets.asyncio.server import serve
import sys
from http.server import SimpleHTTPRequestHandler, HTTPServer
import threading


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory="utils/WebDebug", **kwargs)


def start_http_server():
    server_address = ("", 8000)
    httpd = HTTPServer(server_address, Handler)
    print("HTTP server running on port 8000")
    httpd.serve_forever()


# WebSocket Server
connected_clients = set()
client_lock = asyncio.Lock()

async def websocket_handler(websocket):
    async with client_lock:
        connected_clients.add(websocket)
    print("Websocket client connected")
    try:
        async for message in websocket:
            pass
    finally:
        async with client_lock:
            connected_clients.remove(websocket)


async def handler(websocket):
    while True:
        message = await websocket.recv()
        print(message)


async def broadcast(message):
    if connected_clients:
        async with client_lock:
            for client in connected_clients:
                await client.send(message)
                # print("Should send message " + str(dir(client)))
                # pass


async def start_websocket_server():
    print("Starting websocket handler")
    async with serve(websocket_handler, "localhost", 8765) as server:
        print("WebSocket server running on port 8765")
        await server.serve_forever()


class UdpServerProtocol:
    def __init__(self, loop):
        self.loop = loop

    def datagram_received(self, data, addr):
        messages = data.decode()
        for message in messages.split("\n"):
            m = message.strip()
            if m:
                asyncio.ensure_future(broadcast(message.strip()), loop=self.loop)

    def connection_made(self, transport):
        pass


async def read_stdin():
    reader = asyncio.StreamReader()
    protocol = asyncio.StreamReaderProtocol(reader)
    await asyncio.get_event_loop().connect_read_pipe(lambda: protocol, sys.stdin)
    while True:
        line = (await reader.readline()).decode("ascii").strip()
        if line:
            await broadcast(line)


async def main():
    loop = asyncio.get_event_loop()

    # Listen for UDP packets
    # await loop.create_datagram_endpoint(
    #     lambda: UdpServerProtocol(loop), local_addr=("0.0.0.0", 9999)
    # )

    # Start a HTTP server to serve the local files from
    print("Creating http server")
    threading.Thread(target=start_http_server).start()

    asyncio.gather(
        read_stdin(),
        start_websocket_server()
    )

    while True:
        await asyncio.sleep(0)


if __name__ == "__main__":
    asyncio.run(main())
