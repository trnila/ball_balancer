#ifndef _ROS_SERVICE_SetTargetPosition_h
#define _ROS_SERVICE_SetTargetPosition_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ballbalancer_msgs
{

static const char SETTARGETPOSITION[] = "ballbalancer_msgs/SetTargetPosition";

  class SetTargetPositionRequest : public ros::Msg
  {
    public:
      typedef int16_t _x_type;
      _x_type x;
      typedef int16_t _y_type;
      _y_type y;

    SetTargetPositionRequest():
      x(0),
      y(0)
    {
    }

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int16_t real;
        uint16_t base;
      } u_x;
      u_x.real = this->x;
      *(outbuffer + offset + 0) = (u_x.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_x.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->x);
      union {
        int16_t real;
        uint16_t base;
      } u_y;
      u_y.real = this->y;
      *(outbuffer + offset + 0) = (u_y.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_y.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->y);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int16_t real;
        uint16_t base;
      } u_x;
      u_x.base = 0;
      u_x.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_x.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->x = u_x.real;
      offset += sizeof(this->x);
      union {
        int16_t real;
        uint16_t base;
      } u_y;
      u_y.base = 0;
      u_y.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_y.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->y = u_y.real;
      offset += sizeof(this->y);
     return offset;
    }

    const char * getType(){ return SETTARGETPOSITION; };
    const char * getMD5(){ return "6d78a6b8c9650c754bf0432d3d1707c3"; };

  };

  class SetTargetPositionResponse : public ros::Msg
  {
    public:

    SetTargetPositionResponse()
    {
    }

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
     return offset;
    }

    const char * getType(){ return SETTARGETPOSITION; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class SetTargetPosition {
    public:
    typedef SetTargetPositionRequest Request;
    typedef SetTargetPositionResponse Response;
  };

}
#endif
