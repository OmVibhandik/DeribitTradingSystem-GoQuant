#ifndef PERFORMANCE_UTILS_H
#define PERFORMANCE_UTILS_H

#include "performance_metrics.h"

// Start timing for order placement
inline void startOrderPlacementTiming(PerformanceMetrics &metrics)
{
    metrics.orderPlacementStart = std::chrono::high_resolution_clock::now();
}

// End timing for order placement
inline void endOrderPlacementTiming(PerformanceMetrics &metrics)
{
    metrics.orderPlacementEnd = std::chrono::high_resolution_clock::now();
}

// Start timing for market data fetching
inline void startMarketDataTiming(PerformanceMetrics &metrics)
{
    metrics.marketDataStart = std::chrono::high_resolution_clock::now();
}

// End timing for market data fetching
inline void endMarketDataTiming(PerformanceMetrics &metrics)
{
    metrics.marketDataEnd = std::chrono::high_resolution_clock::now();
}

// Start timing for WebSocket message send
inline void startWebSocketTiming(PerformanceMetrics &metrics)
{
    metrics.websocketSendStart = std::chrono::high_resolution_clock::now();
}

// End timing for WebSocket message send
inline void endWebSocketTiming(PerformanceMetrics &metrics)
{
    metrics.websocketSendEnd = std::chrono::high_resolution_clock::now();
}

// Start timing for full trading loop
inline void startTradingLoopTiming(PerformanceMetrics &metrics)
{
    metrics.tradingLoopStart = std::chrono::high_resolution_clock::now();
}

// End timing for full trading loop
inline void endTradingLoopTiming(PerformanceMetrics &metrics)
{
    metrics.tradingLoopEnd = std::chrono::high_resolution_clock::now();
}

#endif // PERFORMANCE_UTILS_H
