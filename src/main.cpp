#include <iostream>
#include "websocket/websocket_server.h"

int main()
{
    const std::string clientId = "mW3e8pxT";
    const std::string clientSecret = "e1Ii3e1asYyBPST_8m_9jSK90SgEaLdwwOK-zefN-s0";


    WebSocketOrderBookServer wsServer(clientId, clientSecret);


    std::thread wsThread([&wsServer]()
    {
        wsServer.run(9002); 
    });

    wsServer.startOrderBookUpdates(35);
    wsThread.join();

    return 0;
}