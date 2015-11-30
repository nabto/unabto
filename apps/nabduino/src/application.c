#include <unabto/unabto.h>
#include "hal.h"
#include "application.h"

enum
{
  INFORMATION_FIRMWARE_VERSION
};

enum
{
  QUERY_BUTTON_STATUS = 1,
  QUERY_LED = 2,
  QUERY_DIGITAL_IO = 3,
  QUERY_TEMPERATURE = 5,
  QUERY_ANALOG = 6,
  QUERY_PWM = 7,
  QUERY_ANALOG_RAW = 8,
  QUERY_GET_ALL_ANALOG_INPUTS = 10,
  QUERY_GET_SYSTEM_INFORMATION = 1000
};

application_event_result application_event(application_request* request, buffer_read_t* readBuffer, buffer_write_t* writeBuffer)
{
  NABTO_LOG_TRACE(("app req. local=%i legacy=%i", (int) request->isLocal, (int) request->isLegacy));

  switch(request->queryId)
  {
    case QUERY_BUTTON_STATUS:
    {
      //  <query name="button_status" description="Button" id="1">
      //    <request>
      //    </request>
      //    <response>
      //      <parameter name="button_status" type="uint8"/>
      //    </response>
      //  </query>

      if(!buffer_write_uint8(writeBuffer, buttonRead()))
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_LED:
    {
      //  <query name="led" description="LED" id="2">
      //    <request>
      //      <parameter name="led_ids" type="uint8"/>
      //	  </request>
      //    <response>
      //      <parameter name="led_status" type="uint8"/>
      //    </response>
      //  </query>

      uint8_t led_ids;
      uint8_t led_status;

      if(!buffer_read_uint8(readBuffer, &led_ids)) // not actually used as the Nabduino only has one LED
      {
        return AER_REQ_TOO_SMALL;
      }

      // toggle led
      led_status = !ledRead();
      ledWrite(led_status);

      // status
      if(!buffer_write_uint8(writeBuffer, led_status))
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_DIGITAL_IO:
    {
      //  <query name="digital_io" description="Digital I/O" id="3">
      //    <request>
      //      <parameter name="io_index" type="uint8"/>
      //	    <parameter name="io_out" type="uint8"/>
      //    </request>
      //    <response>
      //      <parameter name="io_status" type="uint8"/>
      //    </response>
      //  </query>

      // Bit definitions shared by io_out and io_status:
      // 76543210
      //        +- Output (0=low 1=high)
      //       +-- Not used
      //      +--- Not used
      //     +---- Direction (0=output 1=input)
      //    +----- Input (0=low 1=high)
      //   +------ Not used
      //  +------- Not used
      // +-------- Not used

      uint8_t io_index;
      uint8_t io_out;

      if(!buffer_read_uint8(readBuffer, &io_index) || !buffer_read_uint8(readBuffer, &io_out))
      {
        return AER_REQ_TOO_SMALL;
      }

      // set direction of the pin
      if(io_out & 0x08)
      {
        pinMode(io_index, INPUT);

        DelayMs(20); // workaround problem with reading back value of IO pin after setting it to input mode.
      }
      else
      {
        pinMode(io_index, OUTPUT);

        // if it's and output also write the value of the pin
        if(io_out & 0x01)
        {
          digitalWrite(io_index, HIGH);
        }
        else
        {
          digitalWrite(io_index, LOW);
        }
      }

      // read back pin value
      if(digitalRead(io_index))
      {
        io_out |= 0x10; // read back high
      }
      else
      {
        io_out &= ~0x10; // read back low
      }

      // status
      if(!buffer_write_uint8(writeBuffer, io_out))
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_TEMPERATURE:
    {
      //  <query name="temperature" description="Temp. sensor" id="5">
      //    <request>
      //    </request>
      //    <response>
      //      <parameter name="temperature_celcius" type="uint32"/>
      //    </response>
      //  </query>

      if(!buffer_write_uint32(writeBuffer, (uint32_t) temperatureRead()))
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_ANALOG:
    {
      //  <query name="analog" description="Analog inputs" id="6">
      //    <request>
      //	    <parameter name="ch_index" type="uint8"/>
      //    </request>
      //    <response>
      //      <parameter name="analog_ch" type="uint32"/>
      //    </response>
      //  </query>

      uint8_t ch_index;
      uint32_t analog_ch;

      // read analog channel number
      if(!buffer_read_uint8(readBuffer, &ch_index))
      {
        return AER_REQ_TOO_SMALL;
      }

      analog_ch = analogRead(ch_index);

      // convert raw AD value to mV compensating for the voltage divider on the analog inputs (Rtop = 1k and Rbottom = 1k8).
      // Scaling is used.
      // (((V_in_max * 256) / 1023) * analog) / 256
      // ((((Vref[mV] * ((Rt+Rb) / Rb)) * 256) / 1023) * analog) / 256
      // ((((Vref[mV] * 1.55556) * 256) / 1023) * analog) / 256
      // (((Vref[mV] * 398) / 1023) * analog) / 256
      analog_ch *= 1285;
      analog_ch /= 256;

      // write analog channel value
      if(!buffer_write_uint32(writeBuffer, analog_ch))
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_PWM:
    {
      //  <query name="pwm" description="PWM Set" id="7">
      //    <request>
      //	    <parameter name="pwm_pin" type="uint8"/>
      //	    <parameter name="pwm_val" type="uint8"/>
      //    </request>
      //    <response>
      //      <parameter name="pwm_status" type="uint8"/>
      //    </response>
      //  </query>

      uint8_t pwm_pin;
      uint8_t pwm_val;
      uint8_t pwm_status;

      if(!buffer_read_uint8(readBuffer, &pwm_pin) || !buffer_read_uint8(readBuffer, &pwm_val))
      {
        return AER_REQ_TOO_SMALL;
      }

      //      pwm_status = set_pwm(pwm_pin, true, pwm_val);
      analogWrite(pwm_pin, pwm_val);
      pwm_status = true; // always pretend it succeeded (the Arduino API does not return a value from the analogWrite() function).

      if(!buffer_write_uint8(writeBuffer, pwm_status))
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_ANALOG_RAW:
    {
      //  <query name="analog_raw" description="Raw analog inputs" id="8">
      //    <request>
      //	    <parameter name="ch_index" type="uint8"/>
      //    </request>
      //    <response>
      //      <parameter name="analog_ch" type="uint32"/>
      //    </response>
      //  </query>

      uint8_t ch_index;
      uint32_t analog_ch;

      // read analog channel number
      if(!buffer_read_uint8(readBuffer, &ch_index))
      {
        return AER_REQ_TOO_SMALL;
      }

      analog_ch = analogRead(ch_index);

      // write analog channel value
      if(!buffer_write_uint32(writeBuffer, analog_ch))
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_GET_ALL_ANALOG_INPUTS:
    {
      //  <query name="get_all_analog_inputs.json" description="Read all analog input channels" id="10">
      //    <request>
      //    </request>
      //    <response format="json">
      //      <parameter name="channel0" type="uint16"/>
      //      <parameter name="channel1" type="uint16"/>
      //      <parameter name="channel2" type="uint16"/>
      //      <parameter name="channel3" type="uint16"/>
      //      <parameter name="channel4" type="uint16"/>
      //      <parameter name="channel5" type="uint16"/>
      //    </response>
      //  </query>

      uint8_t i;

      for(i = 0; i < 6; i++)
      {
        uint32_t value = analogRead(i);

        // convert raw AD value to mV compensating for the voltage divider on the analog inputs (Rtop = 1k and Rbottom = 1k8).
        // Scaling is used.
        // (((V_in_max * 256) / 1023) * analog) / 256
        // ((((Vref[mV] * ((Rt+Rb) / Rb)) * 256) / 1023) * analog) / 256
        // ((((Vref[mV] * 1.55556) * 256) / 1023) * analog) / 256
        // (((Vref[mV] * 398) / 1023) * analog) / 256
        value *= 1285;
        value /= 256;

        // write analog channel value
        if(!buffer_write_uint16(writeBuffer, (uint16_t) value))
        {
          return AER_REQ_RSP_TOO_LARGE;
        }
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_GET_SYSTEM_INFORMATION:
    {
      //  <query name="get_system_information" description="Provides a series of information parameters detailing the system's configuration." id="1000">
      //    <request>
      //    </request>
      //    <response>
      //      <parameter name="parameters" type="raw"/>
      //    </response>
      //  </query>

      uint8_t* lengthOffset = buffer_write_head(writeBuffer);

      if(!buffer_write_uint16(writeBuffer, 0))
      {
        return AER_REQ_RSP_TOO_LARGE; // write dummy length
      }

      // write version information
      if(!buffer_write_uint16(writeBuffer, INFORMATION_FIRMWARE_VERSION) || !buffer_write_uint32(writeBuffer, RELEASE_MAJOR) || !buffer_write_uint32(writeBuffer, RELEASE_MINOR))
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      WRITE_U16(lengthOffset, buffer_write_head(writeBuffer) - lengthOffset - 2); // insert actual length

      return AER_REQ_RESPONSE_READY;
    }
  }

  return AER_REQ_INV_QUERY_ID;
}

void setup(char** url)
{
  // Alternative HTML device driver URL. Use this if you want to create your own Nabduino HTML device driver.
  //  static char urlBuffer[NABTO_URL_OVERRIDE_MAX_SIZE];
  //  strcpypgm2ram(urlBuffer, "http://download.nabduino.com/html_dd.zip"); // the URL for the default Nabduino HTML device driver.
  //  strcpypgm2ram(urlBuffer, "file://c:/test/html_dd.zip"); // It is also possible to specify a file on the client's file system.
  //  *url = urlBuffer;
}

void loop(void)
{
}
