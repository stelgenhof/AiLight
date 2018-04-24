/**
 * Ai-Thinker RGBW Light Firmware
 *
 * This file is part of the Ai-Thinker RGBW Light Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.

 * Created by Sacha Telgenhof <me at sachatelgenhof dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2018 Sacha Telgenhof
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
const K_HD = "ha_discovery";
const K_RA = "rest_api";
const K_PM = "powerup_mode";

const S_ON = 'ON';
const S_OFF = 'OFF';

const WAIT = 10000;

/**
 * Object representing a Switch component
 *
 * @param id the DOM element to be rendered as a Switch component
 * @param du should the state of the Switch be broadcasted (WebSockets) or not?
 *
 * @return void
 */
function Switch(id, du = true) {
  this.id = id;
  this.du = du;
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

    let state = {};
    let value = this.state;

    if (this.id === 'state') {
      value = (this.state) ? S_ON : S_OFF;
    }

    // Handle visibility of HA Discovery settings
    if (this.id === K_HD) {
      let ad = document.getElementById('mqtt_ha_discovery');
      ad.style.display = (this.state) ? 'flex' : 'none';
    }

    // Handle visibility of REST API settings
    if (this.id === K_RA) {
      let ap = document.getElementById('rest_api_key');
      ap.style.display = (this.state) ? 'flex' : 'none';
    }

    state[this.id] = value;

    if (this.du) {
      websock.send(JSON.stringify(state));
    }
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

    let output = this.el.parentNode.getElementsByTagName('output')[0];
    if (typeof(output) !== "undefined") {
      output.innerHTML = this.el.value;
    }

  };

  this._send = function() {
    let msg = {
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

    let rgb = [K_R, K_G, K_B];
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
let websock;
let stSwitch = new Switch(K_S);
let brSlider = new Slider(K_BR);
let ctSlider = new Slider(K_CT);
let rSlider = new Slider(K_R);
let gSlider = new Slider(K_G);
let bSlider = new Slider(K_B);
let wSlider = new Slider(K_W);
let gmSwitch = new Switch(K_GM);
let hdSwitch = new Switch(K_HD, false);
let raSwitch = new Switch(K_RA, false);
let hS = false;

/**
 * Sends the RGB triplet state to the connected WebSocket clients
 *
 * @return void
 */
function sendRGB() {
  let msg = {
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
  for (let key in data) {

    // Process Device information
    if (key === 'd') {
      document.title = data.d.app_name;

      // Bind data to DOM
      for (let dev in data[key]) {
        // Bind to span elements
        let d = document.querySelectorAll("span[data-s='" + dev + "']");
        [].forEach.call(d, function(item) {
          item.innerHTML = data[key][dev];
        });
      }
    }

    // Process settings
    if (key === 's') {
      document.title += ' - ' + data[key].hostname;

      // Bind data to DOM
      for (let s in data[key]) {
        // Bind to span elements
        let a = document.getElementById("pagescontent").querySelectorAll("span[data-s='" + s + "']");
        [].forEach.call(a, function(item) {
          item.innerHTML = data[key][s];
        });

        // Bind to specific DOM elements
        if (document.getElementById(s) !== null) {
          document.getElementById(s).value = data[key][s];
        }

        // Set HA Discovery switch and prefix field
        if (s === "switch_ha_discovery") {
          hdSwitch.setState(data[key][s]);

          let ad = document.getElementById('mqtt_ha_discovery');
          if (!data[key][s]) {
            ad.style.display = "none";
          }
        }

        // Set REST API switch and API Key field
        if (s === "switch_rest_api") {
          raSwitch.setState(data[key][s]);

          let ap = document.getElementById('rest_api_key');
          if (!data[key][s]) {
            ap.style.display = "none";
          }
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
  let host = window.location.hostname;
  let port = location.port;

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
    let data = getJSON(e.data);
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
    let source = new EventSource('/events');

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
        let pb = document.getElementById("op");
        let p = parseInt(e.data.split("-")[1]);

        pb.value = p;

        if (p === 100 && !hS) {
          hS = true;
          let f = document.createElement('p');
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
  let response = window.confirm("Are you sure you want to restart your Ai-Thinker RGBW Light?");
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
  let response = window.confirm("You are about to reset your Ai-Thinker RGBW Light to the factory defaults!\n Are you sure you want to reset?");
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
  let ie = document.getElementById(this.dataset.input);
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
  let v = document.createElement('p');
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
  let s = {};
  let msg = {};
  let isValid = true;

  let Valid952HostnameRegex = /^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$/i;

  // Parse all input fields and validate where needed
  let inputs = document.forms[0].querySelectorAll("input");
  for (let i = 0; i < inputs.length; i++) {
    let id = inputs[i].id.split('.').pop();

    // Clear any validation messages
    inputs[i].classList.remove("is-danger");
    let p = inputs[i].parentNode;
    let v = p.querySelectorAll("p.is-danger");
    [].forEach.call(v, function(item) {
      p.removeChild(item);
    });

    // Validate hostname input
    if (id === 'hostname' && !Valid952HostnameRegex.test(inputs[i].value)) {
      addValidationMessage(inputs[i], 'This hostname is invalid.');
      isValid = false;
      continue;
    }

    // Validate WiFi ssid
    if (id === 'wifi_ssid') {
      if (!inputs[i].value || inputs[i].value.length === 0 || inputs[i].value > 31) {
        addValidationMessage(inputs[i], 'A WiFi SSID must be present with a maximum of 31 characters.');
        isValid = false;
        continue;
      }
    }

    // Validate WiFi PSK
    if (id === 'wifi_psk') {
      if (inputs[i].value && inputs[i].value.length > 0 && (inputs[i].value.length > 63 || inputs[i].value.length < 8)) {
        addValidationMessage(inputs[i], 'A WiFi Passphrase Key (Password) must be between 8 and 63 characters.');
        isValid = false;
        continue;
      }
    }

    // Validate API Key
    if (id === 'api_key') {
      if (inputs[i].value && inputs[i].value.length > 0 && (inputs[i].value.length > 32 || inputs[i].value.length < 8)) {
        addValidationMessage(inputs[i], 'An API Key must be between 8 and 32 characters.');
        isValid = false;
        continue;
      }
    }

    s[id] = (inputs[i].type === 'checkbox') ? inputs[i].checked : inputs[i].value;
  }

  // Parse all select fields
  let selects = document.forms[0].querySelectorAll("select");
  for (let j = 0; j < selects.length; j++) {
    let id = selects[j].id.split('.').pop();
    s[id] = selects[j].value;
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
  let container = document.getElementById("menu");
  const TABS_SELECTOR = "div div a";

  // Enable click event to the tabs
  let tabs = container.querySelectorAll(TABS_SELECTOR);
  for (let i = 0; i < tabs.length; i++) {
    tabs[i].onclick = displayPage;
  }

  // Set current tab
  let currentTab = container.querySelector(TABS_SELECTOR);

  // Store which tab is current one
  let id = currentTab.id.split("_")[1];
  currentTab.parentNode.setAttribute("data-current", id);
  currentTab.classList.add("is-active");

  // Hide tab contents we don't need
  let pages = document.getElementById("pagescontent").querySelectorAll("section");
  for (let j = 1; j < pages.length; j++) {
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
  let current = this.parentNode.getAttribute(CURRENT_ATTRIBUTE);

  // Remove class of active tab and hide contents
  document.getElementById("tab_" + current).classList.remove(CLASS_ACTIVE);
  document.getElementById(ID_TABPAGE + current).style.display = "none";

  // Add class to new active tab and show contents
  let id = this.id.split("_")[1];

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
  let nav = document.getElementById("nav-menu");

  if (!nav.classList.contains(CLASS_ACTIVE)) {
    nav.classList.add(CLASS_ACTIVE);
  } else {
    nav.classList.remove(CLASS_ACTIVE);
  }
}

/**
 * Handler for generating an API Key
 *
 * The generated key is a standard UUID value excluding the hyphens.
 * Source: https://stackoverflow.com/questions/105034/create-guid-uuid-in-javascript
 *
 * @return void
 */
function generateAPIKey() {
  let akv = document.getElementById('api_key');

  akv.value = ([1e7] + 1e3 + 4e3 + 8e3 + 1e11).replace(/[018]/g, c =>
    (c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> c / 4).toString(16)
  );
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

  let pw = document.getElementById("pagescontent").querySelectorAll("i.icon-eye");
  [].forEach.call(pw, function(item) {
    item.addEventListener('touchstart', togglePassword, {
      passive: true
    });
    item.addEventListener('click', togglePassword, {
      passive: true
    });
  });

  let ak = document.getElementById("pagescontent").querySelector("i.icon-arrow-sync");
  ak.addEventListener('touchstart', generateAPIKey, {
    passive: true
  });
  ak.addEventListener('click', generateAPIKey, {
    passive: true
  });

  initTabs();
  wsConnect();
  esConnect();

});
