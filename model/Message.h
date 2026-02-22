/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#ifndef __Message_h__
#define __Message_h__


class Message
{
  public:
 
    Message (int size, int ttl, unsigned long id);
    ~Message ();

    Message (const Message & copy);
    Message & operator = (const Message & copy);

    unsigned long  getId () const;

    int  getSize () const;
    int  getHops () const;
    int  getTtl  () const;

    void decTtl ();

    static  unsigned long  globalMessageCount ();
    static  unsigned long  globalDataCount ();


  private:

    static  void   messageCreated (int dataSize);
    static  unsigned long  messageCount_s;
    static  unsigned long  dataCount_s;

    int            size_;
    int            hops_;
    int            ttl_;
    unsigned long  id_;

};
    

#endif

