///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


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

