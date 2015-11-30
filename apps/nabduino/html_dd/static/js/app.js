/*!
 * @file
 * This JavaScript file implements the custom user application
 */

function showWarning(message) {
  if (message) {
    $(".warnings").text(message).show();
  }
  else {
    $(".warnings").show();
  }
}

function queryLed(input) {
  var led = (input.val() === "off")?0:1;
  jNabto.request("led.json?led_ids=" + led, function(err, data) {
    if (!err) {
      input.val(data.led_status?"on":"off").slider("refresh");
    }
  });
}

function queryButton(input) {
  var state = !input.prop("checked");
  input.prop("checked", state).checkboxradio("refresh");
  
  jNabto.request("button_status.json?", function(err, data) {
    if (!err) {
      input.prop("checked", data.button_status?true:false).checkboxradio("refresh");
    }
  });
}

function queryDigital(input) {
  // Register which button was pressed and use it's number
  var nButton = input.attr("id").replace(/\D/g,'');
  
  // Set bits according to buttons
  var ioOut = (($("#digital_level" + nButton).val() === "on")?1:0);
  ioOut += (($("#digital_output" + nButton).prop("checked"))?0:8);
  
  jNabto.request("digital_io.json?io_index=" + nButton + "&io_out=" + ioOut, function(err, data) {
    if (!err) {
      if ($("#digital_input" + nButton).prop("checked")) {
        $("#digital_level" + nButton).val((data.io_status > 15)?"on":"off").slider("refresh");
      }
    }
  });
}

function queryAnalog(input) {
  var nButton = input.attr("id").replace(/\D/g,'');
  jNabto.request("analog.json?ch_index=" + nButton, function(err, data) {
    if (!err) {
      $("#analog_" + nButton).val("Pin " + nButton + " value is: " + data.analog_ch).button("refresh");
    }
  });
}

function queryTemperature(input) {
  jNabto.request("temperature.json?", function(err, data) {
    if (!err) {
      var celcius = data.temperature_celcius;
      var fahrenheit = ((celcius * (9/5)) + 32).toFixed(0);
      input.val("Temperature is: " + celcius + " \u00B0C / " + fahrenheit + " \u00B0F").button("refresh");
    }
  });
}

function queryPWM(input) {
  var pwm_pin = -1;
  
  // Find checked radio button and use the number in it's id
  $('.pwm').each(function() {
    if ($(this).is(':checked')) {
      pwm_pin = $(this).attr('id').replace(/\D/g,'');
      return false;
    }
  });
  
  if (pwm_pin !== -1) {
    jNabto.request("pwm.json?pwm_pin=" + pwm_pin + "&pwm_val=" + $("#pwm_val").val(), function(err, data) {
      if (!err) {
        $("#pwm_enabled").val(((data.pwm_status % 2) > 0)?"on":"off").slider("refresh");
      }
    });
  }
}

/**
 *  Startup functions
 *  Setup and bind html buttons to query send functions
 */

$(document).on("pageinit", "#frontpage", function() {
  jNabto.init({
    debug: false
  });

  $("#led_toggle").change(function() {
    queryLed($(this));
  });
  $("#button_state").click(function() {
    queryButton($(this));
  });
  $(".digital_update").click(function() {
    queryDigital($(this));
  });
  $("#temperature_read").click(function() {
    queryTemperature($(this));
  });
  $(".analog_read").click(function() {
    queryAnalog($(this));
  });
  $("#pwm_set").click(function() {
    queryPWM($(this));
  });
  
  // Clear values when switching channel in pwm (not working on some mobiles)
  $("#pwm_channel").click(function() {
    $("#pwm_enabled").val("off").slider("refresh");
    $("#pwm_val").val("0").slider("refresh");
  });
});
