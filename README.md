# Deribit Trading System

## Overview

The Deribit Trading System is a high-performance trading application designed to connect with the Deribit Exchange's testnet via WebSocket and API. This project enables real-time market data streaming and supports comprehensive trading operations including order placement, modification, cancellation, and position management.

## Key Features

- **API Integration**: Direct connection to Deribit Testnet API for comprehensive trading operations
- **Order Book Data**: Real-time fetching of order book information for any instrument
- **WebSocket Streaming**: 
  - Real-time market data updates
  - Multiple client support
  - Dynamic subscribe/unsubscribe functionality
- **Performance Optimized**: 
  - Low-latency trading
  - Minimized API response times
  - Efficient WebSocket communication

## Technologies Used

- **Language**: C++ (C++17)
- **Libraries**:
  - WebSocket++: WebSocket connection management
  - nlohmann::json: JSON parsing and formatting
  - libcurl: HTTP request handling
- **Concurrency**: Thread-safe mechanisms using std::mutex and std::lock_guard

## Project Structure

```
/DeribitTradingSystem
├── CMakeLists.txt
├── src/
│   ├── api/
│   │   ├── deribit_api.cpp
│   │   ├── performance_metrics.cpp
│   │   ├── performance_metrics.h
│   ├── websocket/
│   │   ├── websocket_server.cpp
│   │   ├── websocket_server.h
│   ├── utils/
│   │   ├── performance_utils.cpp
│   │   ├── performance_utils.h
│   ├── main.cpp
├── README.md
```

## Prerequisites

Before setting up the project, ensure you have the following dependencies installed:

- WebSocket++
- nlohmann::json
- libcurl

## Installation and Setup

### Step 1: Clone the Repository

```bash
git clone https://github.com/your-username/DeribitTradingSystem.git
cd DeribitTradingSystem
```

### Step 2: Build the Project

```bash
mkdir build
cd build
cmake ..
make
```

### Step 3: Run the Application

```bash
./DeribitTradingSystem
```

## WebSocket Server Usage

### 1. Subscribe to a Symbol

```json
{
  "action": "subscribe",
  "symbol": "BTC-PERPETUAL"
}
```

### 2. Unsubscribe from a Symbol

```json
{
  "action": "unsubscribe",
  "symbol": "BTC-PERPETUAL"
}
```

### 3. Place an Order

```json
{
  "action": "place_order",
  "instrument": "BTC-PERPETUAL",
  "price": 45000.5,
  "amount": 1
}
```

### 4. Modify an Existing Order

```json
{
  "action": "modify_order",
  "order_id": "123456",
  "price": 45500.0,
  "amount": 20
}
```

### 5. Cancel an Order

```json
{
  "action": "cancel_order",
  "order_id": "123456"
}
```

### 6. Get Open Orders

```json
{
  "action": "get_open_orders"
}
```

### 7. Get Positions

```json
{
  "action": "get_positions",
  "instrument": "BTC-PERPETUAL"
}
```

## Performance Analysis

- **WebSocket Propagation Latency**: Consistently under 100 microseconds
- **API Response Time**: Primary bottleneck, typically 5-6 seconds
- **Internal Processing**: Highly efficient, with minimal overhead, difference of time is under 1000 microseconds with respect to API response time
    - **Order Placement Latency**
    - **End-to-End Trading Loop Latency**
    - **Market Data Latency**

## Future Improvements

- Implement asynchronous HTTP requests
- Explore message batching techniques
- Optimize networking and parallel processing
