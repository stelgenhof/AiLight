/**
 * Ai-Thinker RGBW Light Firmware
 *
 * This file is part of the Ai-Thinker RGBW Light Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.

 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2017 Sacha Telgenhof
 */

/* jshint curly: true, undef: true, unused: true, eqeqeq: true, esversion: 6, varstmt: true, browser: true, devel: true */

// Key names as used internally and in Home Assistant
const K_S = "state";
const K_BR = "brightness";
const K_CT = "color_temp";
const K_C = "color";
const K_R = "r";
const K_G = "g";
const K_B = "b";
const K_W = "white_value";
const K_GM = "gamma";

const S_ON = 'ON';
const S_OFF = 'OFF';

const WAIT = 15000;

var hS = false;

/**
 * Object representing a Switch component
 *
 * @param id the DOM element to be rendered as a Switch component
 *
 * @return void
 */
function Switch(id) {
  this.id = id;
  this.init();
}

(function() {
  this.getState = function() {
    return this.state;
  };

  this.setState = function(state) {
    const CLASS_CHECKED = 'checked';
    this.state = state;
    this.el.checked = this.state;

    if (this.el.checked) {
      this.el.parentNode.classList.add(CLASS_CHECKED);
    } else {
      this.el.parentNode.classList.remove(CLASS_CHECKED);
    }
  };

  this.toggleState = function() {
    this.state = !this.state;
    this.setState(this.state);

    var state = {};
    var value = this.state;
    if (this.id === 'state') {
      value = (this.state) ? S_ON : S_OFF;
    }
    state[this.id] = value;

    websock.send(JSON.stringify(state));
  };

  this.init = function() {
    this.el = document.getElementById("switch_" + this.id);
    this.state = this.el.checked;
    this.el.addEventListener("click", this.toggleState.bind(this), {
      passive: true
    });
  };

}).call(Switch.prototype);

/**
 * Object representing a Slider component
 *
 * @param id the DOM element to be rendered as a Slider component
 *
 * @return void
 */
function Slider(id) {
  this.id = id;
  this._init();
}

(function() {
  this.getValue = function() {
    return this.el.value;
  };

  this.setValue = function(val) {
    this.el.value = val;
    this._sethigh();
  };

  this._sethigh = function() {
    this._high = (this.el.value - this.el.min) / (this.el.max - this.el.min) * 100 + '%';
    this.el.style.setProperty('--high', this._high);

    var output = this.el.parentNode.getElementsByTagName('output')[0];
    if (typeof(output) !== "undefined") {
      output.innerHTML = this.el.value;
    }

  };

  this._send = function() {
    var msg = {
      'state': S_ON
    };
    msg[this.id] = this.el.value;

    websock.send(JSON.stringify(msg));
  };

  this._init = function() {
    this.el = document.getElementById("slider_" + this.id);
    this.el.style.setProperty('--low', '0%');
    this._sethigh();

    this.el.addEventListener("mousemove", this._sethigh.bind(this), {
      passive: true
    });
    this.el.addEventListener("touchmove", this._sethigh.bind(this), {
      passive: true
    });
    this.el.addEventListener("drag", this._sethigh.bind(this), {
      passive: true
    });
    this.el.addEventListener("click", this._sethigh.bind(this), {
      passive: true
    });

    var rgb = [K_R, K_G, K_B];
    if (rgb.includes(this.id)) {
      this.el.addEventListener("change", sendRGB.bind(this), {
        passive: true
      });
    } else {
      this.el.addEventListener("change", this._send.bind(this), {
        passive: true
      });
    }
  };
}).call(Slider.prototype);

// Globals
var websock;
var stSwitch = new Switch(K_S);
var brSlider = new Slider(K_BR);
var ctSlider = new Slider(K_CT);
var rSlider = new Slider(K_R);
var gSlider = new Slider(K_G);
var bSlider = new Slider(K_B);
var wSlider = new Slider(K_W);
var gmSwitch = new Switch(K_GM);

/**
 * Sends the RGB triplet state to the connected WebSocket clients
 *
 * @return void
 */
function sendRGB() {
  var msg = {
    'state': S_ON
  };

  msg[K_C] = {};
  msg[K_C][rSlider.id] = rSlider.getValue();
  msg[K_C][gSlider.id] = gSlider.getValue();
  msg[K_C][bSlider.id] = bSlider.getValue();

  websock.send(JSON.stringify(msg));
}

/**
 * Parses a text string to a JSON representation
 *
 * @param str text string to be parsed
 *
 * @return mixed JSON structure when succesful; false when not
 */
function getJSON(str) {
  try {
    return JSON.parse(str);
  } catch (e) {
    return false;
  }
}

/**
 * Process incoming data from the WebSocket connection
 *
 * @param data received data structure from the WebSocket connection
 *
 * @return void
 */
