/**
@file
Arduino library for Module Robot communication of two arduino board over I2C protocol.
*/
/*

  ModuleRobotControlSystem.h - Arduino library for communicating with Modbus slaves
  over RS232/485 (via RTU protocol).

  Library:: ModuleRobotControlSystem

  Copyright:: 2018-2019 KwokShing Li(AkitoShing), KaKei Chan(Holiday526)

*/

class RobotModule {
  public:
    RobotModule(Wire);

    void begin(uint8_t, uint8_t, TwoWire &wire);
    void Up(void(*)());
    void Down(void(*)());
    void Left(void(*)());
    void Right(void(*)());
    void Action(void(*)());
    void Restore(void(*)());
    void CoolDown(void(*)());

    void SetCoolDownInterval(int)

    void onReceive();
    void send();
  private:
    char* moduleName;
    char* moduleCreator;
    char* coolDownInterval;
}