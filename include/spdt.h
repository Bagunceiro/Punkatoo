#ifndef FC_SPDT_H
#define FC_SPDT_H

class SPDT
{
    /*
       Implements a single pole double throw switch with centre off
       using two SPST relays driven by the GPIO pins defined in
       the construction.
       Relay coils are active low. Both energised at the same time is permitted
    */
  public:
    SPDT();
    void setPins(int t1, int t2);  // GPIO pins driving the two throws
    void pos(const int p) ;
    int stat();

  private:
    int throw1;
    int throw2;
};

#endif
