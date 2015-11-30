/*!
 * @file
 * JavaScript and jQuery Mobile defaults
 * Must be loaded before the jQuery Mobile Framework
 */

$(document).bind("mobileinit", function() {
  "use strict";

  $.ajaxSetup({
    cache: false,
    isLocal: true
  });

  // NABTO-1083: jQuery Mobile hash navigation fix on newer browsers
  $.mobile.hashListeningEnabled = false;

  $.mobile.defaultPageTransition = "slide";
  $.mobile.toolbar.prototype.options.addBackBtn = true;
});
