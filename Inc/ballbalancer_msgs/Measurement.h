#ifndef _ROS_ballbalancer_msgs_Measurement_h
#define _ROS_ballbalancer_msgs_Measurement_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ballbalancer_msgs
{

  class Measurement : public ros::Msg
  {
    public:
      typedef int16_t _raw_x_type;
      _raw_x_type raw_x;
      typedef int16_t _raw_y_type;
      _raw_y_type raw_y;
      typedef int16_t _pos_x_type;
      _pos_x_type pos_x;
      typedef int16_t _pos_y_type;
      _pos_y_type pos_y;
      typedef int16_t _servo_x_type;
      _servo_x_type servo_x;
      typedef int16_t _servo_y_type;
      _servo_y_type servo_y;
      typedef int16_t _target_x_type;
      _target_x_type target_x;
      typedef int16_t _target_y_type;
      _target_y_type target_y;

    Measurement():
      raw_x(0),
      raw_y(0),
      pos_x(0),
      pos_y(0),
      servo_x(0),
      servo_y(0),
      target_x(0),
      target_y(0)
    {
    }

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int16_t real;
        uint16_t base;
      } u_raw_x;
      u_raw_x.real = this->raw_x;
      *(outbuffer + offset + 0) = (u_raw_x.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_raw_x.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->raw_x);
      union {
        int16_t real;
        uint16_t base;
      } u_raw_y;
      u_raw_y.real = this->raw_y;
      *(outbuffer + offset + 0) = (u_raw_y.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_raw_y.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->raw_y);
      union {
        int16_t real;
        uint16_t base;
      } u_pos_x;
      u_pos_x.real = this->pos_x;
      *(outbuffer + offset + 0) = (u_pos_x.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_pos_x.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->pos_x);
      union {
        int16_t real;
        uint16_t base;
      } u_pos_y;
      u_pos_y.real = this->pos_y;
      *(outbuffer + offset + 0) = (u_pos_y.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_pos_y.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->pos_y);
      union {
        int16_t real;
        uint16_t base;
      } u_servo_x;
      u_servo_x.real = this->servo_x;
      *(outbuffer + offset + 0) = (u_servo_x.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_servo_x.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->servo_x);
      union {
        int16_t real;
        uint16_t base;
      } u_servo_y;
      u_servo_y.real = this->servo_y;
      *(outbuffer + offset + 0) = (u_servo_y.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_servo_y.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->servo_y);
      union {
        int16_t real;
        uint16_t base;
      } u_target_x;
      u_target_x.real = this->target_x;
      *(outbuffer + offset + 0) = (u_target_x.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_target_x.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->target_x);
      union {
        int16_t real;
        uint16_t base;
      } u_target_y;
      u_target_y.real = this->target_y;
      *(outbuffer + offset + 0) = (u_target_y.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_target_y.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->target_y);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int16_t real;
        uint16_t base;
      } u_raw_x;
      u_raw_x.base = 0;
      u_raw_x.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_raw_x.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->raw_x = u_raw_x.real;
      offset += sizeof(this->raw_x);
      union {
        int16_t real;
        uint16_t base;
      } u_raw_y;
      u_raw_y.base = 0;
      u_raw_y.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_raw_y.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->raw_y = u_raw_y.real;
      offset += sizeof(this->raw_y);
      union {
        int16_t real;
        uint16_t base;
      } u_pos_x;
      u_pos_x.base = 0;
      u_pos_x.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_pos_x.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->pos_x = u_pos_x.real;
      offset += sizeof(this->pos_x);
      union {
        int16_t real;
        uint16_t base;
      } u_pos_y;
      u_pos_y.base = 0;
      u_pos_y.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_pos_y.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->pos_y = u_pos_y.real;
      offset += sizeof(this->pos_y);
      union {
        int16_t real;
        uint16_t base;
      } u_servo_x;
      u_servo_x.base = 0;
      u_servo_x.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_servo_x.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->servo_x = u_servo_x.real;
      offset += sizeof(this->servo_x);
      union {
        int16_t real;
        uint16_t base;
      } u_servo_y;
      u_servo_y.base = 0;
      u_servo_y.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_servo_y.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->servo_y = u_servo_y.real;
      offset += sizeof(this->servo_y);
      union {
        int16_t real;
        uint16_t base;
      } u_target_x;
      u_target_x.base = 0;
      u_target_x.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_target_x.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->target_x = u_target_x.real;
      offset += sizeof(this->target_x);
      union {
        int16_t real;
        uint16_t base;
      } u_target_y;
      u_target_y.base = 0;
      u_target_y.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_target_y.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->target_y = u_target_y.real;
      offset += sizeof(this->target_y);
     return offset;
    }

    const char * getType(){ return "ballbalancer_msgs/Measurement"; };
    const char * getMD5(){ return "a82ee485c0f8035f04328ec8c4674894"; };

  };

}
#endif
