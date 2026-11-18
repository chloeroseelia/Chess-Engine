C:\Users\chloe\emsdk\emsdk_env.ps1
em++ -O2 -std=c++20 -Isrc src/engine.cpp web/bindings.cpp -lembind -sMODULARIZE=1 -sEXPORT_NAME=createChessEngine -sALLOW_MEMORY_GROWTH=1 -o docs/chess.js
