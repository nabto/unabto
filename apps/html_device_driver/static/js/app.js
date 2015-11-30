/*!
 *  JavaScript application using Nabto communication.
 */

// Set the light state if no errors are returned
function setLight(err, data) {
  if (!err) {
    $("#living-room-status").val(data.light_state?"on":"off").slider("refresh");
    $("#light-div-on").stop().fadeTo(1000, data.light_state);
  }
}

$(document).on("pageinit", function() {
  // Initialize Nabto JavaScript library
  jNabto.init({
    debug: false
  });
    
  // Update living room status on startup
  jNabto.request("light_read.json?light_id=1", setLight);
  
  // Bind change event to living room switch
  $("#living-room-status").change(function() {
    var state = $(this).val() === "off"?0:1;
    jNabto.request("light_write.json?light_id=1&light_on=" + state, setLight);
  });
});
