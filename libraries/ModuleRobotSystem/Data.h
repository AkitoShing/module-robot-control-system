#define MOBILE_START_TRANSMIT '{'
#define ROBOT_START_TRANSMIT '('

#define DATA_TYPE_REQUEST '?'
#define DATA_TYPE_RESPONSE '!'
#define MODULE_DATA_MODULE_ACTION '@'
#define ROBOT_DATA_JOYSTICK_CONTROL '&'

#define MODULE_DATA_MODULE_INFO '#'
#define MODULE_DATA_MODULE_STATUS '$'

#define MODULE_DATA_MODULE_ACTION_ATTACK 'q'
#define MODULE_DATA_MODULE_ACTION_LEFT 'w'
#define MODULE_DATA_MODULE_ACTION_RIGHT 'e'
#define MODULE_DATA_MODULE_ACTION_UP 'r'
#define MODULE_DATA_MODULE_ACTION_DOWN 't'

#define MODULE_DATA_MODULE_INFO_NAME '%'
#define MODULE_DATA_MODULE_INFO_CREATOR '^'

#define MODULE_DATA_MODULE_STATUS_ATTACK_CD 'y'
#define MODULE_DATA_MODULE_STATUS_ATTACK_READY 'u'
#define MODULE_DATA_MODULE_STATUS_MODULE_ENABLE 'i'
#define MODULE_DATA_MODULE_STATUS_MODULE_DISABLE 'o'

#define MOBILE_END_TRANSMIT '}'
#define ROBOT_END_TRANSMIT ')'


//Wire I2C Communication
#define MASTER_ADDRESS 25
#define SLAVE_ADDRESS 42
#define WIRE_DATA_SIZE 2

#define MODULE_ENABLE_PIN 7