#pragma once

/**
 * @brief A single pole double throw switch with centre off using two SPST relays
 *
 * The relays are driven active low by the GPIO pins.
 *
 * @todo A parameter to disallow both poles on.
 */
class SPDT
{
public:
  SPDT();

  /**
   * @brief set the GPIO pins
   * @param t1, t2 The GPIO pins driving the two throws
   * @todo Shift to constructor?
   */
  void setPins(int t1, int t2);
  /**
   * @brief Operate the switch
   * @param p Desired switch position 0 = both off, 1 or 2 = appropriate throw on, 3 = both on.
   */
  void pos(const int p);
  /**
   * @brief Report status
   * @returns position status - see pos() for values
   */
  int stat();

private:
  /** @brief GPIO for relay controlling throw 1 */
  int throw1;
  /** @brief GPIO for relay controlling throw 1 */

  int throw2;
};
