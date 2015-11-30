/*!
 * @file
 * Nabto JavaScript / jQuery helper functions.
 * www.nabto.com
 */

"use strict";

(function (root, factory) {
  if (typeof define === "function" && define.amd) {
    define(["jquery"], function ($) {
      factory($, root);
      return root.jNabto;
    });
  }
  else {
    factory($, root);
  }
}(window, function($, root) {
  root.notifyCompleted = {};

  if (typeof console === "undefined" || typeof console.log === "undefined") {
    /* global console:true */
    console = {};
    console.log = console.dir = console.error = function() {};
  }

  // Disable jQuery AJAX caching
  $.ajaxSetup({
    cache: false,
    isLocal: true
  });

  if (typeof define === "function") {
    $(document).bind("mobileinit", function() {
      // Set jQuery Mobile defaults
      $.mobile.defaultPageTransition = "slide";
      $.mobile.toolbar.prototype.options.addBackBtn = true;

      // Do not automatically init page when loaded with AMD
      $.mobile.autoInitializePage = false;
    });
  }

  root.jNabto = (function($)
  {
    var nabtoInfo = {},
      initialized = false,
      regInfos = 0,
      alertTimer,
      host = "";

    /**
     *  Default jNabto options
     */
    var options = {
      // Send debug information to browser console or client device
      debug: false,
      // Request Client and uNabto information at startup
      readInfo: false,
      // Use ajaxQueue to avoid concurrent requests
      queueRequests: true,
      // Redirect subpage refresh
      subpageRedirect: false,
      // Load helper scripts with ajax after init
      loadScripts: false,
      // Using native notify for Nabto communication (auto-detected)
      usingNativeNotify: false,
      // Callback function with nabto info when fully initialized
      startup: function(info) {}
    };

    function startup() {
      // Redirect browser logging to 'debug' on iOS
      if (navigator.userAgent.match(/(iPad|iPhone|iPod)/g)) {
        console.log = console.dir = console.error = function(msg) {
          var xhr = new XMLHttpRequest();
          xhr.open('GET', "http://debug/" + encodeURIComponent(msg));
          xhr.send(null);
        };
      }

      // Check to see if window.external is implemented
      try {
        var message = {
          method: "log",
          parameters: ["Using native notify method"]
        };
        window.external.notify(JSON.stringify(message));
        options.usingNativeNotify = true;

        console.log = console.dir = console.error = function(msg) {
          var message = {
            method: "log",
            parameters: [msg]
          };
          window.external.notify(JSON.stringify(message));
        };
      }
      catch (e) {}
    }

    function initAJAXQueue() {
      // This is our queue object
      var ajaxQueue = $({});

      $.ajaxQueue = function(ajaxOpts) {
        var jqXHR, doRequest,
          dfd = $.Deferred(),
          promise = dfd.promise(),
          nativeSuccess = ajaxOpts.success;

        // Run query for native notify
        if (options.usingNativeNotify) {
          doRequest = function(next) {
            root.notifyCompleted = function(r) {
              var response = $.parseJSON(r);
              nativeSuccess(response);
              dfd.resolve(response);
              next();
            };

            var message = {
              method: "fetchUrl",
              parameters: [ajaxOpts.url],
              success: ajaxOpts.success,
              callback: "notifyCompleted",
              id: "1"
            };
            window.external.notify(JSON.stringify(message));
          };
        }
        // Run query for normal AJAX request
        else {
          doRequest = function(next) {
            jqXHR = $.ajax(ajaxOpts);
            jqXHR.done(dfd.resolve)
              .fail(dfd.reject)
              .then(next, next);
          };
        }

        // Queue our request
        ajaxQueue.queue(doRequest);

        return promise;
      };

      // Clear queue
      $.ajaxQueue.clear = function() {
        var n = ajaxQueue.queue().length;
        ajaxQueue.queue().splice(1, n-1);
      };

      // Get size of queue
      $.ajaxQueue.size = function() {
        return ajaxQueue.queue().length;
      };
    }

    function showError(message) {
      clearTimeout(alertTimer);
      $(".errors").text(message || "Error").fadeIn();
      alertTimer = setTimeout(function() {
        $(".errors").fadeOut();
      }, 3000);
    }

    function request(url, opts) {
      opts = opts || {};

      if (typeof url === "object") {
        opts = url;
        url = undefined;
      }

      var callback = opts.callback || function() {};
      if (typeof opts === "function") {
        callback = opts;
      }

      url = url || opts.url;
      if (opts.data) {
        url += '?json={"request":' + JSON.stringify(opts.data) + '}';
      }

      options.debug && console.log("Query for: " + url);

      $.ajaxQueue({
        url: url,
        timeout: 15000,
        dataType: "json",
        success: function(data) {
          options.debug && console.dir(data);

          // Handle nabto errors
          if (!data.response) {
            var errText = "Communication error";
            if (data.error) {
              var appError = "NP_E_";
              var index = data.error.detail && data.error.detail.indexOf(appError);
              errText = index === 0 ? "Application error: " + data.error.detail.substring(appError.length) : "Communication error: " + data.error.header;
            }
            showError(errText);
            return callback(new Error(errText));
          }
          callback(null, data.response);
        },
        error: function(err) {
          showError("Communication error: " + err.statusText);
          callback(new Error(err.statusText));
        }
      });
    }

    function appendInfo(info) {
      $("#admin-panel .ui-panel-inner").append("<p>" + info + "</p>");
    }

    function showInfo() {
      options.debug && console.log(nabtoInfo);

      if (nabtoInfo.client_major && nabtoInfo.client_minor) {
        appendInfo("Client v. "+nabtoInfo.client_major+"."+nabtoInfo.client_minor);
      }
      if (nabtoInfo.html_dd) {
        appendInfo("HTML v. " + nabtoInfo.html_dd);
      }
      if (nabtoInfo.unabto_version) {
        appendInfo("uNabto v. " + nabtoInfo.unabto_version);
      }
      if (nabtoInfo.application_version) {
        appendInfo("uNabto App v. " + nabtoInfo.application_version);
      }
      if (nabtoInfo.hardware_version) {
        appendInfo("Hardware v. " + nabtoInfo.hardware_version);
      }
      if (nabtoInfo.email) {
        appendInfo(nabtoInfo.email);
      }
    }

    function parseInfo(info) {
      var infos = info.keyValue.split("=");
      if (infos.length > 1) {
        nabtoInfo[infos[0]] = infos[1];
      }
    }

    function getGatewayInfo(index) {
      request("get_gateway_information.json?startIndex=" + index, function(err, data) {
        if (!err) {
          regInfos += data.info.length;

          $.each(data.info, function() {
            parseInfo(this);
          });

          if (data.totalNumberOfInfos > regInfos) {
            getGatewayInfo(regInfos);
          }
          else {
            options.debug && console.log("Nabto JavaScript initialized");
            options.startup(nabtoInfo);
            showInfo();
          }
        }
      });
    }

    function getNabtoInfo() {
      $.ajax({
        url: "version.txt",
        dataType: "text",
        success: function(data) {
          if (data.length < 12) {
            nabtoInfo.html_dd = data;
          }
        }
      });

      request("get_nabto_information.json?startIndex=0", function(err, data) {
        if (!err) {
          $.extend(nabtoInfo, data);
        }
      });
    }

    function loadScripts() {
      var scriptsList = ["libs/users.js", "libs/config.js", "libs/control.js"];
      $.each(scriptsList, function() {
        var script = this;
        $.ajax({
          url: script,
          dataType: "script",
          error: function(error) {
            console.log("Could not load script: " + script);
          }
        });
      });
    }

    function historyAdd(_host) {
      var hostString = "history_add?host=" + host;
      if (options.usingNativeNotify) {
        var message = {
          method: "fetchUrlSelf",
          parameters: [hostString],
          id: "2"
        };
        window.external.notify(JSON.stringify(message));
      }
      else {
        $("body").append('<img id="history-link" src="//self/' + hostString + '" style="display: none;"/>');
      }
    }

    function init(opt) {
      if (initialized) { return true; }
      initialized = true;

      $.extend(options, opt);

			// Navigate to root if refreshing from subpage
      if (options.subpageRedirect && window.location.href.indexOf("#") !== -1) {
        window.location.href = "/";
      }

			// Populate device name div
			var $name = $(".device-id");
			if ($name) {
				$name.text(location.host);
			}

			// Initialize AJAX queue
      if (options.queueRequests || options.usingNativeNotify) {
        initAJAXQueue();
      }
      else {
        $.ajaxQueue = $.ajax;
      }

			// Load external scripts
      if (options.loadScripts) {
        loadScripts();
      }

			// Read device info
      if (options.readInfo) {
        regInfos = 0;
        getNabtoInfo();
        getGatewayInfo(regInfos);
      }

      // Most secure way to get host
      host = window.location.href.split("//")[1];
      if (host.indexOf("/") !== -1) {
        host = host.split("/")[0];
      }

      // Add device to history and set toolbar buttons
      historyAdd(host);
      if ($(window).width() > 500) {
        $("#discover-button").removeClass("ui-btn-icon-notext").addClass("ui-btn-icon-left");
        $("#panel-button").removeClass("ui-btn-icon-notext").addClass("ui-btn-icon-right");
      }
    }

    startup();

    return {
      /**
       *  Initialise the jQuery Nabto library with options
       *  @param opt       : object with options that differs from default
       */
      init: function(opt) {
        init(opt);
      },
      /**
       *  Show login client dialog
       */
      showLogin: function() {
        window.location = "//self/login/form?url=" + host;
      },
      /**
       * Show custom error message
       * @param message    : message to show in error popup
       */
      showError: function(message) {
        showError(message);
      },
      /**
       *  Send query to uNabto device and receive response with a callback
       *  @param url       : string holding the query with parameters
       *  @param callback  : callback on success or error
       *  or
       *  @param options   : object containing all options
       */
      request: function(url, callback) {
        request(url, callback);
      },
      /**
       *  Get Client and uNabto information if options.readInfo is set
       */
      getInfo: function() {
        return nabtoInfo;
      },
      /**
       *  Get jNabto options
       */
      getOptions: function() {
        return options;
      },
      /**
       *  Toggle jNabto debugging
       *  jNabto.debug() can be called on-the-fly from the browser console
       */
      debug: function() {
        options.debug = !options.debug;
        return options.debug;
      }
    };
  })($); /* jNabto */

  $(document).on("pageinit", "#frontpage", function() {
    $(this).on("tap", "#panel-button", function(e) {
      e.preventDefault();
      e.stopImmediatePropagation();
      $("#admin-panel").panel("toggle");
    });
    $(this).on("tap", "#panel-login", function() {
      jNabto.showLogin();
    });
  });

  return jNabto;
}));
