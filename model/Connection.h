/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#ifndef __Connection_h__
#define __Connection_h__


class PeerNode;
class Message;


class Connection
{
  public:

    Connection (PeerNode * homeNode);
    ~Connection ();


    unsigned long  getNodeId ();

    bool  tie (Connection * destEndpoint);

    void  closeConnection ();

    bool  processMsg (const Message * msg);

    bool  sendMsg (const Message * msg);

    long  dataLoss ();
    long  msgLoss ();

    void  reset ();

    
    static unsigned long  globalConnectionCount ();

  private:

    PeerNode *     homeNode_;
    Connection *   destEndpoint_;

    long  dataLoss_;
    long  msgLoss_;


    static unsigned long  globalConnectionCount_s;
};


#endif

