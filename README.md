# Alacer

A low-latency C++ trading system with lock-free order queuing and Binance execution support (testnet).
 
## Features
 
- **SPSC lock-free queue** — for passing order intents between threads without locks
- **Order book** — processing for market data / backtesting
- **Live execution** — against Binance Spot Testnet (`/api/v3/order`) with HMAC-SHA256 request signing
- **Backtest mode** — for replaying historical data through the same pipeline as live trading


## Status
 
Work in progress — order status tracking, precision/filter handling for price and quantity, and a WebSocket user-data stream for real-time fill notifications are in progress.
