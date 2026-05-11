"""
Baccarat web bridge.

Serves a single page that connects via WebSocket to a per-session subprocess
running the local ./baccarat binary. The C++ program's stdout streams to the
browser; bets typed in the browser are written to its stdin.

Run:
    pip install -r requirements.txt
    make                      # build ./baccarat first
    uvicorn server:app --reload --port 8000

Then open http://localhost:8000
"""

import asyncio
import logging
from pathlib import Path

from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles

ROOT = Path(__file__).parent.resolve()
BINARY = ROOT / "baccarat"

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
log = logging.getLogger("baccarat")

app = FastAPI()
app.mount("/static", StaticFiles(directory=ROOT / "static"), name="static")


@app.get("/")
async def index():
    return FileResponse(ROOT / "static" / "index.html")


@app.get("/health")
async def health():
    return {"ok": True, "binary_exists": BINARY.is_file()}


@app.websocket("/ws")
async def ws_endpoint(websocket: WebSocket):
    await websocket.accept()

    if not BINARY.is_file():
        await websocket.send_text(
            f"[error] binary not found at {BINARY}. Run `make` first.\n"
        )
        await websocket.close()
        return

    # Per-session config via query params (?decks=6&money=1000)
    decks = websocket.query_params.get("decks", "6")
    money = websocket.query_params.get("money", "1000")

    # Validate so we don't pass shell-ish junk to the binary.
    try:
        int(decks); int(money)
    except ValueError:
        await websocket.send_text("[error] decks and money must be integers.\n")
        await websocket.close()
        return

    log.info("starting baccarat: decks=%s money=%s", decks, money)
    proc = await asyncio.create_subprocess_exec(
        str(BINARY), decks, money,
        stdin=asyncio.subprocess.PIPE,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.STDOUT,
        cwd=str(ROOT),
    )

    async def pump_stdout():
        """Forward subprocess stdout chunks to the browser."""
        assert proc.stdout is not None
        try:
            while True:
                chunk = await proc.stdout.read(1024)
                if not chunk:
                    break
                await websocket.send_text(chunk.decode("utf-8", errors="replace"))
        except Exception as e:
            log.warning("stdout pump ended: %s", e)

    pump_task = asyncio.create_task(pump_stdout())

    try:
        while True:
            msg = await websocket.receive_text()
            if proc.stdin is None or proc.stdin.is_closing():
                break
            proc.stdin.write((msg + "\n").encode())
            await proc.stdin.drain()
            if msg.strip().lower() == "quit":
                # let the binary print its farewell, then close
                await asyncio.sleep(0.1)
                break
    except WebSocketDisconnect:
        log.info("client disconnected")
    finally:
        if proc.returncode is None:
            try:
                proc.terminate()
                await asyncio.wait_for(proc.wait(), timeout=2)
            except asyncio.TimeoutError:
                proc.kill()
                await proc.wait()
        pump_task.cancel()
        try:
            await websocket.close()
        except Exception:
            pass
        log.info("session ended (exit=%s)", proc.returncode)
