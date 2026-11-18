// engine-worker.js
// The C++ engine lives here, on a background thread, so the page never freezes.
// The page talks to it by sending messages; it answers with messages.

importScripts('chess.js');

let engine = null;

// Everything the page needs to draw the current position
function snapshot() {
  const legal = engine.allLegalMoves();
  return {
    board: engine.getBoard(),
    whiteToMove: engine.whiteToMove(),
    status: engine.gameStatus(),        // "playing", "check", "checkmate", "stalemate"
    evaluation: engine.evaluation(),    // centipawns, White's point of view
    legal: legal ? legal.split(',').map(pair => pair.split('-').map(Number)) : []   // [[from, to], ...]
  };
}

// Load the WebAssembly engine, then tell the page we're ready
createChessEngine().then(e => {
  engine = e;
  engine.newGame();
  postMessage({ type: 'ready', state: snapshot() });
});

// Messages from the page
onmessage = (event) => {
  const msg = event.data;
  if (!engine) return;

  if (msg.type === 'newGame') {
    engine.newGame();
    postMessage({ type: 'state', state: snapshot() });
  }

  if (msg.type === 'playMove') {
    const san = engine.playMove(msg.from, msg.to, msg.promo || '');
    postMessage({ type: 'played', san, from: msg.from, to: msg.to, state: snapshot() });
  }

  if (msg.type === 'engineMove') {
    const start = performance.now();

    // While the engine searches, every move it looks at is sent to the page right away
    const raw = engine.engineMove(msg.depth, san => {
      postMessage({ type: 'considering', san });
    });

    const result = raw ? JSON.parse(raw) : null;
    if (result) result.ms = Math.round(performance.now() - start);
    postMessage({ type: 'engineMoved', result, state: snapshot() });
  }
};