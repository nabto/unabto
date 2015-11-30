/**
 * @file
 * JavaScript helper functions for weather station demo
 */
 
function querySpeed(input) {
  jNabto.request("wind_speed.json?", function(err, data) {
    if (!err) {
      input.val("Wind speed: " + data.speed_m_s + " m/s").button("refresh");
    }
  });
}

function queryTemp(input) {
  var temp_sensor = -1;
  
  // Find checked radio button and use the number in it's id
  $('input:radio').each(function() {
    if($(this).is(':checked')) {
      temp_sensor = $(this).attr('id').replace(/\D/g,'');
      return false;
    }
  });
  
  jNabto.request("house_temperature.json?sensor_id=" + temp_sensor, function(err, data) {
    if (!err) {
      input.val("Temperature: " + data.temperature + " \u00B0C").button("refresh");
    }
  });
}

$(document).on("pageinit", function() {
  jNabto.init();
  
  $("#wind_update").click(function() {
    querySpeed($(this));
  });
  $("#temp_update").click(function() {
    queryTemp($(this));
  });
  
  // Clear values when switching sensor
  $("#temp_sensor").click(function() {
    $("#temp_update").val("Read house temperature").button("refresh");
  });
});