function processData(data) {
  for (var key in data) {

    // Process Device information
    if (key === 'd') {
      document.title = data.d.app_name;

      // Bind data to DOM
      for (var dev in data[key]) {
        // Bind to span elements
        var d = document.querySelectorAll("span[data-s='" + dev + "']");
        [].forEach.call(d, function(item) {
          item.innerHTML = data[key][dev];
        });
      }
    }

    // Process settings
    if (key === 's') {
      document.title += ' - ' + data[key].hostname;

      // Bind data to DOM
      for (var s in data[key]) {
        // Bind to span elements
        var a = document.getElementById("pagescontent").querySelectorAll("span[data-s='" + s + "']");
        [].forEach.call(a, function(item) {
          item.innerHTML = data[key][s];
        });

        // Bind to specific DOM elements
        if (document.getElementById(s) !== null) {
          document.getElementById(s).value = data[key][s];
        }
      }
    }

    // Set state
    if (key === K_S) {
      stSwitch.setState((data[key] === S_OFF) ? false : true);
    }

    if (key === K_BR) {
      brSlider.setValue(data[key]);
    }

    if (key === K_CT) {
      ctSlider.setValue(data[key]);
    }

    if (key === 'color') {
      rSlider.setValue(data[key][K_R]);
      gSlider.setValue(data[key][K_G]);
      bSlider.setValue(data[key][K_B]);
    }

    if (key === K_W) {
      wSlider.setValue(data[key]);
    }

    if (key === K_GM) {
      gmSwitch.setState(data[key]);
    }
  }
}

/**
 * WebSocket client initialization and event processing
 *
 * @return void
 */
function wsConnect() {
  var host = window.location.hostname;
  var port = location.port;

  if (websock) {
    websock.close();
  }

  websock = new WebSocket('ws://' + host + ':' + port + '/ws');

  websock.onopen = function(e) {
    console.log('[WEBSOCKET] Connected to ' + e.target.url);
  };

  websock.onclose = function(e) {
    console.log('[WEBSOCKET] Connection closed');
    console.log(e);
    console.log(e.reason);
  };

  websock.onerror = function(e) {
    console.log('[WEBSOCKET] Error: ' + e);
  };

  websock.onmessage = function(e) {
    var data = getJSON(e.data);
    if (data) {
      processData(data);
    }
  };
}

/**
 * EventSource client initialization and event processing
 *
 * @return void
 */
function esConnect() {
  if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('open', function(e) {
      console.log('[EVENTSOURCE] Connected to ' + e.target.url);
    }, false);

    source.addEventListener('error', function(e) {
      if (e.target.readyState !== EventSource.OPEN) {
        console.log('[EVENTSOURCE] Connection closed');
      }
    }, false);

    source.addEventListener('message', function(e) {
      console.log("message", e.data);
    }, false);

    // Handling OTA events
    source.addEventListener('ota', function(e) {
      if (e.data.startsWith("p-")) {
        var pb = document.getElementById("op");
        var p = parseInt(e.data.split("-")[1]);

        pb.value = p;

        if (p === 100 && !hS) {
          hS = true;
          var f = document.createElement('p');
          f.innerHTML = "Completed successfully! Please wait for your Ai-Thinker RGBW Light to be restarted.";
          pb.parentNode.appendChild(f);
          reload(false);
        }
      }

      // Show OTA Modal
      if (e.data === 'start') {
        document.getElementById("om").classList.add("is-active");
      }
    }, false);
  }
}

/**
 * Reloads the page after waiting certain time.
 *
 * The time before the page is being reloaded is defined by the 'WAIT' constant.
 *
 * @param show Indicates whether a modal windows needs to be displayed or not.
 *
 * @return void
 */
function reload(show) {
  if (show) {
    document.getElementById("rm").classList.add("is-active");
  }

  setTimeout(function() {
    location.reload(true);
  }, WAIT);
}

/**
 * Handler for the Restart button
 *
 * @return bool true when user approves, false otherwise
 */
function restart() {
  var response = window.confirm("Are you sure you want to restart your Ai-Thinker RGBW Light?");
  if (response === false) {
    return false;
  }

  websock.send(JSON.stringify({
    'command': 'restart'
  }));

  // Wait for the device to have restarted before reloading the page
  reload(true);
}

/**
 * Handler for the Reset button
 *
 * @return bool true when user approves, false otherwise
 */
function reset() {
  var response = window.confirm("You are about to reset your Ai-Thinker RGBW Light to the factory defaults!\n Are you sure you want to reset?");
  if (response === false) {
    return false;
  }

  websock.send(JSON.stringify({
    'command': 'reset'
  }));

  // Wait for the device to have restarted before reloading the page
  reload(true);
}

/**
 * Handler for making the password (in)visible
 *
 * @return void
 */
