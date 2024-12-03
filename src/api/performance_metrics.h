#ifndef PERFORMANCE_METRICS_H
#define PERFORMANCE_METRICS_H

#include <chrono>
#include <string>
#include <iostream>

class PerformanceMetrics
{
public:
    std::chrono::high_resolution_clock::time_point orderPlacementStart;
    std::chrono::high_resolution_clock::time_point orderPlacementEnd;

    std::chrono::high_resolution_clock::time_point marketDataStart;
    std::chrono::high_resolution_clock::time_point marketDataEnd;

    std::chrono::high_resolution_clock::time_point websocketSendStart;
    std::chrono::high_resolution_clock::time_point websocketSendEnd;

    std::chrono::high_resolution_clock::time_point tradingLoopStart;
    std::chrono::high_resolution_clock::time_point tradingLoopEnd;

    // Measure Order Placement Latency
    void measureOrderPlacementLatency()
    {
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(orderPlacementEnd - orderPlacementStart).count();
        std::cout << "Order Placement Latency: " << latency << " microseconds" << std::endl;
    }

    // Measure Market Data Latency
    void measureMarketDataLatency()
    {
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(marketDataEnd - marketDataStart).count();
        std::cout << "Market Data Latency: " << latency << " microseconds" << std::endl;
    }

    // Measure WebSocket Message Propagation Latency
    void measureWebSocketPropagationLatency()
    {
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(websocketSendEnd - websocketSendStart).count();
        std::cout << "WebSocket Propagation Latency: " << latency << " microseconds" << std::endl;
    }

    // Measure End-to-End Trading Loop Latency
    void measureTradingLoopLatency()
    {
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(tradingLoopEnd - tradingLoopStart).count();
        std::cout << "End-to-End Trading Loop Latency: " << latency << " microseconds" << std::endl;
    }
};

#endif // PERFORMANCE_METRICS_H
