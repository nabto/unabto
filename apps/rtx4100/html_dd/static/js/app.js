/*!
 * @file
 * JavaScript application using Nabto
 */

var updateId,
  timeoutId,
  graphCounter = 0,
  graphData1 = [[]],
  graphData2 = [[]],
  graphData3 = [[]],
  plot = 0;

function updateGraph(x,y,z) {
  var newData1 = newData2 = newData3 = [];
  if (!plot) {
    return false;
  }
  
  // Update each data array
  newData1 = graphData1;
  newData1[0].shift();
  newData1[0].push([graphCounter, x]);
  plot.series[0].data = newData1[0];

  newData2 = graphData2;
  newData2[0].shift();
  newData2[0].push([graphCounter, y]);
  plot.series[1].data = newData2[0];

  newData3 = graphData3;
  newData3[0].shift();
  newData3[0].push([graphCounter, z]);
  plot.series[2].data = newData3[0];
  
  // Set x-axis and replot graph
  graphCounter++;
  plot.axes.xaxis.min = graphCounter - 31;
  plot.axes.xaxis.max = graphCounter - 1;
  plot.replot();
}

function queryAcc() {
  jNabto.request("get_acc.json?", function(err, data) {
    if (!err) {
      // 3D vector space with offset - could be divided by 3.3
      var acc_x = ((data.acc_x - 2000) / 1000.0).toFixed(2);
      var acc_y = ((data.acc_y - 2000) / 1000.0).toFixed(2);
      var acc_z = ((data.acc_z - 2000) / 1000.0).toFixed(2);
      updateGraph(acc_x, acc_y, acc_z);
    }
  });
}

function queryMag() {
  jNabto.request("get_mag.json?", function(err, data) {
    if (!err) {
      // Magnetometer direction with offset
      var mag_x = ((data.mag_x - 2000) / 50.0).toFixed(2);
      var mag_y = ((data.mag_y - 2000) / 50.0).toFixed(2);
      var mag_z = ((data.mag_z - 2000) / 50.0).toFixed(2);
      updateGraph(mag_x, mag_y, mag_z);
    }
  });
}

function queryTemp() {
  jNabto.request("get_temp.json?", function(err, data) {
    if (!err) {
      // Temperatures with offset
      var celcius = data.temp - 2000;
      var fahrenheit = ((celcius * (9/5)) + 32).toFixed(0);
      $(".temperature h1").text(celcius + " \u00B0C / " + fahrenheit + " \u00B0F");
    }
  });
}

function queryLed(input, ledId) {
  var led = input.val();
  jNabto.request("set_led.json?led_id=" + ledId + "&led_state=" + led);
}

function queryState() {
  jNabto.request("get_state.json?", function(err, data) {
    if (!err) {
      $("#led_red").val(data.led_red).slider("refresh");
      $("#led_green").val(data.led_green).slider("refresh");
      $("#button_1").val(data.button_1).slider("refresh");
      $("#button_2").val(data.button_2).slider("refresh");
    }
  });
}

function autoUpdate(input) {
  // Clear currect updates and start new update after some time
  clearTimeout(timeoutId);
  clearInterval(updateId);
  
  timeoutId = setTimeout(function() {
    var inputId = input.attr("id");
    if (inputId === "acc_val") {
      updateId = setInterval(queryAcc, $("#acc_val").val());
    }
    else if (inputId === "mag_val") {
      updateId = setInterval(queryMag, $("#mag_val").val());
    }
  }, 2000);
}

function clearGraph() {
  clearInterval(updateId);
  clearTimeout(timeoutId);
  graphData1 = [[]];
  graphData2 = [[]];
  graphData3 = [[]];
  plot = 0;

  // Fill arrays with initial data
  for (graphCounter = 0; graphCounter < 31; graphCounter++) {
    graphData1[0].push([graphCounter, 0]);
    graphData2[0].push([graphCounter, 0]);
    graphData3[0].push([graphCounter, 0]);
  }
  graphCounter = 31;
}

// Initialize graph component
function initGraph(input) {
  var inputId = input.context.id;
  
  var options = {
    seriesDefaults: {
      rendererOptions: {
        smooth: true,
        animation: {
          show: true
        }
      },
      markerOptions: {
        lineWidth: 1.5,
        size: 5
      }
    },
    axes: {
      xaxis: {
        label: "time",
        numberTicks: 11
      },
      yaxis: {
        label: "",
      }
    },
    legend: {
      show: true,
      location: "sw"
    }
  };
  
  // Create graph chart depending on which page is loaded
  if (inputId === "acc-page") {
    options.axes.yaxis.min = -1;
    options.axes.yaxis.max = 1;
    plot = $.jqplot("acc_chart", [graphData1, graphData2, graphData3], options);
    plot.series[0].label = "X-axis";
    plot.series[1].label = "Y-axis";
    plot.series[2].label = "Z-axis";
    queryAcc();
  }
  else if (inputId === "mag-page") {
    options.axes.yaxis.min = -1;
    options.axes.yaxis.max = 1;
    plot = $.jqplot("mag_chart", [graphData1, graphData2, graphData3], options);
    plot.series[0].label = "X-axis";
    plot.series[1].label = "Y-axis";
    plot.series[2].label = "Z-axis";
    queryMag();
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
});

$(document).on("pageshow", "#frontpage", function() {
  clearGraph();
});

$(document).on("pageinit", "#io-page", function() {
  queryTemp();
  
  $(this).on("change", "#led_red", function() {
    queryLed($(this), 0);
  });
  $(this).on("change", "#led_green", function() {
    queryLed($(this), 1);
  });
  $(this).on("tap", "#io_refresh", function() {
    queryTemp();
    queryState($(this));
  });
});

$(document).on("pageinit", "#acc-page", function() {
  $(this).on("change", "#acc_val", function() {
    autoUpdate($(this));
  });
});

$(document).on("pagebeforeshow", "#acc-page", function() {
  initGraph($(this));
  autoUpdate($("#acc_val"));
});

$(document).on("pageinit", "#mag-page", function() {
  $(this).on("change", "#mag_val", function() {
    autoUpdate($(this));
  });
});

$(document).on("pagebeforeshow", "#mag-page", function() {
  initGraph($(this));
  autoUpdate($("#mag_val"));
});