function togglePassword() {
  var ie = document.getElementById(this.dataset.input);
  ie.type = (ie.type === "text") ? "password" : "text";
}

/**
 * Adds validation message to the selected element
 *
 * @param el the DOM element to which the validation message needs to be added
 * @param message the validation message to be diplayed
 *
 * @return void
 */
function addValidationMessage(el, message) {
  const CLASS_WARNING = 'is-danger';
  var v = document.createElement('p');
  v.innerHTML = message;
  v.classList.add("help", CLASS_WARNING);
  el.parentNode.appendChild(v);
  el.classList.add(CLASS_WARNING);
}

/**
 * Handler for validating and saving user defined settings
 *
 * @return void
 */
function save() {
  var s = {};
  var msg = {};
  var isValid = true;

  var Valid952HostnameRegex = /^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$/i;

  var inputs = document.forms[0].querySelectorAll("input");
  for (var i = 0; i < inputs.length; i++) {
    t = inputs[i].id.split('.');
    id = t.pop();

    // Clear any validation messages
    inputs[i].classList.remove("is-danger");
    var p = inputs[i].parentNode;
    var v = p.querySelectorAll("p.is-danger");
    [].forEach.call(v, function(item) {
      p.removeChild(item);
    });

    // Validate hostname input
    if (id === 'hostname' && !Valid952HostnameRegex.test(inputs[i].value)) {
      addValidationMessage(inputs[i], 'This hostname is invalid.');
      isValid = false;
      continue;
    }

    // Validate WiFi PSK
    if (id === 'wifi_psk' && inputs[i].value.length < 8) {
      addValidationMessage(inputs[i], 'A WiFi Passphrase Key (Password) must be between 8 and 63 characters.');
      isValid = false;
      continue;
    }

    s[id] = inputs[i].value;
  }

  if (isValid) {
    msg.s = s;
    websock.send(JSON.stringify(msg));
  }
}

/**
 * Initializes tab functionality
 *
 * @return void
 */
function initTabs() {
  var container = document.getElementById("menu");
  const TABS_SELECTOR = "div div a";

  // Enable click event to the tabs
  var tabs = container.querySelectorAll(TABS_SELECTOR);
  for (var i = 0; i < tabs.length; i++) {
    tabs[i].onclick = displayPage;
  }

  // Set current tab
  var currentTab = container.querySelector(TABS_SELECTOR);

  // Store which tab is current one
  var id = currentTab.id.split("_")[1];
  currentTab.parentNode.setAttribute("data-current", id);
  currentTab.classList.add("is-active");

  // Hide tab contents we don't need
  var pages = document.getElementById("pagescontent").querySelectorAll("section");
  for (var j = 1; j < pages.length; j++) {
    pages[j].style.display = "none";
  }
}

/**
 * Tab click / page display handler
 *
 * @return void
 */
function displayPage() {
  const CLASS_ACTIVE = "is-active";
  const CURRENT_ATTRIBUTE = "data-current";
  const ID_TABPAGE = "page_";
  var current = this.parentNode.getAttribute(CURRENT_ATTRIBUTE);

  // Remove class of active tab and hide contents
  document.getElementById("tab_" + current).classList.remove(CLASS_ACTIVE);
  document.getElementById(ID_TABPAGE + current).style.display = "none";

  // Add class to new active tab and show contents
  var id = this.id.split("_")[1];

  this.classList.add(CLASS_ACTIVE);
  document.getElementById(ID_TABPAGE + id).style.display = "block";
  this.parentNode.setAttribute(CURRENT_ATTRIBUTE, id);
}

/**
 * Handler for displaying/hiding the hamburger menu (visible on mobile devices)
 *
 * @return void
 */
function toggleNav() {
  const CLASS_ACTIVE = "is-active";
  var nav = document.getElementById("nav-menu");

  if (!nav.classList.contains(CLASS_ACTIVE)) {
    nav.classList.add(CLASS_ACTIVE);
  } else {
    nav.classList.remove(CLASS_ACTIVE);
  }
}

/**
 * Main
 *
 * @return void
 */
document.addEventListener('DOMContentLoaded', function() {
  document.getElementById('button-restart').addEventListener('click', restart, {
    passive: true
  });
  document.getElementById('reset').addEventListener('click', reset, {
    passive: true
  });
  document.getElementById('nav-toggle').addEventListener('click', toggleNav, {
    passive: true
  });
  document.getElementById('save').addEventListener('click', save, {
    passive: true
  });

  var pw = document.getElementById("pagescontent").querySelectorAll("i");
  [].forEach.call(pw, function(item) {
    item.addEventListener('touchstart click', togglePassword, {
      passive: true
    });
    item.addEventListener('click', togglePassword, {
      passive: true
    });
  });

  initTabs();
  wsConnect();
  esConnect();
});
