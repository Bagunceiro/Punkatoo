#include "mqtt.h"
#include "infrared.h"
#include "spdt.h"

class Fan: public MqttControlled, IRControlled
{
    /*
       Speed is a number between -3 and +3 where 0 is off and sign = direction
    */
  public:
    Fan(String devName);
    virtual ~Fan();
    void init(const int d1, const int d2, const int s1, const int s2);
    void setSpeed(const int s);
    int8_t getSpeed();
    char* getStatus(char* buff);
    /*
        Converts speed value to positions of the speed switch.
        The important thing is that pos 0 (off) results in maximum speed, the other two
        are interchangeable (but must be consistent!)
    */
    int speedToPos(const int s);
    int posToSpeed(const int p);
    void onoff();
    void faster();
    void slower();
    bool reverse();
    void mqttaction(String topic, String msg);
    virtual void doSubscriptions(PubSubClient& mqttClient);
    virtual void irmsgRecd(uint32_t code);

  private:
    SPDT dir; // Switch to set direction of motor (or off)
    SPDT spd; // Switch to set speed of motor
};
