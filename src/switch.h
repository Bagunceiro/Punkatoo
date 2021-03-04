#pragma once

#include <vector>

class SwitchedDev
{
  public:
  SwitchedDev(const String& i) { id = i; }
  const String getid() { return id; }
  virtual void switchTo(const int state) = 0;
  private:
  String id;
};

class Switch
{
  public:

  Switch(const String& i, const int pin);
  void addDevice(SwitchedDev& d) { switched.push_back(&d);}
  bool poll();

  private:
  String id;
  int spin;
  int switchState;
  int debounce;
  std::vector<SwitchedDev*> switched;
};

class Switches : public P2Task
{
public:
  Switches(std::vector<Switch>* list) : P2Task("switches", 2000) { swlist = list; }
private:
  std::vector<Switch>* swlist;
  bool operator()();
};